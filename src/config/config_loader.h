//
// Created by xmyci on 20/02/2024.
//

#ifndef TETGEO_CONFIG_LOADER_H
#define TETGEO_CONFIG_LOADER_H

#include <string>
#include "json.hpp"

#include <fstream>


struct Config {
    bool export_six_surface = false;
    bool export_face_related = false;
    bool array_to_number = false;
    std::vector<std::string> input_file_path;
    std::string save_output_path;
    double r_x = 0, r_y = 0, r_z = 0;
};


class Config_Loader {
    using json = nlohmann::json;
public:
    static void create_default_config_file(const std::string &path);

    static bool load_config_file(const std::string &path, Config &c);
};

extern Config config;

#endif //TETGEO_CONFIG_LOADER_H
