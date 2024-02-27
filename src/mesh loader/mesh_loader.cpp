#include "mesh_loader.h"
#include <string.h>
#include <vector>
#include <set>
#include <map>
#include <bitset>
#include "utils/file/file_path.h"

#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLCompositeDataReader.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLReader.h>
#include <vtkXMLRectilinearGridReader.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtksys/SystemTools.hxx>


namespace Mesh_Loader {

    template<class TReader>
    vtkDataSet *ReadAnXMLFile(const char *fileName) {
        vtkSmartPointer<TReader> reader = vtkSmartPointer<TReader>::New();
        reader->SetFileName(fileName);
        reader->Update();
        reader->GetOutput()->Register(reader);
        return vtkDataSet::SafeDownCast(reader->GetOutput());
    }

    char *read_line(char *string, FILE *infile, int *linenumber) {
        char *result;

        // Search for a non-empty line.
        do {
            result = fgets(string, 2048, infile);
            if (linenumber) (*linenumber)++;
            if (result == (char *) NULL) {
                return (char *) NULL;
            }
            // Skip white spaces.
            while ((*result == ' ') || (*result == '\t')) result++;
            // If it's end of line, read another line and try again.
        } while ((*result == '\0') || (*result == '\r') || (*result == '\n'));
        return result;
    }

    bool load_by_extension(const char *in_file_path, FileData &data) {
        auto extension = get_file_extension(in_file_path);

        auto hash_hit = [](std::string inString) {
            if (inString == "obj") return OBJ;
            if (inString == "vtk") return VTK;
            if (inString == "mesh") return MESH;
            if (inString == "vtu") return MESH;
            return UNKNOW;
        };

        switch (hash_hit(extension)) {
            case OBJ:
                return load_obj(in_file_path, data);

            case VTK:
                return load_vtk(in_file_path, data);

            case VTU:
                return load_vtu(in_file_path, data);

            default:
                assert(false);

        }

        return false;
    }

    bool load_vtk(const char *in_file_path, FileData &data) {
        FILE *fp = fopen(in_file_path, "r");
        if (fp == (FILE *) NULL) {
            //printf("File I/O Error:  Cannot create file %s.\n", vtk_file_path);
            return false;
        }


        char buffer[2048];
        char *bufferp;
        int line_count = 0;


        int nverts = 0, iverts = 0;
        int ntetrahedras = 0, itetrahedras = 0, itetrahedrasattr = 0;
        bool readattr = false;

        while ((bufferp = read_line(buffer, fp, &line_count)) != NULL) {
            if (nverts == 0) {
                read_line(buffer, fp, &line_count); //Unstructured Grid
                read_line(buffer, fp, &line_count); //ASCII
                read_line(buffer, fp, &line_count); //DATASET UNSTRUCTURED_GRID
                read_line(buffer, fp, &line_count); //POINTS xxxx double
                sscanf(bufferp, "%*s %d %*s", &nverts);
                if (nverts < 3) {
                    //printf("Syntax error reading header on line %d in file %s\n",
                    //	line_count, vtk_file_path);
                    fclose(fp);
                    return false;
                }
                data.numberOfPoints = nverts;
                data.pointList = new double[nverts * 3];
            } else if (nverts > iverts) {
                data.pointList[iverts * 3] = (double) strtod(bufferp, &bufferp);
                data.pointList[iverts * 3 + 1] = (double) strtod(bufferp, &bufferp);
                data.pointList[iverts * 3 + 2] = (double) strtod(bufferp, &bufferp);

                iverts++;
            } else if (ntetrahedras == 0) {
                //CELLS 35186 175930
                sscanf(bufferp, "%*s %d %*d", &ntetrahedras);
                data.cellList = new Cell[ntetrahedras];
                data.numberOfCell = ntetrahedras;
                data.cellattrname.push_back("default");
            } else if (ntetrahedras > itetrahedras) {
                int p0, p1, p2, p3;
                sscanf(bufferp, "%*d %d %d %d %d",
                       &p0,
                       &p1,
                       &p2,
                       &p3
                );


                data.cellList[itetrahedras].pointList = new int[4];
                data.cellList[itetrahedras].numberOfPoints = 4;

                auto Cell_point = &data.cellList[itetrahedras].pointList[0];
                Cell_point[0] = p0;
                Cell_point[1] = p1;
                Cell_point[2] = p2;
                Cell_point[3] = p3;

                itetrahedras++;
            } else if (!readattr) {
                char s[20];
                sscanf(bufferp, "%s", &s);
                if (strcmp(s, "CELL_DATA") == 0) {
                    readattr = true;
                    read_line(buffer, fp, &line_count); //SCALARS cell_scalars int 1
                    read_line(buffer, fp, &line_count); //LOOKUP_TABLE default
                }
            } else {
                int attr;
                sscanf(bufferp, "%d", &attr);
                data.cellList[itetrahedrasattr].cellattr = (double *) malloc(sizeof(double) * 1);

                *data.cellList[itetrahedrasattr].cellattr = attr;
                itetrahedrasattr++;
            }
        }
        fclose(fp);
        return true;
    }

