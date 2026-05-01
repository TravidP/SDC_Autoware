
#include "yolo_autoware_tools/nearest_trafflic_light.hpp"

NearestTrafficLight::NearestTrafficLight() : Node("nearest_traffic_light")
{
  map_path_ = declare_parameter("map_path", "/home/greendinokubra/autoware/src/Maps/maps/leyl/");

  traffic_light_info_ = extract_traffic_light_info(map_path_ + "/lanelet2_map.osm");

  YAML::Node yaml_node = YAML::LoadFile(map_path_ + "/map_projector_info.yaml");
  float map_origin_latitude = yaml_node["map_origin"]["latitude"].as<float>();
  float map_origin_longitude = yaml_node["map_origin"]["longitude"].as<float>();

  subscription_ = this->create_subscription<nav_msgs::msg::Odometry>(
    "/localization/kinematic_state", qos_profile_,
    std::bind(&NearestTrafficLight::localization_callback, this, std::placeholders::_1));

  publisher_ = this->create_publisher<std_msgs::msg::Int32>("closest_traffic_light_id", 10);

  timer_ = this->create_wall_timer(
    std::chrono::seconds(1), std::bind(&NearestTrafficLight::publish_traffic_light_info, this));

  _projector = lanelet::projection::TransverseMercatorProjector(
    lanelet::Origin({map_origin_latitude, map_origin_longitude}));
}

void NearestTrafficLight::localization_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
{
  lanelet::BasicPoint3d projected_point;
  projected_point.x() = msg->pose.pose.position.x;
  projected_point.y() = msg->pose.pose.position.y;
  projected_point.z() = msg->pose.pose.position.z;

  lanelet::GPSPoint gps_point = _projector.reverse(projected_point);
  vehicle_lat_ = gps_point.lat;
  vehicle_lon_ = gps_point.lon;
  vehicle_ele_ = gps_point.ele;
}

double NearestTrafficLight::calculate_distance(double lat1, double lon1, double lat2, double lon2)
{
  lat1 = degrees_to_radians(lat1);
  lon1 = degrees_to_radians(lon1);
  lat2 = degrees_to_radians(lat2);
  lon2 = degrees_to_radians(lon2);

  double dlon = lon2 - lon1;
  double dlat = lat2 - lat1;
  double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
             std::cos(lat1) * std::cos(lat2) * std::sin(dlon / 2) * std::sin(dlon / 2);
  double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
  double distance = 6371 * c * 1000;  // Radius of the earth in m
  return distance;
}

double NearestTrafficLight::degrees_to_radians(double degrees) { return degrees * M_PI / 180.0; }

std::vector<NearestTrafficLight::RelationInfo> NearestTrafficLight::extract_traffic_light_info(
  const std::string & file_path)
{
  std::vector<RelationInfo> traffic_light_info;
  pugi::xml_document doc;
  if (!doc.load_file(file_path.c_str())) {
    return traffic_light_info;
  }

  auto root = doc.child("osm");
  if (!root) return traffic_light_info;

  std::unordered_map<std::string, std::pair<double, double>> nodes;
  std::unordered_map<std::string, std::vector<std::string>> ways;

  for (auto node = root.child("node"); node; node = node.next_sibling("node")) {
    std::string node_id = node.attribute("id").value();
    double lat = node.attribute("lat").as_double();
    double lon = node.attribute("lon").as_double();
    nodes[node_id] = std::make_pair(lat, lon);
  }

  for (auto way = root.child("way"); way; way = way.next_sibling("way")) {
    std::string way_id = way.attribute("id").value();
    std::vector<std::string> nd_refs;
    for (auto nd = way.child("nd"); nd; nd = nd.next_sibling("nd")) {
      nd_refs.push_back(nd.attribute("ref").value());
    }
    ways[way_id] = nd_refs;
  }

  for (auto relation = root.child("relation"); relation;
       relation = relation.next_sibling("relation")) {
    bool is_traffic_light = false;
    RelationInfo relation_info;
    relation_info.id = relation.attribute("id").value();

    for (auto tag = relation.child("tag"); tag; tag = tag.next_sibling("tag")) {
      if (
        std::string(tag.attribute("k").value()) == "subtype" &&
        std::string(tag.attribute("v").value()) == "traffic_light") {
        is_traffic_light = true;
        break;
      }
    }

    if (is_traffic_light) {
      for (auto member = relation.child("member"); member; member = member.next_sibling("member")) {
        std::string role = member.attribute("role").value();
        std::string way_id = member.attribute("ref").value();
        if (ways.find(way_id) != ways.end()) {
          for (const auto & node_id : ways[way_id]) {
            if (nodes.find(node_id) != nodes.end()) {
              auto lat_lon = nodes[node_id];
              if (role == "ref_line") {
                relation_info.ref_line.push_back(lat_lon);
              } else if (role == "light_bulbs") {
                relation_info.light_bulbs.push_back(lat_lon);
              } else if (role == "refers") {
                relation_info.refers.push_back(lat_lon);
              }
            }
          }
        }
      }
      traffic_light_info.push_back(relation_info);
    }
  }

  return traffic_light_info;
}

void NearestTrafficLight::publish_traffic_light_info()
{
  auto msg = std::make_shared<std_msgs::msg::Int32>();
  double min_distance = 25.0;  //std::numeric_limits<double>::infinity();
  std::string closest_relation_id;

  for (const auto & item : traffic_light_info_) {
    for (const auto & coord : item.ref_line) {
      double distance = calculate_distance(coord.first, coord.second, vehicle_lat_, vehicle_lon_);
      if (distance < min_distance) {
        min_distance = distance;
        closest_relation_id = item.id;
      }
    }
  }

  if (!closest_relation_id.empty()) {
    msg->data = std::stoi(closest_relation_id);
    // RCLCPP_WARN(this->get_logger(), "Closest ref_line distance: %f m for relation ID: %s", min_distance, closest_relation_id.c_str());
    publisher_->publish(*msg);
  } else {
    // RCLCPP_WARN(this->get_logger(), "No traffic light found");
  }
}

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<NearestTrafficLight>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
