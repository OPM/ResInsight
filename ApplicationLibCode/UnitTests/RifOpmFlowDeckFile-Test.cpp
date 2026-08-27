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
#include "opm/input/eclipse/Parser/ParserKeywords/C.hpp"

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
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
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
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
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
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
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
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
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
    bool               reloadSuccess = reloadedDeckFile.loadDeck( savedFileName.toStdString() ).has_value();
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
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
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
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
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
    bool               reloadSuccess = reloadedDeckFile.loadDeck( savedFileName.toStdString() ).has_value();
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
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
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
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
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
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();

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

//--------------------------------------------------------------------------------------------------
/// Saving to a folder that cannot be created must fail gracefully instead of crashing.
/// Opm::FileDeck::dump() throws std::filesystem::filesystem_error when the output folder cannot
/// be created. Here a regular file is used as a parent path component to trigger the exception.
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, SaveDeckToInvalidFolder )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file";

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    QString blockerFilePath = tempDir.filePath( "blocker" );
    QFile   blockerFile( blockerFilePath );
    ASSERT_TRUE( blockerFile.open( QIODevice::WriteOnly ) );
    blockerFile.close();

    std::string invalidFolder = ( blockerFilePath + "/subfolder" ).toStdString();

    EXPECT_FALSE( deckFile.saveDeck( invalidFolder, "test.DATA" ) ) << "saveDeck should return false for an invalid output folder";
    EXPECT_FALSE( deckFile.saveDeckInline( invalidFolder, "test.DATA" ) )
        << "saveDeckInline should return false for an invalid output folder";
}

//--------------------------------------------------------------------------------------------------
/// Verify that a keyword is inserted as the first entry inside the requested section.
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, InsertKeywordAtSectionStart )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file";

    Opm::DeckKeyword bcpropKw( ( Opm::ParserKeywords::BCPROP() ) );
    bool             insertSuccess = deckFile.insertKeywordAtSectionStart( "SCHEDULE", bcpropKw );
    EXPECT_TRUE( insertSuccess ) << "Should successfully insert BCPROP at start of SCHEDULE section";

    auto keywords = deckFile.keywords( false );
    auto schedIt  = std::find( keywords.begin(), keywords.end(), "SCHEDULE" );
    ASSERT_NE( schedIt, keywords.end() ) << "SCHEDULE section keyword not found";

    auto nextIt = std::next( schedIt );
    ASSERT_NE( nextIt, keywords.end() ) << "Expected a keyword after SCHEDULE";
    EXPECT_EQ( "BCPROP", *nextIt ) << "BCPROP should be the first keyword inside SCHEDULE";
}

//--------------------------------------------------------------------------------------------------
/// Inserting into a section that does not exist should fail and leave the deck unchanged.
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, InsertKeywordAtSectionStartNonexistentSection )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file";

    auto keywordsBefore = deckFile.keywords( false );

    Opm::DeckKeyword bcpropKw( ( Opm::ParserKeywords::BCPROP() ) );
    bool             insertSuccess = deckFile.insertKeywordAtSectionStart( "NONEXISTENT", bcpropKw );
    EXPECT_FALSE( insertSuccess ) << "Should fail when target section does not exist";

    auto keywordsAfter = deckFile.keywords( false );
    EXPECT_EQ( keywordsBefore, keywordsAfter ) << "Deck should be unchanged when section is missing";
}

