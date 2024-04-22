#include "gtest/gtest.h"

// #include "RiaTestDataDirectory.h"

#include "RifOpmCommonSummary.h"
#include "RifReaderEclipseSummary.h"
#include <chrono>

size_t iterationCount = 5;
size_t maxCount       = 500;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_RifSummaryDataTest, OpmCommonAllData )
{
    QString filename = "e:/models/reek_history_match_large/realization-1/iter-0/eclipse/model/R001_REEK-1.SMSPEC";

    for ( size_t iteration = 0; iteration < iterationCount; iteration++ )
    {
        RifOpmCommonEclipseSummary reader;

        {
            auto start = std::chrono::high_resolution_clock::now();

            reader.open( filename, true, nullptr );

            auto                          end  = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            std::cout << "OPM : Open " << std::setw( 9 ) << diff.count() << " s\n";
        }

        //     for ( auto adr : reader.allResultAddresses() )
        //     {
        //         std::cout << adr.uiText();
        //         std::cout << std::endl;
        //     }

        {
            auto start = std::chrono::high_resolution_clock::now();

            size_t totalValuesRead = 0;

            // do some work
            size_t i = 0;
            for ( auto adr : reader.allResultAddresses() )
            {
                auto [isOk, values] = reader.values( adr );
                totalValuesRead += values.size();
                i++;
                if ( i > maxCount ) break;
            }

            auto                          end  = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            std::cout << "OPM Read data " << std::setw( 9 ) << totalValuesRead << "totalValueCount" << diff.count() << " s\n";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_RifSummaryDataTest, LibEclAllData )
{
    QString filename = "e:/models/reek_history_match_large/realization-1/iter-0/eclipse/model/R001_REEK-1.SMSPEC";

    for ( size_t iteration = 0; iteration < iterationCount; iteration++ )
    {
        RifReaderEclipseSummary reader;
        {
            auto start = std::chrono::high_resolution_clock::now();

            reader.open( filename, nullptr );

            auto                          end  = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            std::cout << "resdata : Open " << std::setw( 9 ) << diff.count() << " s\n";
        }

        //     for ( auto adr : reader.allResultAddresses() )
        //     {
        //         std::cout << adr.uiText();
        //         std::cout << std::endl;
        //     }

        {
            auto start = std::chrono::high_resolution_clock::now();

            size_t totalValuesRead = 0;

            size_t i = 0;
            for ( auto adr : reader.allResultAddresses() )
            {
                auto [isOk, values] = reader.values( adr );
                totalValuesRead += values.size();

                i++;
                if ( i > maxCount ) break;
            }

            auto                          end  = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            std::cout << "resdata read data" << std::setw( 9 ) << totalValuesRead << "totalValueCount" << diff.count() << " s\n";
        }
    }
}
