#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifOpmDeckTools.h"
#include "RifOpmFlowDeckFile.h"

#include "RigEclipseResultTools.h"

#include "ProjectDataModel/Jobs/RimKeywordFactory.h"

#include "cvfStructGrid.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/B.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, RegdimsExistingKeyword )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "NORNE_ATW2013.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() );
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file";

    // Test reading existing REGDIMS
    auto regdimsValues = deckFile.regdims();
    EXPECT_FALSE( regdimsValues.empty() ) << "REGDIMS should exist in NORNE file";
    EXPECT_EQ( 7, regdimsValues.size() ) << "REGDIMS should have 7 values";

    // Values from the NORNE file: 22 3 1* 20 (rest are defaults)
    EXPECT_EQ( 22, regdimsValues[0] ) << "NTFIP should be 22";
    EXPECT_EQ( 3, regdimsValues[1] ) << "NMFIPR should be 3";
    // Third value is 1* (default), but OPM should handle this
    EXPECT_EQ( 20, regdimsValues[3] ) << "NTFREG should be 20";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, RegdimsAddKeyword )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() );
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file without REGDIMS";

    // Verify REGDIMS doesn't exist initially
    auto initialRegdimsValues = deckFile.regdims();
    EXPECT_TRUE( initialRegdimsValues.empty() ) << "REGDIMS should not exist initially";

    // Add REGDIMS keyword with default values
    bool addSuccess = deckFile.ensureRegdimsKeyword();
    EXPECT_TRUE( addSuccess ) << "Should successfully add REGDIMS keyword";

    // Verify REGDIMS now exists
    auto regdimsValues = deckFile.regdims();
    EXPECT_FALSE( regdimsValues.empty() ) << "REGDIMS should exist after adding";
    EXPECT_EQ( 7, regdimsValues.size() ) << "REGDIMS should have 7 values";

    // Verify default values (6* 1 /) - items 1-6 are defaults, item 7 (MAX_OPERNUM) is 1
    EXPECT_EQ( 1, regdimsValues[6] ) << "MAX_OPERNUM should be 1 (the explicit value)";

    // Test that calling ensureRegdimsKeyword again doesn't fail
    bool addAgainSuccess = deckFile.ensureRegdimsKeyword();
    EXPECT_TRUE( addAgainSuccess ) << "Should return true when REGDIMS already exists";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, RegdimsSetValues )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() );
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file";

    // Add REGDIMS keyword first
    bool addSuccess = deckFile.ensureRegdimsKeyword();
    EXPECT_TRUE( addSuccess ) << "Should successfully add REGDIMS keyword";

    // Set custom REGDIMS values (NTFIP NMFIPR NRFREG NTFREG MAX_ETRACK NTCREG MAX_OPERNUM)
    bool setSuccess = deckFile.setRegdims( 10, 5, 3, 8, 0, 2, 4 );
    EXPECT_TRUE( setSuccess ) << "Should successfully set REGDIMS values";

    // Verify the values were set correctly
    auto regdimsValues = deckFile.regdims();
    EXPECT_FALSE( regdimsValues.empty() ) << "REGDIMS should exist";
    EXPECT_EQ( 7, regdimsValues.size() ) << "REGDIMS should have 7 values";

    EXPECT_EQ( 10, regdimsValues[0] ) << "NTFIP should be 10";
    EXPECT_EQ( 5, regdimsValues[1] ) << "NMFIPR should be 5";
    EXPECT_EQ( 3, regdimsValues[2] ) << "NRFREG should be 3";
    EXPECT_EQ( 8, regdimsValues[3] ) << "NTFREG should be 8";
    EXPECT_EQ( 0, regdimsValues[4] ) << "MAX_ETRACK should be 0";
    EXPECT_EQ( 2, regdimsValues[5] ) << "NTCREG should be 2";
    EXPECT_EQ( 4, regdimsValues[6] ) << "MAX_OPERNUM should be 4";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, RegdimsSaveAndReload )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() );
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file";

    // Add REGDIMS keyword
    bool addSuccess = deckFile.ensureRegdimsKeyword();
    EXPECT_TRUE( addSuccess ) << "Should successfully add REGDIMS keyword";

    // Save the deck to a temporary location
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    bool saveSuccess = deckFile.saveDeck( tempDir.path().toStdString(), "test_with_regdims.DATA" );
    EXPECT_TRUE( saveSuccess ) << "Should successfully save deck with REGDIMS";

    // Reload the saved deck
    QString            savedFileName = tempDir.path() + "/test_with_regdims.DATA";
    RifOpmFlowDeckFile reloadedDeckFile;
    bool               reloadSuccess = reloadedDeckFile.loadDeck( savedFileName.toStdString() );
    EXPECT_TRUE( reloadSuccess ) << "Should successfully reload saved deck";

    // Verify REGDIMS exists in reloaded deck
    auto reloadedRegdimsValues = reloadedDeckFile.regdims();
    EXPECT_FALSE( reloadedRegdimsValues.empty() ) << "REGDIMS should exist in reloaded deck";
    EXPECT_EQ( 7, reloadedRegdimsValues.size() ) << "REGDIMS should have 7 values in reloaded deck";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, AddIncludeKeyword )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() );
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file";

    // Add INCLUDE statement in REGIONS section for OPERNUM
    bool addSuccess = deckFile.addIncludeKeyword( "REGIONS", "OPERNUM", "./include/opernum.prop" );
    EXPECT_TRUE( addSuccess ) << "Should successfully add INCLUDE statement in REGIONS section";

    // Add INCLUDE statement in GRID section for PERMX
    bool addGridSuccess = deckFile.addIncludeKeyword( "GRID", "PERMX", "./include/permx.prop" );
    EXPECT_TRUE( addGridSuccess ) << "Should successfully add INCLUDE statement in GRID section";

    // Test adding to non-existent section
    bool addFailure = deckFile.addIncludeKeyword( "NONEXISTENT", "SOME_KEYWORD", "./include/test.prop" );
    EXPECT_FALSE( addFailure ) << "Should fail when adding to non-existent section";

    // Verify the keywords list contains our INCLUDE statements
    auto keywords            = deckFile.keywords();
    bool foundRegionsInclude = false;
    bool foundGridInclude    = false;

    for ( const auto& keyword : keywords )
    {
        if ( keyword == "INCLUDE" )
        {
            // We can't easily verify the file path from keywords() output,
            // but we can verify INCLUDE keywords were added
            if ( !foundRegionsInclude )
            {
                foundRegionsInclude = true;
            }
            else if ( !foundGridInclude )
            {
                foundGridInclude = true;
            }
        }
    }

    EXPECT_TRUE( foundRegionsInclude || foundGridInclude ) << "Should have added at least one INCLUDE keyword";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, AddIncludeSaveAndReload )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() );
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file";

    // Create a temporary directory and OPERNUM include file
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    // Create the regions subdirectory
    QDir().mkpath( tempDir.path() + "/regions" );

    // Create a temporary OPERNUM file
    QString opernumFilePath = tempDir.path() + "/regions/opernum.inc";
    QFile   opernumFile( opernumFilePath );
    ASSERT_TRUE( opernumFile.open( QIODevice::WriteOnly | QIODevice::Text ) );

    QTextStream out( &opernumFile );
    out << "OPERNUM\n";
    out << "1000*1 /\n"; // Simple OPERNUM data for 1000 cells, all region 1
    opernumFile.close();

    // Add INCLUDE statement with relative path
    bool addSuccess = deckFile.addIncludeKeyword( "REGIONS", "OPERNUM", "./regions/opernum.inc" );
    EXPECT_TRUE( addSuccess ) << "Should successfully add INCLUDE statement";

    bool saveSuccess = deckFile.saveDeck( tempDir.path().toStdString(), "test_with_include.DATA" );
    EXPECT_TRUE( saveSuccess ) << "Should successfully save deck with INCLUDE";

    // Read the saved file as text to verify INCLUDE statement was written
    QString savedFileName = tempDir.path() + "/test_with_include.DATA";
    QFile   savedFile( savedFileName );
    ASSERT_TRUE( savedFile.open( QIODevice::ReadOnly | QIODevice::Text ) );

    QString content = savedFile.readAll();
    EXPECT_TRUE( content.contains( "INCLUDE" ) ) << "Saved file should contain INCLUDE keyword";
    EXPECT_TRUE( content.contains( "./regions/opernum.inc" ) ) << "Saved file should contain the include file path";
    savedFile.close();

    // Reload the saved deck
    RifOpmFlowDeckFile reloadedDeckFile;
    bool               reloadSuccess = reloadedDeckFile.loadDeck( savedFileName.toStdString() );
    EXPECT_TRUE( reloadSuccess ) << "Should successfully reload saved deck with INCLUDE";

    // Verify that OPERNUM keyword appears in reloaded deck (it should be included from the file)
    auto reloadedKeywords = reloadedDeckFile.keywords();
    bool foundOpernum     = false;
    for ( const auto& keyword : reloadedKeywords )
    {
        if ( keyword == "OPERNUM" )
        {
            foundOpernum = true;
            break;
        }
    }
    EXPECT_TRUE( foundOpernum ) << "Reloaded deck should contain OPERNUM keyword from included file";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, AddOperaterKeyword )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() );
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file";

    // Add OPERATER statement to EDIT section: PORV 9 MULTX PORV 1.0e6 1* 1*
    bool addSuccess = deckFile.replaceKeyword( "EDIT", RimKeywordFactory::operaterKeyword( "PORV", 9, "MULTX", "PORV", 1.0e6f, std::nullopt ) );
    EXPECT_TRUE( addSuccess ) << "Should successfully add OPERATER statement in EDIT section";

    // Test adding to non-existent section
    bool addFailure = deckFile.replaceKeyword( "NONEXISTENT",
                                               RimKeywordFactory::operaterKeyword( "PORV", 1, "MULTX", "PORV", std::nullopt, std::nullopt ) );
    EXPECT_FALSE( addFailure ) << "Should fail when adding to non-existent section";

    // Verify the keywords list contains our OPERATER statement
    auto keywords      = deckFile.keywords();
    bool foundOperater = false;

    for ( const auto& keyword : keywords )
    {
        if ( keyword == "OPERATER" )
        {
            foundOperater = true;
            break;
        }
    }

    EXPECT_TRUE( foundOperater ) << "Should have added OPERATER keyword";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, AddOperaterSaveAndReload )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() );
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file";

    // Add OPERATER statement to EDIT section: PORV 9 MULTX PORV 1.0e6 1* 1*
    bool addSuccess = deckFile.replaceKeyword( "EDIT", RimKeywordFactory::operaterKeyword( "PORV", 9, "MULTX", "PORV", 1.0e6f, std::nullopt ) );
    EXPECT_TRUE( addSuccess ) << "Should successfully add OPERATER statement to EDIT section";

    // Save the deck to a temporary location
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    bool saveSuccess = deckFile.saveDeck( tempDir.path().toStdString(), "test_with_operater.DATA" );
    EXPECT_TRUE( saveSuccess ) << "Should successfully save deck with OPERATER";

    // Read the saved file as text to verify OPERATER statement was written correctly
    QString savedFileName = tempDir.path() + "/test_with_operater.DATA";
    QFile   savedFile( savedFileName );
    ASSERT_TRUE( savedFile.open( QIODevice::ReadOnly | QIODevice::Text ) );

    QString content = savedFile.readAll();

    EXPECT_TRUE( content.contains( "OPERATER" ) ) << "Saved file should contain OPERATER keyword";
    EXPECT_TRUE( content.contains( "PORV" ) ) << "Saved file should contain PORV";
    EXPECT_TRUE( content.contains( "MULTX" ) ) << "Saved file should contain MULTX equation";
    EXPECT_TRUE( content.contains( "1e+06" ) || content.contains( "1000000" ) ) << "Saved file should contain the alpha value 1.0e6";

    // Verify OPERATER is in the EDIT section (between EDIT and PROPS keywords)
    int editPos     = content.indexOf( "EDIT" );
    int propsPos    = content.indexOf( "PROPS" );
    int operaterPos = content.indexOf( "OPERATER" );

    EXPECT_NE( editPos, -1 ) << "File should contain EDIT section";
    EXPECT_NE( propsPos, -1 ) << "File should contain PROPS section";
    EXPECT_NE( operaterPos, -1 ) << "File should contain OPERATER keyword";

    EXPECT_GT( operaterPos, editPos ) << "OPERATER should be after EDIT keyword";
    EXPECT_LT( operaterPos, propsPos ) << "OPERATER should be before PROPS keyword (i.e., in EDIT section)";

    savedFile.close();

    // Note: OPERATER statements may be processed by OPM during loading and might not
    // be preserved as standalone keywords in the reloaded deck. This is expected behavior.
    // The test validates that our addOperaterKeyword functionality works correctly
    // by checking that the OPERATER statement is properly saved to the file.
}

