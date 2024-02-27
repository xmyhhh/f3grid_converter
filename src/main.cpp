//
// Created by xmyci on 20/02/2024.
//

#include "tbb/tbb.h"
#include "CLI11.hpp"

#include "config/config_loader.h"
#include "utils/log/log.h"
#include "utils/file/file_path.h"

#define  ASSERT_MSG(condition, msg) \
    if((condition) == false) { log_print(msg) ; assert(false);}

Config config;

int main(int argc, char **argv) {

    CLI::App app{"App description"};
    argv = app.ensure_utf8(argv);

    std::string file_path = "default";
    //file_path = "C:/Users/xmy/Desktop/TetGeo/config/default_config_s.json";
    app.add_option("-f,--file", file_path, "A help string");

    CLI11_PARSE(app, argc, argv);

    if (strcmp(file_path.c_str(), "default") == 0) {
        log_print("ERROR: input config path can not be null!");
        return 0;
    }


    if (is_file_exist(file_path) == false) {
        log_print("create config file in path:" + file_path);
        Config_Loader::create_default_config_file(file_path);
        return 0;
    } else {
        bool r = Config_Loader::load_config_file(file_path, config);
        if (!r)
            return -1;
        log_print("read config file in path success!");
        std::error_code err;
        ASSERT_MSG(create_directory_recursive(config.save_output_path, err), "ERROR: can not create the output dir!");
    }


    return 0;
}