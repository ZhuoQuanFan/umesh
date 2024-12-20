
/* iw - quick tool to convert from vtk vtu format to ugrid32 format */

#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellData.h>
#include <vtkPointData.h>

#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkNamedColors.h>

std::vector<double> vertex;
std::vector<double> perCellValue;
std::vector<size_t> hex_index;

#ifndef PRINT
# define PRINT(var) std::cout << #var << "=" << var << std::endl;
# define PING std::cout << __FILE__ << "::" << __LINE__ << ": " << __PRETTY_FUNCTION__ << std::endl;
#endif


void readFile(const std::string fileName)
{
  std::cout << "parsing vtu file " << fileName << std::endl;
  //read all the data from the file
  vtkSmartPointer<vtkXMLUnstructuredGridReader> reader =
    vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
  reader->SetFileName(fileName.c_str());
  reader->Update();

  vtkUnstructuredGrid *grid = reader->GetOutput();
  
  vtkPointData* pointData = grid->GetPointData();
  size_t firstIndexThisVTU = vertex.size() / 3;
  
  // ==================================================================
  const int numPoints = grid->GetNumberOfPoints();
  std::cout << " - found " << numPoints << " points" << std::endl;
  for (int pointID=0;pointID<numPoints;pointID++) {
    double point[3];
    grid->GetPoint(pointID,point);
    for (int i=0;i<3;i++)
      vertex.push_back(point[i]);
  }

  // ==================================================================
  const int numCells = grid->GetNumberOfCells();
  std::cout << " - found " << numCells << " cells" << std::endl;
  for (int cellID=0;cellID<numCells;cellID++) {
    vtkIdType cellPoints;
    const vtkIdType *pointIDs;
    grid->GetCellPoints(cellID,cellPoints,pointIDs);

    if (cellPoints == 8) {
      for (int i=0;i<cellPoints;i++) {
        int vtxID = pointIDs[i];
        if (vtxID < 0 || vtxID >= numPoints) {
          PING;
          PRINT(vtxID);
          for (int j=0;j<8;j++)
            PRINT(pointIDs[j]);
          exit(0);
        }
        hex_index.push_back(firstIndexThisVTU + pointIDs[i]);
      }
    } else
      throw std::runtime_error("unsupported number of points per cell : "
                               +std::to_string((int)numPoints));
    
    // std::cout << " cell N=" << numPoints << " { ";
    // for (int i = 0; i<numPoints; i++)
    //   std::cout << pointIDs[i] << " ";
    // std::cout << "}" << std::endl;
  }

  vtkCellData* cellData = grid->GetCellData();
  if (!cellData)
    throw std::runtime_error("could not read cell data ....");
  
  vtkDataArray *dataArray = cellData->GetArray(0);
  if (!dataArray)
    throw std::runtime_error("could not read data array from cell data");
  for (int i=0;i<numCells;i++)
    perCellValue.push_back(dataArray->GetTuple1(i));

  std::cout << "-------------------------------------------------------" << std::endl;
  std::cout << "done reading " << fileName << " : "
            << std::endl << "  " << (vertex.size()/3) << " vertices "
            << std::endl << "  " << (hex_index.size()/8) << " hexes" << std::endl;

  

}



int main ( int argc, char *argv[] )
{
  std::string outFileName = "";
  std::vector<std::string> inFileNames;
  size_t maxFiles = 1ULL<<60;
  
  for (int i=1;i<argc;i++) {
    const std::string arg = argv[i];
    if (arg == "-o")
      outFileName = argv[++i];
    else if (arg == "--max-files")
      maxFiles = atol(argv[++i]);
    else
      inFileNames.push_back(arg);
  }
  
  if(inFileNames.empty() || outFileName.empty())
  {
    std::cerr << "Usage: " << argv[0] << " -o outfile <infiles.vtu>+" << std::endl;
    return EXIT_FAILURE;
  }

  if (inFileNames.size() > maxFiles)
    inFileNames.resize(maxFiles);
  
  for (auto fn : inFileNames)
    readFile(fn);

  std::cout << "=======================================================" << std::endl;
  std::cout << "computing per-vertex data from per cell data ..." << std::endl;
  std::cout << "=======================================================" << std::endl;

  int numVertices = vertex.size() / 3;
  std::vector<float> perVertexValue(numVertices);
  std::vector<float> perVertexCount(numVertices);
  for (int i=0;i<hex_index.size();i++) {
    int cellID = i/8;
    int vtxID = hex_index[i];
    if (vtxID == -1) continue;
    if (vtxID < 0 || vtxID >= perVertexValue.size()) {
      PING;
      PRINT(vtxID);
      PRINT(perVertexValue.size());
    }
    float cellValue = perCellValue[cellID];
    perVertexValue[vtxID] += cellValue;
    perVertexCount[vtxID] += 1.f;
  }
  for (int i=0;i<numVertices;i++)
    perVertexValue[i] /= (perVertexCount[i] + 1e-20f);

  struct {
    //size_t
    uint32_t
    n_verts, n_tris, n_quads, n_tets, n_pyrs, n_prisms, n_hexes;
  } header;
  header.n_verts  = numVertices;
  header.n_tris   = 0;
  header.n_quads  = 0;
  header.n_tets   = 0;
  header.n_pyrs   = 0;
  header.n_prisms = 0;
  header.n_hexes  = hex_index.size()/8;

  std::cout << "=======================================================" << std::endl;
  std::cout << "writing out result ..." << std::endl;
  std::cout << "=======================================================" << std::endl;
  std::ofstream out(outFileName+".ugrid32");
  out.write((char*)&header,sizeof(header));
  out.write((char*)vertex.data(),vertex.size()*sizeof(vertex[0]));

  for (auto &idx : hex_index) idx += 1;
  out.write((char*)hex_index.data(),hex_index.size()*sizeof(hex_index[0]));
  
  std::ofstream value_out(outFileName+".scalar");
  value_out.write((char*)perVertexValue.data(),perVertexValue.size()*sizeof(perVertexValue[0]));
  
  return EXIT_SUCCESS;
}