//--------------------------------------------------------------------------------------------------
/// Verify the documented use case: a pre-existing keyword can be moved to the start of a
/// section by combining removeKeywords + insertKeywordAtSectionStart.
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, RemoveAndInsertKeywordAtSectionStart )
{
    static const QString testDataFolder = QString( "%1/RifOpmFlowDeckFile/" ).arg( TEST_DATA_DIR );
    QString              fileName       = testDataFolder + "SIMPLE_NO_REGDIMS.DATA";

    RifOpmFlowDeckFile deckFile;
    bool               loadSuccess = deckFile.loadDeck( fileName.toStdString() ).has_value();
    ASSERT_TRUE( loadSuccess ) << "Failed to load test deck file";

    // Place BCPROP somewhere in the deck first (in GRID), so that the second insert
    // exercises the "remove existing then insert at section start" pattern.
    Opm::DeckKeyword bcpropKw( ( Opm::ParserKeywords::BCPROP() ) );
    ASSERT_TRUE( deckFile.replaceKeyword( "GRID", bcpropKw ) );

    auto keywordsAfterFirst = deckFile.keywords( false );
    EXPECT_EQ( 1, std::count( keywordsAfterFirst.begin(), keywordsAfterFirst.end(), std::string( "BCPROP" ) ) );

    // Remove the existing occurrence and re-insert at SCHEDULE start.
    int removed = deckFile.removeKeywords( bcpropKw.name() );
    EXPECT_EQ( 1, removed ) << "Should remove the existing BCPROP keyword";

    bool insertSuccess = deckFile.insertKeywordAtSectionStart( "SCHEDULE", bcpropKw );
    EXPECT_TRUE( insertSuccess );

    auto keywords = deckFile.keywords( false );
    EXPECT_EQ( 1, std::count( keywords.begin(), keywords.end(), std::string( "BCPROP" ) ) ) << "BCPROP should appear exactly once";

    auto schedIt = std::find( keywords.begin(), keywords.end(), "SCHEDULE" );
    ASSERT_NE( schedIt, keywords.end() );
    auto nextIt = std::next( schedIt );
    ASSERT_NE( nextIt, keywords.end() );
    EXPECT_EQ( "BCPROP", *nextIt ) << "BCPROP should be the first keyword inside SCHEDULE";
}

//--------------------------------------------------------------------------------------------------
/// Issue #14620: table keywords were rewritten as one unbounded line, exceeding the 132 character
/// line limit of Eclipse 100. Verify that saved decks wrap all lines at 132 characters and that
/// quoted values are kept intact.
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, SaveDeckLimitsLineWidthTo132Characters )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    // Build a deck with a rel. perm. table large enough to exceed 132 characters when written on a
    // single line, and a record of quoted mnemonics that must not be split inside the quotes.
    QString deckText;
    deckText += "RUNSPEC\n";
    deckText += "DIMENS\n 2 2 2 /\n";
    deckText += "OIL\nWATER\nGAS\nMETRIC\n";
    deckText += "START\n 01 'JAN' 2000 /\n";
    deckText += "GRID\n";
    deckText += "DX\n8*100.0 /\n";
    deckText += "DY\n8*100.0 /\n";
    deckText += "DZ\n8*10.0 /\n";
    deckText += "TOPS\n4*2000.0 /\n";
    deckText += "PORO\n8*0.2 /\n";
    deckText += "PERMX\n8*100.0 /\n";
    deckText += "PROPS\n";
    deckText += "SWOF\n";
    const int rowCount = 40;
    for ( int i = 0; i < rowCount; i++ )
    {
        double sw   = 0.2 + 0.8 * i / rowCount;
        double krw  = sw * sw * 0.987654321;
        double krow = ( 1.0 - sw ) * 0.876543219;
        deckText += QString( "%1 %2 %3 0.0\n" ).arg( sw, 0, 'g', 10 ).arg( krw, 0, 'g', 10 ).arg( krow, 0, 'g', 10 );
    }
    deckText += "/\n";
    deckText += "SOLUTION\n";
    deckText += "RPTRST\n";
    deckText += " 'BASIC=4' 'FREQ=6' 'FLOWS' 'KRO' 'KRW' 'KRG' 'SGTRAP' 'RK' 'CONV' 'PORV' 'RPORV' 'BG' 'BO' 'BW' 'SFIP' 'PBPD' 'PCOW' /\n";
    deckText += "SCHEDULE\n";
    deckText += "END\n";

    QString inputFileName = tempDir.filePath( "LONG_TABLES.DATA" );
    {
        QFile inputFile( inputFileName );
        ASSERT_TRUE( inputFile.open( QIODevice::WriteOnly | QIODevice::Text ) );
        QTextStream out( &inputFile );
        out << deckText;
    }

    RifOpmFlowDeckFile deckFile;
    ASSERT_TRUE( deckFile.loadDeck( inputFileName.toStdString() ).has_value() );

    QString outDir = tempDir.filePath( "out" );
    ASSERT_TRUE( QDir().mkpath( outDir ) );
    ASSERT_TRUE( deckFile.saveDeck( outDir.toStdString(), "LONG_TABLES.DATA" ) );

    QString savedFileName = outDir + "/LONG_TABLES.DATA";
    QFile   savedFile( savedFileName );
    ASSERT_TRUE( savedFile.open( QIODevice::ReadOnly | QIODevice::Text ) );

    QTextStream in( &savedFile );
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        EXPECT_LE( line.size(), 132 ) << "Line exceeds 132 characters: " << line.toStdString();
    }
    savedFile.close();

    // Quoted values must be preserved unbroken, and the wrapped deck must still be parseable
    ASSERT_TRUE( savedFile.open( QIODevice::ReadOnly | QIODevice::Text ) );
    QString content = QTextStream( &savedFile ).readAll();
    savedFile.close();
    EXPECT_TRUE( content.contains( "'BASIC=4'" ) );
    EXPECT_TRUE( content.contains( "'SGTRAP'" ) );

    RifOpmFlowDeckFile reloadedDeckFile;
    EXPECT_TRUE( reloadedDeckFile.loadDeck( savedFileName.toStdString() ).has_value() );
}

