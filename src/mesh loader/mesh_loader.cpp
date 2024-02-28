
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

        data.numberOfPoints = numberofpoint;
        data.pointList = new double[numberofpoint * 3];
        for (int i = 0; i < numberofpoint; i++){

        }



            data.cellblocks.resize(numberofcelltypes);
        for (int i = 0; i < numberofcelltypes; i++) {
            auto &cellblock = data.cellblocks[i];
            unsigned char celltype = p_CellTypesArray->GetValue(i);
            if (celltype == 10) {
                //tete
                cellblock.numberOfCell = 0;
                for (int j = 0; j < numberofcell; j++) {
                    if (g->GetCellType(j) == 10)
                        cellblock.numberOfCell++;
                }
                cellblock.cellList = new Cell[cellblock.numberOfCell];
                for (int j = 0; j < numberofcell; j++) {
                    auto &cell = cellblock.cellList[j];
                    cell.numberOfPoints = 4;
                    cell.pointList = new int[4];
                    vtkIdType npts;
                    vtkIdType const * pts;
                    g->GetCellPoints(j, npts, pts);
                    cell.pointList[0] = pts[0];
                    cell.pointList[1] = pts[1];
                    cell.pointList[2] = pts[2];
                    cell.pointList[3] = pts[3];

                }


            } else if (celltype == 5) {
                //triangle
                //vtkCellTypes::GetClassNameFromTypeId
            } else {
                ASSERT_MSG(false, "ERROR: unsupport vtu cell type, currently only support tet and triangle!");
            }

            //cellblock.numberOfCell
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
        int ntetrahedras = 0, itetrahedras = 0, itetrahedrasattr = 0;
        int ntriangles = 0, itriangles = 0;

        int nFGROUP = 0;
        int nZGROUP = 0;
        while ((bufferp = read_line(buffer, fp, &line_count)) != NULL) {
            char string[15];
            sscanf(bufferp, "%s", &string);
            std::string sub03 = std::string(bufferp).substr(0, 4);
            std::string sub05 = std::string(bufferp).substr(0, 6);
            if (strcmp(string, "G") == 0) {
                nverts++;
            } else if (strcmp(sub03.c_str(), "Z T4") == 0) {
                ntetrahedras++;
            } else if (strcmp(sub03.c_str(), "F T3") == 0) {
                ntriangles++;
            } else if (strcmp(sub05.c_str(), "FGROUP") == 0) {
                nFGROUP++;
            } else if (strcmp(sub05.c_str(), "ZGROUP") == 0) {
                nZGROUP++;
            }
        }

        line_count = 0;
        fclose(fp);
        fp = fopen(in_file_path, "r");

        data.numberOfPoints = nverts;
        data.pointList = new double[nverts * 3];


        fclose(fp);
        return true;
    }


    bool save_vtu(const char *out_file_path, const FileData &data) {


        return true;
    }


}