    bool load_vtu(const char *in_file_path, FileData &data) {

        vtkDataSet *dataSet = ReadAnXMLFile<vtkXMLUnstructuredGridReader>(in_file_path);
        FILE *fp = fopen(in_file_path, "r");
        if (fp == (FILE *) NULL) {
            //printf("File I/O Error:  Cannot create file %s.\n", vtk_file_path);
            return false;
        }
        fclose(fp);
        int numberOfCells = dataSet->GetNumberOfCells();
        int numberOfPoints = dataSet->GetNumberOfPoints();
        if (numberOfPoints == 0)
            return false;



        return true;
    }

    bool load_obj(const char *in_file_path, FileData &data) {
        FILE *fp = fopen(in_file_path, "r");
        if (fp == (FILE *) NULL) {
            //printf("File I/O Error:  Cannot create file %s.\n", vtk_file_path);
            return false;
        }


        char buffer[2048];
        char *bufferp;
        int line_count = 0;

        int nverts = 0, iverts = 0;
        int ncells = 0, icells = 0;


        while ((bufferp = read_line(buffer, fp, &line_count)) != NULL) {
            char string[15];
            sscanf(bufferp, "%s", &string);
            if (strcmp(string, "v") == 0) {
                nverts++;
            } else if (strcmp(string, "f") == 0) {
                ncells++;
            }
        }

        line_count = 0;
        fclose(fp);
        fp = fopen(in_file_path, "r");

        data.numberOfPoints = nverts;
        data.pointList = new double[nverts * 3];
        data.cellList = new Cell[ncells];
        data.numberOfCell = ncells;

        while ((bufferp = read_line(buffer, fp, &line_count)) != NULL) {
            char string[15];
            char x[25], y[25], z[25];
            char *ptr;
            sscanf(bufferp, "%s", &string);
            if (strcmp(string, "v") == 0) {
                sscanf(bufferp, "%*s %s %s %s", &x, &y, &z);
                double d1 = (double) strtod(x, &ptr);
                double d2 = (double) strtod(y, &ptr);
                double d3 = (double) strtod(z, &ptr);
                data.pointList[iverts * 3] = (double) strtod(x, &ptr);
                data.pointList[iverts * 3 + 1] = (double) strtod(y, &ptr);;
                data.pointList[iverts * 3 + 2] = (double) strtod(z, &ptr);;
                iverts++;
            } else if (strcmp(string, "f") == 0) {
                char x[35], y[35], z[35];
                int p0, p1, p2, p3;
                sscanf(bufferp, "%*s %s %s %s",
                       &x,
                       &y,
                       &z
                );

                auto string_split = [](std::string s, std::string delimiter) {
                    std::vector<std::string> res;
                    int index = 0;

                    while (true) {
                        int find_index = s.find(delimiter, index);
                        if (find_index == -1) {
                            res.push_back(s.substr(index, s.size() - index));
                            break;
                        }
                        res.push_back(s.substr(index, find_index - index));
                        index = find_index + 1;
                    }
                    return res;
                };

                data.cellList[icells].pointList = new int[3];
                data.cellList[icells].numberOfPoints = 3;

                auto Cell_point = &data.cellList[icells].pointList[0];
                Cell_point[0] = (int) strtod(string_split(x, "/")[0].c_str(), &ptr) - 1;
                Cell_point[1] = (int) strtod(string_split(y, "/")[0].c_str(), &ptr) - 1;
                Cell_point[2] = (int) strtod(string_split(z, "/")[0].c_str(), &ptr) - 1;

                icells++;
            }
        }

        fclose(fp);
        return true;
    }