//--------------------------------------------------------------------------------------------------
/// Issue #14621: include files containing only INCLUDE statements (e.g. a wrapper collecting many
/// lift curve files) were dropped when saving the deck, and all their child includes were written
/// directly into the main .DATA file. Verify that the include hierarchy is preserved.
//--------------------------------------------------------------------------------------------------
TEST( RifOpmFlowDeckFileTest, SaveDeckPreservesIncludeOnlyWrapperFiles )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );
    ASSERT_TRUE( QDir().mkpath( tempDir.filePath( "include" ) ) );

    auto writeTextFile = []( const QString& fileName, const QString& text )
    {
        QFile file( fileName );
        ASSERT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
        QTextStream out( &file );
        out << text;
    };

    auto vfpprodText = []( int tableNumber )
    {
        QString text;
        text += "VFPPROD\n";
        text += QString( " %1 2000.0 'LIQ' 'WCT' 'GOR' 'THP' ' ' 'METRIC' 'BHP' /\n" ).arg( tableNumber );
        text += " 100.0 /\n";
        text += " 10.0 /\n";
        text += " 0.0 /\n";
        text += " 0.0 /\n";
        text += " 0.0 /\n";
        text += " 1 1 1 1 200.0 /\n";
        return text;
    };

    // Main deck includes a wrapper file containing only INCLUDE statements. The wrapper includes
    // one lift curve file directly, and a nested wrapper which includes a second lift curve file.
    QString deckText;
    deckText += "RUNSPEC\n";
    deckText += "DIMENS\n 2 2 2 /\n";
    deckText += "OIL\nWATER\nGAS\nMETRIC\n";
    deckText += "START\n 01 'JAN' 2000 /\n";
    deckText += "GRID\n";
    deckText += "DX\n8*100.0 /\n";
    deckText += "DY\n8*100.0 /\n";
    deckText += "DZ\n8*10.0 /\n";
    deckText += "TOPS\n4*2000.0 /\n";
    deckText += "PORO\n8*0.2 /\n";
    deckText += "PERMX\n8*100.0 /\n";
    deckText += "SCHEDULE\n";
    deckText += "INCLUDE\n 'include/lift_curves.inc' /\n";
    deckText += "END\n";
    writeTextFile( tempDir.filePath( "WRAPPER_INCLUDES.DATA" ), deckText );

    QString wrapperText;
    wrapperText += "INCLUDE\n 'include/vfp_curve_1.ecl' /\n";
    wrapperText += "INCLUDE\n 'include/more_curves.inc' /\n";
    writeTextFile( tempDir.filePath( "include/lift_curves.inc" ), wrapperText );

    writeTextFile( tempDir.filePath( "include/more_curves.inc" ), "INCLUDE\n 'include/vfp_curve_2.ecl' /\n" );
    writeTextFile( tempDir.filePath( "include/vfp_curve_1.ecl" ), vfpprodText( 1 ) );
    writeTextFile( tempDir.filePath( "include/vfp_curve_2.ecl" ), vfpprodText( 2 ) );

    RifOpmFlowDeckFile deckFile;
    ASSERT_TRUE( deckFile.loadDeck( tempDir.filePath( "WRAPPER_INCLUDES.DATA" ).toStdString() ).has_value() );

    QString outDir = tempDir.filePath( "out" );
    ASSERT_TRUE( QDir().mkpath( outDir ) );
    ASSERT_TRUE( deckFile.saveDeck( outDir.toStdString(), "WRAPPER_INCLUDES.DATA" ) );

    auto readTextFile = []( const QString& fileName )
    {
        QFile file( fileName );
        EXPECT_TRUE( file.open( QIODevice::ReadOnly | QIODevice::Text ) ) << "Missing file: " << fileName.toStdString();
        return QTextStream( &file ).readAll();
    };

    // The main .DATA file must reference the wrapper only, not the individual lift curve files
    QString mainText = readTextFile( outDir + "/WRAPPER_INCLUDES.DATA" );
    EXPECT_TRUE( mainText.contains( "include/lift_curves.inc" ) );
    EXPECT_FALSE( mainText.contains( "vfp_curve" ) );
    EXPECT_FALSE( mainText.contains( "more_curves" ) );

    // The wrapper files must be recreated with their child includes
    QString wrapperOutText = readTextFile( outDir + "/include/lift_curves.inc" );
    EXPECT_TRUE( wrapperOutText.contains( "include/vfp_curve_1.ecl" ) );
    EXPECT_TRUE( wrapperOutText.contains( "include/more_curves.inc" ) );

    QString nestedWrapperOutText = readTextFile( outDir + "/include/more_curves.inc" );
    EXPECT_TRUE( nestedWrapperOutText.contains( "include/vfp_curve_2.ecl" ) );

    // The exported model must still be parseable, with both lift curves present
    RifOpmFlowDeckFile reloadedDeckFile;
    ASSERT_TRUE( reloadedDeckFile.loadDeck( ( outDir + "/WRAPPER_INCLUDES.DATA" ).toStdString() ).has_value() );
    auto keywords = reloadedDeckFile.keywords( false );
    EXPECT_EQ( 2, std::count( keywords.begin(), keywords.end(), std::string( "VFPPROD" ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimKeywordFactoryTest, DeckKeywordToAlignedStringShortensLongHeaders )
{
    using C = Opm::ParserKeywords::COMPDAT;

    Opm::DeckKeyword kw( ( Opm::ParserKeywords::COMPDAT() ) );

    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( C::WELL::itemName, std::string( "WELL-1" ) ) );
    items.push_back( RifOpmDeckTools::item( C::I::itemName, 12 ) );
    items.push_back( RifOpmDeckTools::item( C::J::itemName, 34 ) );
    items.push_back( RifOpmDeckTools::item( C::K1::itemName, 5 ) );
    items.push_back( RifOpmDeckTools::item( C::K2::itemName, 7 ) );
    items.push_back( RifOpmDeckTools::item( C::STATE::itemName, std::string( "OPEN" ) ) );
    items.push_back( RifOpmDeckTools::defaultItem( C::SAT_TABLE::itemName ) );
    items.push_back( RifOpmDeckTools::item( C::CONNECTION_TRANSMISSIBILITY_FACTOR::itemName, 0.1234567891 ) );
    items.push_back( RifOpmDeckTools::item( C::DIAMETER::itemName, 0.216 ) );
    items.push_back( RifOpmDeckTools::defaultItem( C::Kh::itemName ) );
    items.push_back( RifOpmDeckTools::item( C::SKIN::itemName, 0.0 ) );
    items.push_back( RifOpmDeckTools::defaultItem( C::D_FACTOR::itemName ) );
    items.push_back( RifOpmDeckTools::item( C::DIR::itemName, std::string( "Z" ) ) );
    kw.addRecord( Opm::DeckRecord{ std::move( items ) } );

    QString text = RimKeywordFactory::deckKeywordToAlignedString( kw );

    // Long parser item names must be abbreviated so they do not widen the columns (issue #14136).
    EXPECT_FALSE( text.contains( "CONNECTION_TRANSMISSIBILITY_FACTOR" ) );

    for ( const QString& line : text.split( '\n' ) )
    {
        EXPECT_LE( line.size(), 132 ) << "Line exceeds 132 characters: " << line.toStdString();
    }
}
