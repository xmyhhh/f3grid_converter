//
// Created by xmyci on 20/02/2024.
//

#include <iostream>
#include "config_loader.h"
#include "utils/file/file_path.h"
#include "utils/log/log.h"
#include "mesh loader/mesh_loader.h"

void Config_Loader::create_default_config_file(const std::string &path) {
    json j;

    j["input"]["input_file_path"] = {"...", "..."};

    j["output"]["save_output_path"] = ".";
    j["output"]["array_to_number"] = true;
    j["output"]["export_six_surface"] = true;
    j["output"]["export_face_related"] = false;

    j["export_six_surface_setting"]["r_x"] = -50;
    j["export_six_surface_setting"]["r_y"] = 0;
    j["export_six_surface_setting"]["r_z"] = 0;

    std::error_code err;

    create_directory_recursive(get_file_path(path), err);

    std::ofstream o(path);
    o << std::setw(4) << j << std::endl;
}

bool Config_Loader::load_config_file(const std::string &path, Config &c) {

    std::ifstream f(path);
    json j = json::parse(f);

    c.save_output_path = j["output"]["save_output_path"];
    c.export_six_surface = j["output"]["export_six_surface"];
    c.export_face_related = j["output"]["export_face_related"];
    c.array_to_number = j["output"]["array_to_number"];

    c.r_x = j["export_six_surface_setting"]["r_x"];
    c.r_y = j["export_six_surface_setting"]["r_y"];
    c.r_z = j["export_six_surface_setting"]["r_z"];

    if (j["input"]["input_file_path"].size() != 0) {
        for (std::string element: j["input"]["input_file_path"]) {
            if (!is_file_exist(element)) {
                log_print("file not exist:" + element);
                return false;
            }
            auto hash_hit = [](std::string inString) {
                if (inString == "obj") return Mesh_Loader::OBJ;
                if (inString == "vtk") return Mesh_Loader::VTK;
                if (inString == "vtu") return Mesh_Loader::VTU;
                if (inString == "mesh") return Mesh_Loader::MESH;
                if (inString == "f3grid") return Mesh_Loader::F3GRID;
                return Mesh_Loader::UNKNOW;
            };

            switch (hash_hit(get_file_extension(element))) {
                case Mesh_Loader::F3GRID:
                    c.input_file_path.push_back(element);
                    break;
                case Mesh_Loader::VTU:
                    c.input_file_path.push_back(element);
                    break;
                default:
                    log_print("file format not support:" + element);
                    return false;
            }

        }
    }
    return true;

}
