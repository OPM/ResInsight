#ifdef USE_HDF5

#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifHdf5Exporter.h"
#include "RifHdf5SummaryReader.h"
#include "RifOpmHdf5Summary.h"
#include "RifReaderEclipseSummary.h"

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif
#include "H5Cpp.h"

#include <iostream>

#include "RifHdf5SummaryExporter.h"

#ifdef _MSC_VER
// Disable warning from external library to make sure treat warnings as error works
#pragma warning( disable : 4267 )
#endif
#include "opm/io/eclipse/ESmry.hpp"
#include <numeric>

static const QString H5_TEST_DATA_DIRECTORY_2 = QString( "%1/h5-file/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_HDFTests, WriteToHdf5SummaryExporter )
{
    QString file_path = H5_TEST_DATA_DIRECTORY_2 + "NORNE_ATW2013_RFTPLT_V2.SMSPEC";

    std::string exportFileName = "e:/project/scratch_export/hdf_complete.h5";

    int  threadCount                = 1;
    bool createEnhancedSummaryFiles = true;
    RifHdf5SummaryExporter::ensureHdf5FileIsCreatedMultithreaded( { file_path.toStdString() },
                                                                  { exportFileName },
                                                                  createEnhancedSummaryFiles,
                                                                  threadCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_HDFTests, WriteDataToH5 )
{
    QString file_path = H5_TEST_DATA_DIRECTORY_2 + "NORNE_ATW2013_RFTPLT_V2.SMPEC";

    try
    {
        RifReaderEclipseSummary summaryReader;
        summaryReader.open( file_path, nullptr );

        {
            std::string exportFileName = "e:/project/scratch_export/hdf-test.h5";

            H5::H5File file( exportFileName, H5F_ACC_TRUNC ); // Overwrite existing

            // General group
            {
                std::vector<int> values( 7 );

                int day    = 1;
                int month  = 2;
                int year   = 3;
                int hour   = 4;
                int minute = 5;
                int second = 6;

                values[0] = day;
                values[1] = month;
                values[2] = year;
                values[3] = hour;
                values[4] = minute;
                values[5] = second;
                values[6] = 0;

                auto generalGroup = file.createGroup( "/general" );

                // start_date
                {
                    hsize_t dimsf[1];
                    dimsf[0] = values.size();
                    H5::DataSpace dataspace( 1, dimsf );

                    H5::DataType datatype( H5::PredType::NATIVE_INT );
                    H5::DataSet  dataset = generalGroup.createDataSet( "start_date", datatype, dataspace );
                    dataset.write( values.data(), H5::PredType::NATIVE_INT );
                }
            }

            {
                auto summaryGroup  = file.createGroup( "/summary_vectors" );
                auto myVectorGroup = summaryGroup.createGroup( "BPR" );
                auto dataGroup     = myVectorGroup.createGroup( "66" );

                std::vector<float> a( 10 );
                std::iota( a.begin(), a.end(), 10 );

                // dataset dimensions
                hsize_t dimsf[1];
                dimsf[0] = a.size();
                H5::DataSpace dataspace( 1, dimsf );

                H5::DataType datatype( H5::PredType::NATIVE_FLOAT );
                H5::DataSet  dataset = dataGroup.createDataSet( "values", datatype, dataspace );

                dataset.write( a.data(), H5::PredType::NATIVE_FLOAT );
            }
        }
    }

    catch ( H5::FileIException& error ) // catch failure caused by the H5File operations
    {
        std::cout << error.getCDetailMsg();
    }

    catch ( H5::DataSetIException& error ) // catch failure caused by the DataSet operations
    {
        std::cout << error.getCDetailMsg();
    }

    catch ( H5::DataSpaceIException& error ) // catch failure caused by the DataSpace operations
    {
        std::cout << error.getCDetailMsg();
    }

    catch ( H5::DataTypeIException& error ) // catch failure caused by the DataSpace operations
    {
        std::cout << error.getCDetailMsg();
    }
}

#endif // USE_HDF5
