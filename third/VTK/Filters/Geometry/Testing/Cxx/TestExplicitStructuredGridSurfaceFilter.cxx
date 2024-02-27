// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

// Description
// This test read a unstructured grid and creates a explicit grid using
// vtkUnstructuredGridToExplicitStructuredGrid

#include "vtkActor.h"
#include "vtkDataSetMapper.h"
#include "vtkExplicitStructuredGridSurfaceFilter.h"
#include "vtkNew.h"
#include "vtkProperty.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkTestUtilities.h"
#include "vtkTesting.h"
#include "vtkUnstructuredGridToExplicitStructuredGrid.h"
#include "vtkXMLUnstructuredGridReader.h"

int TestExplicitStructuredGridSurfaceFilter(int argc, char* argv[])
{
  vtkNew<vtkXMLUnstructuredGridReader> reader;
  char* fname = vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/explicitStructuredGrid.vtu");
  reader->SetFileName(fname);
  delete[] fname;
  reader->Update();

  vtkNew<vtkUnstructuredGridToExplicitStructuredGrid> esgConvertor;
  esgConvertor->SetInputConnection(reader->GetOutputPort());
  esgConvertor->SetWholeExtent(0, 5, 0, 13, 0, 3);
  esgConvertor->SetInputArrayToProcess(0, 0, 0, 1, "BLOCK_I");
  esgConvertor->SetInputArrayToProcess(1, 0, 0, 1, "BLOCK_J");
  esgConvertor->SetInputArrayToProcess(2, 0, 0, 1, "BLOCK_K");
  esgConvertor->Update();

  vtkNew<vtkExplicitStructuredGridSurfaceFilter> surf;
  surf->SetInputConnection(esgConvertor->GetOutputPort());
  surf->Update();

  vtkNew<vtkDataSetMapper> mapper;
  mapper->SetInputConnection(surf->GetOutputPort());
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetOpacity(0.5);
  vtkNew<vtkRenderer> ren;
  ren->AddActor(actor);

  vtkNew<vtkRenderWindow> renWin;
  renWin->SetSize(300, 300);
  renWin->AddRenderer(ren);
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  ren->ResetCamera();
  renWin->Render();

  int retVal = vtkRegressionTestImage(renWin);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
  {
    iren->Start();
  }
  return !retVal;
}
