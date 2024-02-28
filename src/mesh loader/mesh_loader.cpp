
#include <string.h>
#include <vector>
#include <set>
#include <map>
#include <bitset>

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

#include "utils/file/file_path.h"
#include "mesh_loader.h"
#include "utils/log/log.h"
#include "utils/string/string_utils.h"

#define  ASSERT_MSG(condition, msg) \
    if((condition) == false) { log_print(msg) ; assert(false);}

namespace Mesh_Loader {

    template<class TReader>
    vtkDataSet *ReadAnXMLFile(const char *fileName) {
        vtkSmartPointer<TReader> reader = vtkSmartPointer<TReader>::New();
        reader->SetFileName(fileName);
        reader->Update();
        reader->GetOutput()->Register(reader);
        return vtkDataSet::SafeDownCast(reader->GetOutput());
    }

    char *read_line(char *string, FILE *infile, int *linenumber, bool skip_sapce_and_tab = true) {
        char *result;

        // Search for a non-empty line.
        do {
            result = fgets(string, 2048, infile);
            if (linenumber) (*linenumber)++;
            if (result == (char *) NULL) {
                return (char *) NULL;
            }
            // Skip white spaces.
            if (skip_sapce_and_tab)
                while ((*result == ' ') || (*result == '\t')) result++;
            // If it's end of line, read another line and try again.
        } while ((*result == '\0') || (*result == '\r') || (*result == '\n'));
        return result;
    }