    bool load_f3grid(const char *in_file_path, FileData &data) {
        FILE *fp = fopen(in_file_path, "r");
        if (fp == (FILE *) NULL) {
            //printf("File I/O Error:  Cannot create file %s.\n", vtk_file_path);
            return false;
        }


        char buffer[2048];
        char *bufferp;
        int line_count = 0;


        int nverts = 0, iverts = 0;
        int ntetrahedras = 0, itetrahedras = 0, itetrahedrasattr = 0;
        bool readattr = false;

        while ((bufferp = read_line(buffer, fp, &line_count)) != NULL) {
            char string[15];
            sscanf(bufferp, "%s", &string);
            if (strcmp(string, "G") == 0) {
                nverts++;
            } else if (strcmp(string, "Z") == 0) {
                ntetrahedras++;
            }
        }

        line_count = 0;
        fclose(fp);
        fp = fopen(in_file_path, "r");

        data.numberOfPoints = nverts;
        data.pointList = new double[nverts * 3];
        data.cellList = new Cell[ntetrahedras];
        data.numberOfCell = ntetrahedras;

        while ((bufferp = read_line(buffer, fp, &line_count)) != NULL) {
            char string[15];
            char x[25], y[25], z[25];
            char *ptr;
            sscanf(bufferp, "%s", &string);
            if (strcmp(string, "G") == 0) {
                sscanf(bufferp, "%*s %*s %s %s %s", &x, &y, &z);
                double d1 = (double) strtod(x, &ptr);
                double d2 = (double) strtod(y, &ptr);
                double d3 = (double) strtod(z, &ptr);
                data.pointList[iverts * 3] = (double) strtod(x, &ptr);
                data.pointList[iverts * 3 + 1] = (double) strtod(y, &ptr);;
                data.pointList[iverts * 3 + 2] = (double) strtod(z, &ptr);;
                iverts++;
            } else if (strcmp(string, "Z") == 0) {

                int p0, p1, p2, p3;
                sscanf(bufferp, "%*s %*s %*s %d %d %d %d",
                       &p0,
                       &p1,
                       &p2,
                       &p3
                );

                data.cellList[itetrahedras].pointList = new int[4];
                data.cellList[itetrahedras].numberOfPoints = 4;

                auto Cell_point = &data.cellList[itetrahedras].pointList[0];
                Cell_point[0] = p0 - 1;
                Cell_point[1] = p1 - 1;
                Cell_point[2] = p2 - 1;
                Cell_point[3] = p3 - 1;

                itetrahedras++;
            }
        }

        int aa = 0;
        fclose(fp);
        return true;
    }

    bool load_mesh(const char *in_file_path, FileData &data) {
        FILE *fp = fopen(in_file_path, "r");
        if (fp == (FILE *) NULL) {
            //printf("File I/O Error:  Cannot create file %s.\n", vtk_file_path);
            return false;
        }

        char buffer[2048];
        char *bufferp;
        int line_count = 0;

        int nverts = 0, iverts = 0;
        int ntetrahedras = 0, itetrahedras = 0, itetrahedrasattr = 0;
        bool read_vtx_begin = false;
        bool read_tet_begin = false;

        while ((bufferp = read_line(buffer, fp, &line_count)) != NULL) {
            if (read_vtx_begin && nverts > iverts) {
                data.pointList[iverts * 3] = (double) strtod(bufferp, &bufferp);
                data.pointList[iverts * 3 + 1] = (double) strtod(bufferp, &bufferp);
                data.pointList[iverts * 3 + 2] = (double) strtod(bufferp, &bufferp);
                iverts++;
                continue;
            } else if (read_tet_begin && ntetrahedras > itetrahedras) {
                int p0, p1, p2, p3;
                int type;
                sscanf(bufferp, "%d %d %d %d %d",
                       &p0,
                       &p1,
                       &p2,
                       &p3,
                       &type
                );

                data.cellList[itetrahedras].pointList = new int[4];
                data.cellList[itetrahedras].numberOfPoints = 4;

                auto Cell_point = &data.cellList[itetrahedras].pointList[0];
                Cell_point[0] = p0 - 1;
                Cell_point[1] = p1 - 1;
                Cell_point[2] = p2 - 1;
                Cell_point[3] = p3 - 1;

                data.cellList[itetrahedras].cellattr = new double[1];
                data.cellList[itetrahedras].cellattr[0] = type;
                itetrahedras++;
                continue;
            }


            char s[500];
            sscanf(bufferp, "%s", &s);
            if (strcmp(s, "Vertices") == 0) {
                read_vtx_begin = true;
                read_line(buffer, fp, &line_count);
                sscanf(bufferp, "%d", &nverts);
                data.numberOfPoints = nverts;
                data.pointList = new double[nverts * 3];
                data.cellattrname.push_back("type");
            } else if (strcmp(s, "Tetrahedra") == 0) {
                read_tet_begin = true;
                read_line(buffer, fp, &line_count);
                sscanf(bufferp, "%d", &ntetrahedras);
                data.cellList = new Cell[ntetrahedras];
                data.numberOfCell = ntetrahedras;
            }
        }
        fclose(fp);
        return true;
    }

