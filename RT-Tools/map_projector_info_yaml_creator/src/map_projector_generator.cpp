#include <iostream>
#include <fstream>
#include <string>

#include "pugixml.hpp"
#include "yaml-cpp/yaml.h"

void parseOSM(const std::string& filename, std::string* latitude_value, std::string* longitude_value) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());
    if (!result) {
        throw std::runtime_error("Errors occurred while parsing osm file: " + std::string(result.description()));
    }

    pugi::xml_node osmNode = doc.child("osm");
    pugi::xml_node nodeinfo = osmNode.child("node");
    if (nodeinfo.attribute("lat")) {
        *latitude_value = nodeinfo.attribute("lat").value();
    }
    if (nodeinfo.attribute("lon")) {
        *longitude_value = nodeinfo.attribute("lon").value();
    }
}

void createYAMLFile(const std::string& filename, const std::string& latitude_value, const std::string& longitude_value) {
    YAML::Node node;
    node["projector_type"] = "TransverseMercator";
    node["vertical_datum"] = "WGS84";
    node["map_origin"]["latitude"] = latitude_value;
    node["map_origin"]["longitude"] = longitude_value;
    node["map_origin"]["altitude"] = "54.5648";
    
    std::ofstream fout(filename);
    if (fout) {
        fout << node;
        std::cout << "YAML file saved successfully." << std::endl;
    } else {
        throw std::runtime_error("Error: Unable to open file '" + filename + "' for writing.");
    }
}

int main(int argc, char** argv) {
    if (argc < 2)
    {
        throw std::runtime_error("Filepath where the osm file lies need to be provided as the first argument!");
    }
    std::string map_file_path = argv[1];

    std::string map_name = map_file_path + "/lanelet2_map.osm";
    std::string yaml_file_name = map_file_path + "/map_projector_info.yaml";
    std::string latitude_value, longitude_value;
    
    try {
        parseOSM(map_name, &latitude_value, &longitude_value);
        createYAMLFile(yaml_file_name, latitude_value, longitude_value);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
