
#include "RiaTestDataDirectory.h"
#include "gtest/gtest.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/io/eclipse/ERst.hpp"
#include "opm/io/eclipse/RestartFileView.hpp"
#include "opm/io/eclipse/rst/state.hpp"
#include "opm/io/eclipse/rst/well.hpp"

#include <QDir>
#include <QString>

const std::string drogonPath = "e:/gitroot/resinsight-tutorials/model-data/drogon/DROGON-0.UNRST";

TEST( DISABLED_opm_well_data_test, TestImport )
{
    Opm::Deck deck;

    // It is required to create a deck as the input parameter to runspec. The = default() initialization of the runspec keyword does not
    // initialize the object as expected.
    Opm::Runspec runspec( deck );
    Opm::Parser  parser( false );

    try
    {
        QDir baseFolder( TEST_MODEL_DIR );
        bool subFolderExists = baseFolder.cd( "TEST10K_FLT_LGR_NNC" );
        EXPECT_TRUE( subFolderExists );
        QString filename( "TEST10K_FLT_LGR_NNC.UNRST" );
        QString filePath = baseFolder.absoluteFilePath( filename );

        auto stdFilename = baseFolder.absoluteFilePath( filename ).toStdString();

        auto rstFile = std::make_shared<Opm::EclIO::ERst>( drogonPath );
        // rstFile->loadData();
        auto reportStepCount = rstFile->numberOfReportSteps();

        for ( auto seqNumber : rstFile->listOfReportStepNumbers() )
        {
            auto fileView = std::make_shared<Opm::EclIO::RestartFileView>( rstFile, seqNumber );

            auto state = Opm::RestartIO::RstState::load( fileView, runspec, parser );

            for ( const auto& w : state.wells )
            {
                auto name = w.name;
                std::cout << name << std::endl;
            }
        }
    }
    catch ( std::exception& e )
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}
