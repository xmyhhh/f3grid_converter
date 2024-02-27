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
        UNKNOW
    };

    struct Cell {
        int numberOfPoints = 3; //3表示三角形
        int *pointList;

        double *cellattr;
    };

    struct FileData {

        int numberOfPoints = 0;
        double *pointList;

        std::vector<std::string> pointattrname;
        int *pointattr;


        std::vector<std::string> cellattrname;
        int numberOfCell = 0;

        Cell *cellList;

        ~FileData() {
            return;
            free(pointList);
            if (pointattrname.size() != 0)
                free(pointattr);
            for (int i = 0; i < numberOfCell; i++) {
                auto c = cellList[i];
                free(c.pointList);
                if (cellattrname.size() != 0)
                    free(c.cellattr);
            }
            free(cellList);
        }

    };

    bool load_by_extension(const char *in_file_path, FileData &data);

    bool load_obj(const char *in_file_path, FileData &data);

    bool load_vtk(const char *in_file_path, FileData &data);

    bool load_vtu(const char *in_file_path, FileData &data);

    bool load_f3grid(const char *in_file_path, FileData &data);

    bool load_mesh(const char *in_file_path, FileData &data);

    bool save_off(const char *out_file_path, const FileData &data);

    bool save_mesh(const char *out_file_path, const FileData &data);

    bool save_obj(const char *out_file_path, const FileData &data);

    bool save_f3grid(const char *out_file_path, const FileData &data);

    bool save_vtu(const char *out_file_path, const FileData &data);

    bool vtk_to_off(const char *in_file_path, const char *out_file_path);

    bool vtk_to_obj(const char *in_file_path, const char *out_file_path);

    bool vtk_to_mesh(const char *in_file_path, const char *out_file_path);

    bool vtk_to_f3grid(const char *in_file_path, const char *out_file_path);

    bool vtk_to_vtu(const char *in_file_path, const char *out_file_path);

    bool f3grid_to_vtu(const char *in_file_path, const char *out_file_path);

    bool mesh_to_vtu(const char *in_file_path, const char *out_file_path);


}