    bool save_off(const char *out_file_path, const FileData &data) {

        FILE *fout = fopen(out_file_path, "w");

        if (fout == (FILE *) NULL) {
            //printf("File I/O Error:  Cannot create file %s.\n", vtk_file_path);

            return false;
        }
        fprintf(fout, "OFF\n");
        fprintf(fout, "%d  %d  %d\n", data.numberOfPoints, data.numberOfCell, 0);
        for (int i = 0; i < data.numberOfPoints; i++) {
            fprintf(fout, "%.16g %.16g %.16g\n", data.pointList[i * 3], data.pointList[i * 3 + 1],
                    data.pointList[i * 3 + 2]);
        }
        for (int i = 0; i < data.numberOfCell; i++) {
            auto &face = data.cellList[i];
            assert(face.numberOfPoints == 3);

            fprintf(fout, "%d %d %d %d\n", 3, face.pointList[0], face.pointList[1], face.pointList[2]);
        }
        fclose(fout);
        return true;
    }

    bool save_mesh(const char *out_file_path, const FileData &data) {

        FILE *fout = fopen(out_file_path, "w");

        if (fout == (FILE *) NULL) {
            //printf("File I/O Error:  Cannot create file %s.\n", vtk_file_path);

            return false;
        }
        fprintf(fout, "MeshVersionFormatted 1\n");
        fprintf(fout, "Dimension 3\n");
        fprintf(fout, "Vertices\n");
        fprintf(fout, "%d\n", data.numberOfPoints);
        for (int i = 0; i < data.numberOfPoints; i++) {
            fprintf(fout, "%.16g %.16g %.16g %d\n", data.pointList[i * 3], data.pointList[i * 3 + 1], data.pointList[i * 3 + 2], 0);
        }
        fprintf(fout, "Tetrahedra\n");
        fprintf(fout, "%d\n", data.numberOfCell);
        assert(data.cellattrname.size() == 1);
        for (int i = 0; i < data.numberOfCell; i++) {
            auto &face = data.cellList[i];
            assert(face.numberOfPoints == 4);

            fprintf(fout, "%d %d %d %d %f\n", face.pointList[0] + 1, face.pointList[1] + 1, face.pointList[2] + 1, face.pointList[3] + 1, face.cellattr[0]);
        }
        fclose(fout);
        return true;
    }

    bool save_obj(const char *out_file_path, const FileData &data) {

        FILE *fout = fopen(out_file_path, "w");

        if (fout == (FILE *) NULL) {
            //printf("File I/O Error:  Cannot create file %s.\n", vtk_file_path);

            return false;
        }


        for (int i = 0; i < data.numberOfPoints; i++) {
            fprintf(fout, "%c %.16g %.16g %.16g\n", 'v', data.pointList[i * 3], data.pointList[i * 3 + 1],
                    data.pointList[i * 3 + 2]);
        }
        for (int i = 0; i < data.numberOfCell; i++) {
            auto &face = data.cellList[i];
            assert(face.numberOfPoints == 3);

            fprintf(fout, "%c %d %d %d\n", 'f', face.pointList[0] + 1, face.pointList[1] + 1, face.pointList[2] + 1);
        }
        fclose(fout);
        return true;
    }

