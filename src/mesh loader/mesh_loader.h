#pragma once

#include <cstdio>
#include <stdlib.h>
#include <cassert>
#include <string>
#include <vector>
#include "utils/file/file_path.h"

namespace Mesh_Loader {

    enum string_code {
        OBJ,
        OFF,
        VTK,
        VTU,
        MESH,
        F3GRID,
        UNKNOW
    };

    struct Cell {
        int numberOfPoints = 3; //3表示三角形
        int *pointList;
    };


    struct FileData {
        int numberOfPoints = 0;
        double *pointList;

        std::vector<std::string> cellattrname;
        int numberOfCell = 0;
        Cell *cellList;

        std::vector<std::vector<std::string>> cell_data_array;


        ~FileData() {

        }

    };


    bool load_f3grid(const char *in_file_path, FileData &data);
    bool load_vtu(const char *in_file_path, FileData &data);

    bool save_vtu(const char *out_file_path, const FileData &data);


}






