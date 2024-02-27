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

        int nFGROUP=0;
        int nZGROUP=0;
        while ((bufferp = read_line(buffer, fp, &line_count)) != NULL) {
            char string[15];
            sscanf(bufferp, "%s", &string);
            std::string sub03 = std::string(bufferp).substr(0,4);
            std::string sub05= std::string(bufferp).substr(0,6);
            if (strcmp(string, "G") == 0) {
                nverts++;
            } else if (strcmp(sub03.c_str(), "Z T4") == 0) {
                ntetrahedras++;
            }
            else if (strcmp(sub03.c_str(), "F T3") == 0) {
                ntriangles++;
            }
            else if (strcmp(sub05.c_str(), "FGROUP") == 0) {
                nFGROUP++;
            }
            else if (strcmp(sub05.c_str(), "ZGROUP") == 0) {
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