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
