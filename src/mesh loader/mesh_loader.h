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

    template<typename T>
    struct DataArray {
        //std::string type_name;
        std::vector<T> content;
    };


    struct FileData {
        int numberOfPoints = 0;
        double *pointList;

        int numberOfCell = 0;
        Cell *cellList;

        std::map<std::string, DataArray<std::string>> cellDataString;
        std::map<std::string, DataArray<double>> cellDataDouble;
        std::map<std::string, DataArray<float>> cellDataFloat;
        std::map<std::string, DataArray<int>> cellDataInt;
        std::map<std::string, DataArray<unsigned int>> cellDataUInt;
        std::map<std::string, DataArray<bool>> cellDataBool;

        std::map<std::string, DataArray<std::string>> pointDataString;
        std::map<std::string, DataArray<double>> pointDataDouble;
        std::map<std::string, DataArray<float>> pointDataFloat;
        std::map<std::string, DataArray<int>> pointDataInt;
        std::map<std::string, DataArray<unsigned int>> pointDataUInt;
        std::map<std::string, DataArray<bool>> pointDataBool;


        ~FileData() {

        }

    };


    bool load_f3grid(const char *in_file_path, FileData &data);

    bool load_vtu(const char *in_file_path, FileData &data);

    bool save_vtu(const char *out_file_path, const FileData &data);


}






