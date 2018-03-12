#ifdef USE_HDF5

#include "gtest/gtest.h"

#include "H5Cpp.h"
#include <iostream>

#include <H5Exception.h>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_HDFTests, BasicFileRead)
{
    std::string file_path = "D:/ResInsight/SourSim/PKMUNK_NOV_TEST_SS.sourpre.00001";

    try
    {
        H5::Exception::dontPrint();                                // Turn off auto-printing of failures to handle the errors appropriately

        H5::H5File file(file_path.c_str(), H5F_ACC_RDONLY);

        {
            H5::Group        timestep = file.openGroup("Timestep_00001");
            H5::Attribute    attr     = timestep.openAttribute("timestep");

            double timestep_value = 0.0;

            H5::DataType type = attr.getDataType();
            attr.read(type, &timestep_value);

            //std::cout << "Timestep value " << timestep_value << std::endl;
            EXPECT_NEAR(timestep_value, 1.0, 1e-1);
        }


        {
            // Group size is not an attribute!

            H5::Group GridFunctions = file.openGroup("Timestep_00001/GridParts/GridPart_00000/GridFunctions");

            hsize_t   group_size = GridFunctions.getNumObjs();

            //std::cout << "GridFunctions group_size " << group_size << std::endl;
            EXPECT_EQ(size_t(20), group_size);

/*            for (hsize_t i = 0; i < group_size; i++)
            {
                // H5std_string node_name = GridFunctions.getObjnameByIdx(i);     // crashes on VS2017 due to lib/heap/runtime differences to HDF5 VS2015 lib

                std::string node_name;
                node_name.resize(1024);

                ssize_t slen = GridFunctions.getObjnameByIdx(i, &node_name[0], 1023);

                node_name.resize(slen + 1);

                std::cout << "GridFunctions sub-node name " << node_name << std::endl;
            }
*/

            std::string first_subnode(1024, '\0');

            ssize_t slen = GridFunctions.getObjnameByIdx(0, &first_subnode[0], 1023);
            first_subnode.resize(slen + 1);

            EXPECT_TRUE(first_subnode.compare(0, slen, "GridFunction_00002") == 0);
        }


        {
            H5::Group        GridFunction_00002 = file.openGroup("Timestep_00001/GridParts/GridPart_00000/GridFunctions/GridFunction_00002");
            H5::Attribute    attr = GridFunction_00002.openAttribute("limits_max");

            double limits_max = 0.0;
        
            H5::DataType type = attr.getDataType();
            attr.read(type, &limits_max);

//            std::cout << "limits_max " << limits_max << std::endl;
            EXPECT_NEAR(limits_max, 0.3970204292629652, 1e-10);
        }


        {
            H5::Group        GridFunction_00002 = file.openGroup("Timestep_00001/GridParts/GridPart_00000/GridFunctions/GridFunction_00002");
            H5::DataSet        dataset = H5::DataSet(GridFunction_00002.openDataSet("values"));

            hsize_t dims[2];
            H5::DataSpace    dataspace = dataset.getSpace();
            dataspace.getSimpleExtentDims(dims, nullptr);

            std::vector<double> values;
            values.resize(dims[0]);
            dataset.read(values.data(), H5::PredType::NATIVE_DOUBLE);

/*            for (hsize_t i = 0; i < dims[0]; i++)
            {
                std::cout << "value " << i << " " << values[i] << std::endl;

            }
*/
            EXPECT_NEAR(values[0], 0.32356910366452146, 1e-10);
            EXPECT_NEAR(values[dims[0] - 1], 0.12200070891582514, 1e-10);
        }



    }  // end of try block
    
       
    catch (H5::FileIException error)        // catch failure caused by the H5File operations
    {
        std::cout << error.getCDetailMsg();
    }
    
    catch (H5::DataSetIException error)        // catch failure caused by the DataSet operations
    {
        std::cout << error.getCDetailMsg();
    }
    
    catch (H5::DataSpaceIException error)    // catch failure caused by the DataSpace operations
    {
        std::cout << error.getCDetailMsg(); 
    }
    
    catch (H5::DataTypeIException error)        // catch failure caused by the DataSpace operations
    {
        std::cout << error.getCDetailMsg();
    }

}

#endif //USE_HDF5