    bool load_vtu(const char *in_file_path, FileData &data) {
        //vtkDataSet *dataSet = ReadAnXMLFile<vtkXMLUnstructuredGridReader>(in_file_path);
        vtkSmartPointer<vtkXMLUnstructuredGridReader> reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
        reader->SetFileName(in_file_path);
        reader->Update();
        reader->GetOutput()->Register(reader);
        vtkUnstructuredGrid *g = reader->GetOutput();

        int numberofcell = g->GetNumberOfCells();
        int numberofpoint = g->GetNumberOfPoints();

        vtkCellTypes *p_vtkCellTypes = vtkCellTypes::New();
        g->GetCellTypes(p_vtkCellTypes);
        int numberofcelltypes = p_vtkCellTypes->GetNumberOfTypes();
        auto p_CellTypesArray = p_vtkCellTypes->GetCellTypesArray();

        //Point
        data.numberOfPoints = numberofpoint;
        data.pointList = new double[numberofpoint * 3];
        for (int i = 0; i < numberofpoint; i++) {
            g->GetPoint(i, &data.pointList[i]);
        }

        //Cell
        data.numberOfCell = numberofcell;
        data.cellList = new Cell[numberofcell];
        for (int i = 0; i < numberofcell; i++) {
            auto &cell = data.cellList[i];
            if (g->GetCellType(i) == 10) {
                cell.numberOfPoints = 4;
                cell.pointList = new int[4];
                vtkIdType npts;
                vtkIdType const *pts;
                g->GetCellPoints(i, npts, pts);
                assert(npts == 4);
                cell.pointList[0] = pts[0];
                cell.pointList[1] = pts[1];
                cell.pointList[2] = pts[2];
                cell.pointList[3] = pts[3];
            }
            else if (g->GetCellType(i) == 5) {
                //triangle
                //vtkCellTypes::GetClassNameFromTypeId
                cell.numberOfPoints = 3;
                cell.pointList = new int[3];
                vtkIdType npts;
                vtkIdType const *pts;
                g->GetCellPoints(i, npts, pts);
                assert(npts == 3);
                cell.pointList[0] = pts[0];
                cell.pointList[1] = pts[1];
                cell.pointList[2] = pts[2];
            }
            else {
                ASSERT_MSG(false, "ERROR: unsupport vtu cell type, currently only support tetrahedra and triangle!");
            }
        }

        //Cell Data
        vtkCellData *g_celldata = g->GetCellData();
        int array_number = g_celldata->GetNumberOfArrays();
        data.cellDataArray.resize(array_number);
        for (int i = 0; i < array_number; i++) {
            vtkAbstractArray *array = g_celldata->GetAbstractArray(i);
            //g_celldata->GetAbstractArray(0)->GetDataType () -> 13
            //g_celldata->GetAbstractArray(0)->GetNumberOfValues() ->1558755
            ASSERT_MSG(array->GetDataType() == 13, "RROR: unsupport vtu celldata type, currently only support string!")
            data.cellDataArray[i].name = array->GetName();
            std::vector<std::string> &string_array = data.cellDataArray[i].content;
            string_array.resize(array->GetNumberOfValues());
            for (int i = 0; i < g_celldata->GetAbstractArray(0)->GetNumberOfValues(); i++) {
                vtkStdString *vtkstring = (vtkStdString *) g_celldata->GetAbstractArray(0)->GetVoidPointer(i);
                string_array[i] = *vtkstring;
            }
        }
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
        int ntetrahedras = 0, icells = 0;
        int ntriangles = 0;

        struct Slot {
            //std::string slot_name;
            std::map<std::string, std::vector<int>> index_groups;
        };
        std::map<std::string, Slot> Z_slot_map;
        std::map<std::string, Slot> F_slot_map;

        while ((bufferp = read_line(buffer, fp, &line_count)) != NULL) {
            scan_begin:
            char string[15];
            sscanf(bufferp, "%s", &string);
            std::string sub03 = std::string(bufferp).substr(0, 4);
            std::string sub05 = std::string(bufferp).substr(0, 6);

            if (strcmp(string, "G") == 0) {
                nverts++;
            }
            else if (strcmp(sub03.c_str(), "Z T4") == 0) {
                ntetrahedras++;
            }
            else if (strcmp(sub03.c_str(), "F T3") == 0) {
                ntriangles++;
            }
            else if (strcmp(sub05.c_str(), "FGROUP") == 0) {
                char g_name[35];
                char slot_name[35];
                sscanf(bufferp, "FGROUP \"%s\" SLOT \"%s\"", &g_name, &slot_name);
                auto split_res = string_split(bufferp, " ");
                auto s_1 = string_shrink(split_res[1]);
                auto s_3 = string_shrink(split_res[3]);
                while ((bufferp = read_line(buffer, fp, &line_count, false)) != NULL) {
                    char char0;
                    sscanf(bufferp, "%c", &char0);
                    if (char0 != ' ') {
                        goto scan_begin;
                    }
                    auto split_res_internal = string_split(bufferp + 1, " ");
                    for (auto item: split_res_internal) {
                        int index;
                        sscanf(item.c_str(), "%d", &index);
                        F_slot_map[s_3].index_groups[s_1].push_back(index);
                    }
                }

            }
            else if (strcmp(sub05.c_str(), "ZGROUP") == 0) {
                char g_name[35];
                char slot_name[35];
                sscanf(bufferp, "ZGROUP \"%s\" SLOT \"%s\"", &g_name, &slot_name);
                auto split_res = string_split(bufferp, " ");
                auto s_1 = string_shrink(split_res[1]);
                auto s_3 = string_shrink(split_res[3]);
                while ((bufferp = read_line(buffer, fp, &line_count, false)) != NULL) {
                    char char0;
                    sscanf(bufferp, "%c", &char0);
                    if (char0 != ' ') {
                        goto scan_begin;
                    }
                    auto split_res_internal = string_split(bufferp + 1, " ");
                    for (auto item: split_res_internal) {
                        int index;
                        sscanf(item.c_str(), "%d", &index);
                        Z_slot_map[s_3].index_groups[s_1].push_back(index);
                    }
                }
            }
        }

        line_count = 0;
        fclose(fp);
        fp = fopen(in_file_path, "r");

        data.numberOfPoints = nverts;
        data.pointList = new double[nverts * 3];
        data.numberOfCell = ntetrahedras + ntriangles;
        data.cellList = new Cell[data.numberOfCell];

        data.cellDataArray.resize(2);

        while ((bufferp = read_line(buffer, fp, &line_count)) != NULL) {
            char string[15];
            char *ptr;
            char x[25], y[25], z[25];
            sscanf(bufferp, "%s", &string);
            std::string sub03 = std::string(bufferp).substr(0, 4);
            std::string sub05 = std::string(bufferp).substr(0, 6);
            if (strcmp(string, "G") == 0) {
                sscanf(bufferp, "%*s %*s %s %s %s", &x, &y, &z);
                double d1 = (double) strtod(x, &ptr);
                double d2 = (double) strtod(y, &ptr);
                double d3 = (double) strtod(z, &ptr);
                data.pointList[iverts * 3] = (double) strtod(x, &ptr);
                data.pointList[iverts * 3 + 1] = (double) strtod(y, &ptr);;
                data.pointList[iverts * 3 + 2] = (double) strtod(z, &ptr);;
                iverts++;
            }
            else if (strcmp(sub03.c_str(), "Z T4") == 0) {
                int p0, p1, p2, p3;
                sscanf(bufferp, "%*s %*s %*s %d %d %d %d",
                       &p0,
                       &p1,
                       &p2,
                       &p3
                );

                data.cellList[icells].pointList = new int[4];
                data.cellList[icells].numberOfPoints = 4;

                auto Cell_point = &data.cellList[icells].pointList[0];
                Cell_point[0] = p0 - 1;
                Cell_point[1] = p1 - 1;
                Cell_point[2] = p2 - 1;
                Cell_point[3] = p3 - 1;

                icells++;
            }
            else if (strcmp(sub03.c_str(), "F T3") == 0) {

                int p0, p1, p2;
                sscanf(bufferp, "%*s %*s %*s %d %d %d",
                       &p0,
                       &p1,
                       &p2
                );

                data.cellList[icells].pointList = new int[3];
                data.cellList[icells].numberOfPoints = 3;

                auto Cell_point = &data.cellList[icells].pointList[0];
                Cell_point[0] = p0 - 1;
                Cell_point[1] = p1 - 1;
                Cell_point[2] = p2 - 1;

                icells++;
            }
        }

        fclose(fp);


        log_print("* load_f3grid success!");
        log_print("* tetrahedra number: " + std::to_string(ntetrahedras));
        log_print("* triangle number: " + std::to_string(ntriangles));
        log_print("* ZGROUP SLOT number: " + std::to_string(Z_slot_map.size()));
        for (auto iter = Z_slot_map.begin(); iter != Z_slot_map.end(); iter++) {
            log_print("* ZGROUP SLOT name: " + iter->first, 2);
            for (auto iter_index_groups = iter->second.index_groups.begin(); iter_index_groups != iter->second.index_groups.end(); iter_index_groups++) {
                log_print("* ZGROUP in SLOT \"" + iter->first + " \" name: " + iter_index_groups->first, 3);
            }
        }
        log_print("* FGROUP SLOT number: " + std::to_string(F_slot_map.size()));
        for (auto iter = F_slot_map.begin(); iter != F_slot_map.end(); iter++) {
            log_print("* FGROUP SLOT name: " + iter->first, 2);
            for (auto iter_index_groups = iter->second.index_groups.begin(); iter_index_groups != iter->second.index_groups.end(); iter_index_groups++) {
                log_print("* FGROUP in SLOT \"" + iter->first + " \" name: " + iter_index_groups->first, 3);
            }
        }
        return true;
    }


    bool save_vtu(const char *out_file_path, const FileData &data) {


        return true;
    }


}