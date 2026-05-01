#include <cmath>
#include <fstream>
#include <iostream>
#include <eigen3/Eigen/Dense>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/segment.hpp>

#include "map_generator/map_generator.hpp"
#include "map_generator/gd_conversion.hpp"

#include "lanelet2_projection/LocalCartesian.h"

namespace bg = boost::geometry;

uint getUID() {
    static uint global_uid = 1;
    return global_uid++;
}

MapGenerator::MapGenerator(const MapGeneratorOptions& opts):
    speed_limit_(opts.speed_limit),
    lane_width_(opts.lane_width),
    max_nodes_in_way_(opts.max_nodes_in_way),
    loop_(opts.loop),
    averaging_window_size_(opts.averaging_window_size),
    base_to_front_(opts.base_to_front)
    {}

void MapGenerator::generate_map_projector(std::vector<std::pair<double, double>> const &coordinates, const std::string &output_folder) {
    std::ofstream projector_filestream(output_folder + "/map_projector_info.yaml");
    projector_filestream << "projector_type: TransverseMercator \n" 
                         << "vertical_datum: WGS84 \n" 
                         << "mgrs_grid: '' \n" 
                         << "map_origin:\n" 
                         << "  latitude: " << std::setprecision(15) << coordinates[0].first << "\n" 
                         << "  longitude: " << std::setprecision(15) << coordinates[0].second << "\n" 
                         << "  altitude: " <<  "54.5648" << "\n" ; //This should not be hardcoded TODO
    projector_filestream.close(); 
}

void MapGenerator::generate_lanelet_map(std::vector<std::pair<double, double>> coordinates, const std::string &output_folder) {
    std::ofstream map_filestream(output_folder + "/lanelet2_map.osm");
    map_filestream << "<?xml version='1.0' encoding='UTF-8'?>\n";
    map_filestream << "<osm version='0.6' generator='navsatfix_to_lanelets'>\n";
    clear_data();
    generate_nodes(coordinates, map_filestream);
    generate_ways(map_filestream);
    generate_relations(map_filestream);
    map_filestream << "</osm>\n";
    map_filestream.close();   
}

void MapGenerator::clear_data()
{
    node_pairs_.first.clear();
    node_pairs_.second.clear();
    way_pairs_.clear();
    relations_.clear();
}

void MapGenerator::generate_nodes(const std::vector<std::pair<double, double>> &coordinates, std::ofstream &filestream) {    
    // keep track of the distance to the next point
    double accumulatedDistance = 0.0;

    for (auto it = std::next(coordinates.begin()); it != coordinates.end(); it++)
    {
        auto previous_element = std::prev(it);
        double lat1 = previous_element->first;
        double lon1 = previous_element->second;
        double lat2 = it->first;
        double lon2 = it->second;

        double x1 = deg2rad(lon1) * EARTH_RADIUS_M * cos(deg2rad(lat1));
        double y1 = deg2rad(lat1) * EARTH_RADIUS_M;
        double x2 = deg2rad(lon2) * EARTH_RADIUS_M * cos(deg2rad(lat2));
        double y2 = deg2rad(lat2) * EARTH_RADIUS_M;
        
        double prev_x, prev_y;

        // Use midpoint of last confirmed lanelet node as reference
        if (!node_pairs_.first.empty()) {
            prev_x = deg2rad((node_pairs_.first.back().lon_ + node_pairs_.second.back().lon_) / 2.0) * EARTH_RADIUS_M * cos(deg2rad((node_pairs_.first.back().lat_ + node_pairs_.second.back().lat_) / 2.0));
            prev_y = deg2rad((node_pairs_.first.back().lat_ + node_pairs_.second.back().lat_) / 2.0) * EARTH_RADIUS_M;
        } 
        // For first segment, fallback to previous GPS point
        else {
            prev_x = x1;
            prev_y = y1;
        }

        double direction_x, direction_y;
        direction_x = x2 - prev_x;
        direction_y = y2 - prev_y;
        
        // Skip segment if movement is smaller then 25 cm
        double distance = sqrt(direction_x * direction_x + direction_y * direction_y);
        accumulatedDistance += distance;
        if (accumulatedDistance < 0.25)
            continue;

        accumulatedDistance = 0.0;

        direction_x /= distance;
        direction_y /= distance;

        double offset_left_x = -direction_y * lane_width_ * 0.5; //Get rid of magic numbers. they should be part of configuration file.
        double offset_left_y = direction_x * lane_width_ * 0.5;

        double offset_lat = rad2deg(offset_left_y / EARTH_RADIUS_M);
        double offset_lon = rad2deg(offset_left_x / (EARTH_RADIUS_M * cos(deg2rad(lat1))));

        if (std::isnan(lat1 + offset_lat) || std::isnan(lat1 - offset_lat) || std::isnan(lon1 + offset_lon) || std::isnan(lon1 - offset_lon)) {
            continue;
        }

        if (node_pairs_.first.size() == 0) {
            node_pairs_.first.emplace_back(lat1 + offset_lat, lon1 + offset_lon); //left side
            node_pairs_.second.emplace_back(lat1 - offset_lat, lon1 - offset_lon); //right side
        }
        else {
            typedef bg::model::point<double, 2, bg::cs::cartesian> Point;
            typedef bg::model::segment<Point> Segment;
            std::vector<Point> intersection;
            double distance_to_left;
            double distance_to_right;

            // Created line segments
            // First line is last line of the node pair, second line is the current line(candidate for node pair)
            Point line_segment1_point1(node_pairs_.first.back().lat_, node_pairs_.first.back().lon_);
            Point line_segment1_point2(node_pairs_.second.back().lat_, node_pairs_.second.back().lon_);
            Point line_segment2_point1(lat1 + offset_lat, lon1 + offset_lon);
            Point line_segment2_point2(lat1 - offset_lat, lon1 - offset_lon);
            Segment line_segment1(line_segment1_point1, line_segment1_point2);
            Segment line_segment2(line_segment2_point1, line_segment2_point2);

            // Checking if segments intersect or not
            bg::intersection(line_segment1, line_segment2, intersection);
            if (intersection.size() > 1)
            {
                continue;
            }
            else
            {
                node_pairs_.first.emplace_back(lat1 + offset_lat, lon1 + offset_lon); //left side
                node_pairs_.second.emplace_back(lat1 - offset_lat, lon1 - offset_lon); //right side
            }
        }
    }

    add_nodes_in_line();

    if (node_pairs_.first.size() == node_pairs_.second.size()) 
    {
        if (averaging_window_size_ > 0) //apply smoothing only if requested
        {
            int num_points = node_pairs_.first.size();
            for (int i = 1; i < num_points; ++i) {
                double sum_left_x = 0.0;
                double sum_left_y = 0.0;
                double sum_right_x = 0.0;
                double sum_right_y = 0.0;
                int count = 0;

                // Sliding window to sum up the points in the window
                for (int j = i; j < std::min(i + averaging_window_size_, num_points); ++j) {
                    sum_left_x += node_pairs_.first.at(j).lat_;
                    sum_left_y += node_pairs_.first.at(j).lon_;
                    sum_right_x += node_pairs_.second.at(j).lat_;
                    sum_right_y += node_pairs_.second.at(j).lon_;
                    count++;
                }
                node_pairs_.first[i].lat_ = sum_left_x / count; //left side
                node_pairs_.first[i].lon_ = sum_left_y / count; //left side
                node_pairs_.second[i].lat_ = sum_right_x / count; //right side
                node_pairs_.second[i].lon_ = sum_right_y / count; //right side
            }
        }

        for (size_t i = 0; i < node_pairs_.first.size(); ++i) {
            node_pairs_.first.at(i).write_to_file(filestream);
            node_pairs_.second.at(i).write_to_file(filestream);
        }
        std::cout <<"Number of nodes created : " <<static_cast<int>(node_pairs_.first.size() + node_pairs_.second.size()) <<std::endl;
    }
    return;
}

