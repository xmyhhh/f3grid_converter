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
    struct Group{
        std::string g_name;
    };

    struct Cell {
        int numberOfPoints = 3; //3表示三角形
        int *pointList;

        double *cellattr;
    };

    struct CellBlock{
        std::vector<std::string> cellattrname;
        int numberOfCell = 0;
        Cell *cellList;
    };

    struct FileData {
        int numberOfPoints = 0;
        double *pointList;

        std::vector<CellBlock> cellblocks;

        ~FileData() {

        }

    };


    bool load_f3grid(const char *in_file_path, FileData &data);
    bool load_vtu(const char *in_file_path, FileData &data);

    bool save_vtu(const char *out_file_path, const FileData &data);


}






