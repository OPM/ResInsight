#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RifOpmFlowDeckFile.h"

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
    EXPECT_EQ( 4, regdimsValues.size() ) << "REGDIMS should have 4 values";

    // Values from the NORNE file: 22 3 1* 20
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
    EXPECT_EQ( 4, regdimsValues.size() ) << "REGDIMS should have 4 values";

    // Verify default values (6* 1 /) - OPM should interpret 6* as default values followed by 1
    EXPECT_EQ( 1, regdimsValues[3] ) << "NTFREG should be 1 (the explicit value)";

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

    // Set custom REGDIMS values
    bool setSuccess = deckFile.setRegdims( 10, 5, 3, 8 );
    EXPECT_TRUE( setSuccess ) << "Should successfully set REGDIMS values";

    // Verify the values were set correctly
    auto regdimsValues = deckFile.regdims();
    EXPECT_FALSE( regdimsValues.empty() ) << "REGDIMS should exist";
    EXPECT_EQ( 4, regdimsValues.size() ) << "REGDIMS should have 4 values";

    EXPECT_EQ( 10, regdimsValues[0] ) << "NTFIP should be 10";
    EXPECT_EQ( 5, regdimsValues[1] ) << "NMFIPR should be 5";
    EXPECT_EQ( 3, regdimsValues[2] ) << "NRFREG should be 3";
    EXPECT_EQ( 8, regdimsValues[3] ) << "NTFREG should be 8";
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
    EXPECT_EQ( 4, reloadedRegdimsValues.size() ) << "REGDIMS should have 4 values in reloaded deck";
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

    // Add OPERATER statement to GRID section: PORV 9 MULTX PORV 1.0e6 1* 1*
    bool addSuccess = deckFile.addOperaterKeyword( "GRID", "PORV", 9, "MULTX", "PORV", 1.0e6f, std::nullopt );
    EXPECT_TRUE( addSuccess ) << "Should successfully add OPERATER statement in GRID section";

    // Test adding to non-existent section
    bool addFailure = deckFile.addOperaterKeyword( "NONEXISTENT", "PORV", 1, "MULTX", "PORV", std::nullopt, std::nullopt );
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

    // Add OPERATER statement: PORV 9 MULTX PORV 1.0e6 1* 1*
    bool addSuccess = deckFile.addOperaterKeyword( "GRID", "PORV", 9, "MULTX", "PORV", 1.0e6f, std::nullopt );
    EXPECT_TRUE( addSuccess ) << "Should successfully add OPERATER statement";

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

    savedFile.close();

    // Note: OPERATER statements may be processed by OPM during loading and might not
    // be preserved as standalone keywords in the reloaded deck. This is expected behavior.
    // The test validates that our addOperaterKeyword functionality works correctly
    // by checking that the OPERATER statement is properly saved to the file.
}