    bool save_f3grid(const char *out_file_path, const FileData &data) {

        FILE *fout = fopen(out_file_path, "w");

        if (fout == (FILE *) NULL) {
            //printf("File I/O Error:  Cannot create file %s.\n", vtk_file_path);
            return false;
        }

        fprintf(fout, "* GRIDPOINTS\n");
        for (int i = 0; i < data.numberOfPoints; i++) {
            fprintf(fout, "%c %d %.16g %.16g %.16g\n", 'G', i + 1, data.pointList[i * 3], data.pointList[i * 3 + 1],
                    data.pointList[i * 3 + 2]);
        }
        fprintf(fout, "* ZONES\n");
        for (int i = 0; i < data.numberOfCell; i++) {
            auto &cell = data.cellList[i];
            assert(cell.numberOfPoints == 4);

            fprintf(fout, "%c T4 %d %d %d %d %d\n", 'Z', i + 1, cell.pointList[0] + 1, cell.pointList[1] + 1,
                    cell.pointList[2] + 1, cell.pointList[3] + 1);
        }
        fprintf(fout, "* ZONE GROUPS\n");
        std::vector<std::vector<int>> all_zone_group(100);

        for (int i = 0; i < data.numberOfCell; i++) {
            auto &cell = data.cellList[i];
            int g_id = cell.cellattr[0] - 1;
            all_zone_group[g_id].push_back(i + 1);
        }

        for (int i = 0; i < all_zone_group.size(); i++) {
            auto g = all_zone_group[i];
            if (g.size() != 0) {
                fprintf(fout, "ZGROUP \"ZG_00%d\" SLOT 1 \n", i);

                for (int j = 0; j < g.size(); j++) {
                    fprintf(fout, "%d ", g[j]);
                }

                fprintf(fout, "\n");
            }
        }
        fclose(fout);
        return true;
    }