void MapGenerator::add_nodes_in_line() {

    /*projects the two last (lat/lon) points in local ENU, and returns a point that is
    1.1* distance in the direction of the vector between the two points.*/
    auto get_extended_node_coords = [](std::vector<Node>& node_vec, double distance) {
        auto it = node_vec.end() - 2;
        lanelet::projection::LocalCartesianProjector projector(
            lanelet::Origin({it->lat_, it->lon_}));
        lanelet::BasicPoint3d second_point = projector.forward(lanelet::GPSPoint{std::next(it)->lat_, std::next(it)->lon_});
        Eigen::Vector2d vector (second_point[0] ,second_point[1]);
        Eigen::Vector2d new_point = vector.normalized() * (vector.norm() + 1.1 * distance);
        return lanelet::GPSPoint{projector.reverse(lanelet::BasicPoint3d{new_point[0], new_point[1], 0.0})};
    };

    auto left_node_coords = get_extended_node_coords(node_pairs_.first, base_to_front_);
    auto right_node_coords = get_extended_node_coords(node_pairs_.second, base_to_front_);
    node_pairs_.first.emplace_back(left_node_coords.lat, left_node_coords.lon);
    node_pairs_.second.emplace_back(right_node_coords.lat, right_node_coords.lon);
}

void MapGenerator::generate_ways(std::ofstream &filestream) {

    std::vector<uint> left_way_node_ids;
    std::vector<uint> right_way_node_ids;
    for (size_t i = 0; i < node_pairs_.first.size(); i++)
    {
        left_way_node_ids.push_back(node_pairs_.first.at(i).id_);
        right_way_node_ids.push_back(node_pairs_.second.at(i).id_);
        if (static_cast<int>(left_way_node_ids.size()) >= max_nodes_in_way_ || i+1 >= node_pairs_.first.size())
        {
            way_pairs_.emplace_back(left_way_node_ids, right_way_node_ids);
            left_way_node_ids.clear();
            right_way_node_ids.clear();
            left_way_node_ids.push_back(node_pairs_.first.at(i).id_);
            right_way_node_ids.push_back(node_pairs_.second.at(i).id_);
        }
    }
    if (loop_)
    {
        // create a new way by taking the first and last node in map. in this way, the map beomes a loop map
        std::vector<uint> connection_left{node_pairs_.first.back().id_, node_pairs_.first.front().id_};
        std::vector<uint> connection_right{node_pairs_.second.back().id_, node_pairs_.second.front().id_};

        way_pairs_.emplace_back(connection_left, connection_right);
    }

    std::cout <<"Number of ways created : " <<static_cast<int>(way_pairs_.size()*2) <<std::endl;
    for (const auto& way_pair : way_pairs_)
    {
        way_pair.first.write_to_file(filestream);
        way_pair.second.write_to_file(filestream);
    }
    return;
}

void MapGenerator::generate_relations(std::ofstream &filestream)
{
    for (const auto& way_pair : way_pairs_)
    {
        relations_.emplace_back(way_pair.first.id_, way_pair.second.id_, speed_limit_);
    }
    for (const auto& relation : relations_)
    {
        relation.write_to_file(filestream);
    }
    std::cout <<"Number of relations created : " <<static_cast<int>(relations_.size()) <<std::endl;
    return;
}
