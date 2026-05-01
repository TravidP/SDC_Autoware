#ifndef MAP_GENERATOR_HPP
#define MAP_GENERATOR_HPP

#include <string>
#include <utility>
#include <vector>
#include <fstream>

uint getUID(); //returns unique id. not thread safe.

struct MapGeneratorOptions
{
    double speed_limit;
    double lane_width;
    int max_nodes_in_way;
    bool loop;
    int averaging_window_size;
    double base_to_front;
};
struct Node {
    Node(double lat, double lon) : id_(getUID()), lat_(lat), lon_(lon)
    {}
    uint id_ = 0;
    double lat_ = 0.;
    double lon_ = 0.;

    void write_to_file(std::ofstream &filestream) const
    {
        filestream << "  <node id='" << id_ << "' visible='true' version='1' lat='" << std::setprecision(15) << lat_
               << "' lon='" << lon_ << "'>\n";
        filestream << "    <tag k='ele' v='0.0' />\n";
        filestream << "  </node>\n";
    }
};

struct Way {
    Way(std::vector<uint> node_ids) : id_(getUID()), node_ids_(node_ids)
    {}
    uint id_ = 0;
    std::vector<uint> node_ids_;

    void write_to_file(std::ofstream &filestream) const
    {
        filestream << "  <way id='" << id_ << "' visible='true' version='1'>\n";
    for (const auto& node_id : node_ids_) {
        filestream << "    <nd ref='" << node_id << "' />\n";
    }
    filestream << "    <tag k='subtype' v='solid' />\n";
    filestream << "    <tag k='type' v='line_thin' />\n";
    filestream << "    <tag k='width' v='0.200' />\n";
    filestream << "  </way>\n";
    }
};

struct Relation {
    Relation(uint left_way_id, uint right_way_id, double speed_limit) : id_(getUID()), left_way_id_(left_way_id), right_way_id_(right_way_id), speed_limit_(speed_limit)
    {}
    uint id_ = 0;
    uint left_way_id_ = 0;
    uint right_way_id_ = 0;
    double speed_limit_ = 5.0;

    void write_to_file(std::ofstream &filestream) const
    {
        filestream << "  <relation id='" << id_ << "' visible='true' version='1'>\n";
        filestream << "    <member type='way' ref='" << left_way_id_<< "' role='left' />\n";
        filestream << "    <member type='way' ref='" << right_way_id_ << "' role='right' />\n";
        filestream << "    <tag k='location' v='urban' />\n";
        filestream << "    <tag k='one_way' v='yes' />\n";
        filestream << "    <tag k='speed_limit' v='" << speed_limit_ << "' />\n";
        filestream << "    <tag k='subtype' v='road' />\n";
        filestream << "    <tag k='turn_direction' v='straight' />\n";
        filestream << "    <tag k='type' v='lanelet' />\n";
        filestream << "  </relation>\n";
    }
};

class MapGenerator {
public:
    MapGenerator(const MapGeneratorOptions& opts);
    void generate_lanelet_map(std::vector<std::pair<double, double>> coordinates, const std::string &output_folder);
    void generate_map_projector(std::vector<std::pair<double, double>> const &coordinates, const std::string &output_folder);
private:
    void generate_nodes(std::vector<std::pair<double, double>> const &coordinates, std::ofstream &filestream);
    void generate_ways(std::ofstream &filestream);
    void generate_relations(std::ofstream &filestream);
    void add_nodes_in_line();
    void clear_data();

    std::pair<std::vector<Node>, std::vector<Node>> node_pairs_; //Holds two vectors, one for left side nodes and one for right. node_pairs.first --> left side
    std::vector<std::pair<Way,Way>> way_pairs_; //Each pair in the vector holds two ways, one for left side and one for right
    std::vector<Relation> relations_;

    double speed_limit_;
    double lane_width_;
    int max_nodes_in_way_;
    bool loop_;
    int averaging_window_size_;
    double base_to_front_;
};

#endif // MAP_GENERATOR_HPP