//--------------------------------------------------------------------------------------------------
/// Test BCPROP keyword generation
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, BcpropKeyword )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() ) << "Failed to create temporary directory";

    // Load the deck file
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() );

    ASSERT_TRUE( loadSuccess ) << "Failed to load deck file";

    // Create boundary conditions with different indices
    std::vector<RigEclipseResultTools::BorderCellFace> boundaryConditions;
    boundaryConditions.push_back( { caf::VecIjk0( 5, 5, 2 ), cvf::StructGridInterface::POS_I, 1 } );
    boundaryConditions.push_back( { caf::VecIjk0( 5, 6, 2 ), cvf::StructGridInterface::POS_J, 1 } );
    boundaryConditions.push_back( { caf::VecIjk0( 6, 5, 2 ), cvf::StructGridInterface::NEG_I, 2 } );
    boundaryConditions.push_back( { caf::VecIjk0( 7, 5, 2 ), cvf::StructGridInterface::POS_K, 2 } );

    // Create boundary condition properties
    // BC 1: Free flow boundary with specified pressure
    // BC 2: Fixed pressure boundary with temperature
    std::vector<Opm::DeckRecord> bcProperties;

    using B = Opm::ParserKeywords::BCPROP;

    // Property for BC 1 (index will be added by addBcpropKeyword)
    {
        std::vector<Opm::DeckItem> items;
        items.push_back( RifOpmDeckTools::item( B::TYPE::itemName, std::string( "FREE" ) ) );
        items.push_back( RifOpmDeckTools::item( B::COMPONENT::itemName, std::string( "NONE" ) ) );
        items.push_back( RifOpmDeckTools::item( B::RATE::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::PRESSURE::itemName, 200.0 ) ); // 200 bar
        items.push_back( RifOpmDeckTools::item( B::TEMPERATURE::itemName, 80.0 ) ); // 80 C
        items.push_back( RifOpmDeckTools::item( B::MECHTYPE::itemName, std::string( "NONE" ) ) );
        items.push_back( RifOpmDeckTools::item( B::FIXEDX::itemName, 1 ) );
        items.push_back( RifOpmDeckTools::item( B::FIXEDY::itemName, 1 ) );
        items.push_back( RifOpmDeckTools::item( B::FIXEDZ::itemName, 1 ) );
        items.push_back( RifOpmDeckTools::item( B::STRESSXX::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::STRESSYY::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::STRESSZZ::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::DISPX::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::DISPY::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::DISPZ::itemName, 0.0 ) );
        bcProperties.push_back( Opm::DeckRecord{ std::move( items ) } );
    }

    // Property for BC 2
    {
        std::vector<Opm::DeckItem> items;
        items.push_back( RifOpmDeckTools::item( B::TYPE::itemName, std::string( "DIRICH" ) ) );
        items.push_back( RifOpmDeckTools::item( B::COMPONENT::itemName, std::string( "WATER" ) ) );
        items.push_back( RifOpmDeckTools::item( B::RATE::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::PRESSURE::itemName, 250.0 ) ); // 250 bar
        items.push_back( RifOpmDeckTools::item( B::TEMPERATURE::itemName, 90.0 ) ); // 90 C
        items.push_back( RifOpmDeckTools::item( B::MECHTYPE::itemName, std::string( "NONE" ) ) );
        items.push_back( RifOpmDeckTools::item( B::FIXEDX::itemName, 1 ) );
        items.push_back( RifOpmDeckTools::item( B::FIXEDY::itemName, 1 ) );
        items.push_back( RifOpmDeckTools::item( B::FIXEDZ::itemName, 1 ) );
        items.push_back( RifOpmDeckTools::item( B::STRESSXX::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::STRESSYY::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::STRESSZZ::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::DISPX::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::DISPY::itemName, 0.0 ) );
        items.push_back( RifOpmDeckTools::item( B::DISPZ::itemName, 0.0 ) );
        bcProperties.push_back( Opm::DeckRecord{ std::move( items ) } );
    }

    // Create BCPROP keyword using factory and replace in deck
    Opm::DeckKeyword bcpropKw    = RimKeywordFactory::bcpropKeyword( boundaryConditions, bcProperties );
    bool             bcpropAdded = deckFile.replaceKeyword( "GRID", bcpropKw );
    ASSERT_TRUE( bcpropAdded ) << "Failed to replace BCPROP keyword";

    // Save deck and verify format
    QString outputDeckPath = tempDir.filePath( "output_bcprop.DATA" );
    bool    deckSaved      = deckFile.saveDeck( tempDir.path().toStdString(), "output_bcprop.DATA" );
    ASSERT_TRUE( deckSaved ) << "Failed to save deck file";
    ASSERT_TRUE( QFile::exists( outputDeckPath ) ) << "Output deck file not created";

    // Read and verify BCPROP content
    QFile outputFile( outputDeckPath );
    ASSERT_TRUE( outputFile.open( QIODevice::ReadOnly | QIODevice::Text ) );
    QString content = QTextStream( &outputFile ).readAll();
    outputFile.close();

    // Verify BCPROP keyword is present
    EXPECT_TRUE( content.contains( "BCPROP" ) ) << "BCPROP keyword not found in output";

    // Verify boundary condition types are present
    EXPECT_TRUE( content.contains( "FREE" ) ) << "FREE boundary condition type not found";
    EXPECT_TRUE( content.contains( "DIRICH" ) ) << "DIRICH boundary condition type not found";

    // Verify pressure values
    EXPECT_TRUE( content.contains( "200" ) ) << "Pressure 200 bar not found";
    EXPECT_TRUE( content.contains( "250" ) ) << "Pressure 250 bar not found";

    // Verify temperature values
    EXPECT_TRUE( content.contains( "80" ) ) << "Temperature 80 C not found";
    EXPECT_TRUE( content.contains( "90" ) ) << "Temperature 90 C not found";

    // Verify component
    EXPECT_TRUE( content.contains( "WATER" ) ) << "Component WATER not found";
}
