#include <vtkXMLUnstructuredGridReader.h>
#include <vtkSmartPointer.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>

#include <ecl_kw.hpp>
#include <fortio.hpp>



EclKW getKW(vtkCellData * cellData , std::string sourceName , std::string targetName , ecl_type_enum type) {
  vtkDataArray * dataArray = cellData->GetArray( sourceName.c_str() );
  if (dataArray) {
    vtkFloatArray * floatArray = vtkFloatArray::SafeDownCast( dataArray );
    
    return EclKW::create( targetName.c_str() , floatArray->GetSize() , type , floatArray->GetPointer(0));
  } 
}





int main ( int argc, char *argv[] )
{
  //parse command line arguments
  if(argc != 2)
    {
      std::cerr << "Usage: " << argv[0]
                << " Filename(.vtu)" << std::endl;
      return EXIT_FAILURE;
    }

  vtkDataSet * dataSet;
  std::string filename = argv[1];
 
  //read all the data from the file
  vtkSmartPointer<vtkXMLUnstructuredGridReader> reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
  reader->SetFileName(filename.c_str());
  reader->Update();
  reader->GetOutput()->Register(reader);
  
  dataSet = vtkDataSet::SafeDownCast(reader->GetOutput());
  
  std::cout << "Number of cells: " << dataSet->GetNumberOfCells() << std::endl;
  std::cout << "Classname: " << dataSet->GetClassName() << std::endl;
  
  // Now check for cell data

  vtkCellData *cd = dataSet->GetCellData();
  if (cd)
    {
      FortIO writer = FortIO::writer("/tmp/file" , true);
      std::cout << " contains cell data with "
                << cd->GetNumberOfArrays()
                << " arrays." << std::endl;
      
      {
        EclKW poro  = getKW( cd , "PORO"  , "PORO"  , ECL_FLOAT_TYPE );
        EclKW permx = getKW( cd , "PERMX" , "PERMX" , ECL_FLOAT_TYPE );
        EclKW permy = getKW( cd , "PERMY" , "PERMY" , ECL_FLOAT_TYPE );
        EclKW permz = getKW( cd , "PERMZ" , "PERMZ" , ECL_FLOAT_TYPE );
      }

      for (int i = 0; i < cd->GetNumberOfArrays(); i++)
        {
          vtkFloatArray * array = vtkFloatArray::SafeDownCast( cd->GetArray(i));
          float * data;

          if (!array)
            std::cout << "SafeDownCast() failed" << std::endl;
        
          std::cout << "\tArray " << i
                    << " is named "
                    << (cd->GetArrayName(i) ? cd->GetArrayName(i) : "NULL")
                    << "Value: "
                    << array->GetTuple1( 0 );
          

          array->SetTuple1( 0 , 0.25 );
          std::cout << "  Updated value: " << array->GetTuple1( 0 ) << std::endl;

          data = array->GetPointer( 0 );
          {
            int i;
            for (i=0; i < 10; i++)
              std::cout << "  data[" << i << "] = " << data[i] << std::endl;
          }
          {
            EclKW kw = EclKW::wrap_data("KJELL" , array->GetSize( ) , ECL_FLOAT_TYPE , data);
             
              
            kw.fwrite( writer );
          }
        }
      writer.close();
    }
  
  return EXIT_SUCCESS;
}