    bool save_vtu(const char *out_file_path, const FileData &data) {

        auto base64_decode = [](char *str) {
            static const std::map<char, std::bitset<6>> base64_alphabet = {
                    {'A', std::bitset<6>(std::string("0101111001"))},

            };


        };
        FILE *fout = fopen(out_file_path, "w");

        if (fout == (FILE *) NULL) {
            //printf("File I/O Error:  Cannot create file %s.\n", vtk_file_path);
            return false;
        }

        /*
         <VTKFile type="StructuredGrid" ...>
          <StructuredGrid WholeExtent="x1 x2 y1 y2 z1 z2">
            <Piece Extent="x1 x2 y1 y2 z1 z2">
            <PointData>...</PointData>
            <CellData>...</CellData>
            <Points>...</Points>
            </Piece>
          </StructuredGrid>
        </VTKFile>
         */


        fprintf(fout, "<?xml version=\"1.0\"?>\n");
        fprintf(fout,
                "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n");
        fprintf(fout, "<UnstructuredGrid>\n");
        {
            fprintf(fout, "<Piece NumberOfPoints=\"%d\" NumberOfCells=\"%d\">\n", data.numberOfPoints,
                    data.numberOfCell);
            {
                //points
                {
                    fprintf(fout, "<Points>\n");
                    {
                        fprintf(fout,
                                "<DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">\n");
                        {
                            for (int i = 0; i < data.numberOfPoints; i++)
                                fprintf(fout, "%.16g %.16g %.16g\n", data.pointList[i * 3], data.pointList[i * 3 + 1],
                                        data.pointList[i * 3 + 2]);
                        }
                        fprintf(fout, "</DataArray>\n");
                    }
                    fprintf(fout, "</Points>\n");
                }
                //cells
                {
                    fprintf(fout, "<Cells>\n");
                    {
                        //connectivity
                        {
                            fprintf(fout, "<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n");
                            {
                                for (int i = 0; i < data.numberOfCell; i++) {
                                    auto &cell = data.cellList[i];
                                    //assert(cell.numberOfPoints == 4);
                                    if (cell.numberOfPoints == 4) {
                                        fprintf(fout, "%d %d %d %d\n", cell.pointList[0], cell.pointList[1],
                                                cell.pointList[2], cell.pointList[3]);
                                    } else if (cell.numberOfPoints == 3) {
                                        fprintf(fout, "%d %d %d\n", cell.pointList[0], cell.pointList[1],
                                                cell.pointList[2]);
                                    } else {
                                        assert(false);
                                    }
                                }
                            }
                            fprintf(fout, "</DataArray>\n");
                        }
                        //offsets
                        {
                            fprintf(fout, "<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n");
                            {

                                int next = 0;
                                for (int i = 0; i < data.numberOfCell; i++) {

                                    auto &cell = data.cellList[i];
                                    next += cell.numberOfPoints;
                                    fprintf(fout, "%d\n", next);
                                }
                            }
                            fprintf(fout, "</DataArray>\n");
                        }
                        //types
                        {
                            fprintf(fout, "<DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n");
                            {
                                for (int i = 0; i < data.numberOfCell; i++) {
                                    auto &cell = data.cellList[i];
                                    if (cell.numberOfPoints == 4)
                                        fprintf(fout, "%d\n", 10);
                                    else if (cell.numberOfPoints == 3)
                                        fprintf(fout, "%d\n", 5);
                                    else
                                        assert(false);
                                }
                            }
                            fprintf(fout, "</DataArray>\n");
                        }
                    }
                    fprintf(fout, "</Cells>\n");
                }
                //pointdata
                {
                    fprintf(fout, "<PointData>\n");
                    {
                        for (int attr_index = 0; attr_index < data.pointattrname.size(); attr_index++) {

                            fprintf(fout, " <DataArray type=\"UInt64\" Name=\"%s\" format=\"ascii\">\n",
                                    data.pointattrname[attr_index].c_str());
                            {
                                for (int i = 0; i < data.numberOfPoints; i++) {
                                    auto &attr = data.pointattr[i * data.pointattrname.size() + attr_index];
                                    fprintf(fout, "%d\n", attr);
                                }
                            }
                            fprintf(fout, "</DataArray>\n");
                        }
                    }
                    fprintf(fout, "</PointData>\n");
                }
                //celldata
                {
                    fprintf(fout, "<CellData>\n");
                    {
                        for (int attr_index = 0; attr_index < data.cellattrname.size(); attr_index++) {
                            //assert(data.cellattrname.size() == 1);

                            fprintf(fout, " <DataArray type=\"Float64\" Name=\"%s\" format=\"ascii\">\n",
                                    data.cellattrname[attr_index].c_str());
                            {
                                for (int i = 0; i < data.numberOfCell; i++) {
                                    auto &cell = data.cellList[i];
                                    fprintf(fout, "%lf\n", cell.cellattr[attr_index]);
                                }
                            }
                            fprintf(fout, "</DataArray>\n");
                        }

                    }
                    fprintf(fout, "</CellData>\n");
                }
            }
            fprintf(fout, "</Piece>\n");
        }
        fprintf(fout, "</UnstructuredGrid>\n");
        fprintf(fout, "</VTKFile>\n");
        fclose(fout);
        return true;
    }

//public function
    bool vtk_to_off(const char *in_file_path, const char *out_file_path) {
        FileData data;
        //load
        bool res = load_vtk(in_file_path, data);
        //save
        if (res)
            res = save_off(out_file_path, data);
        return res;
    }

    bool vtk_to_mesh(const char *in_file_path, const char *out_file_path) {
        FileData data;
        //load
        bool res = load_vtk(in_file_path, data);
        //save
        if (res)
            res = save_mesh(out_file_path, data);
        return res;
    }

    bool vtk_to_obj(const char *in_file_path, const char *out_file_path) {
        FileData data;
        //load
        bool res = load_vtk(in_file_path, data);

        if (res)
            //save
            res = save_obj(out_file_path, data);

        return res;
    }

    bool vtk_to_f3grid(const char *in_file_path, const char *out_file_path) {
        FileData data;
        //load
        bool res = load_vtk(in_file_path, data);

        //save
        if (res)
            res = save_f3grid(out_file_path, data);

        return res;
    }

    bool vtk_to_vtu(const char *in_file_path, const char *out_file_path) {
        FileData data;
        //load
        bool res = load_vtk(in_file_path, data);

        //save
        if (res)
            res = save_vtu(out_file_path, data);

        return res;
    }

    bool f3grid_to_vtu(const char *in_file_path, const char *out_file_path) {
        FileData data;
        //load
        bool res = load_f3grid(in_file_path, data);

        //save
        if (res)
            res = save_vtu(out_file_path, data);

        return res;
    }

    bool mesh_to_vtu(const char *in_file_path, const char *out_file_path) {
        FileData data;
        //load
        bool res = load_mesh(in_file_path, data);

        //save
        if (res)
            res = save_vtu(out_file_path, data);

        return res;
    }
}