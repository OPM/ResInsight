#include "gtest/gtest.h"

#include "RiaRegressionTest.h"
#include "RiaTestDataDirectory.h"
#include "RiaTextFileCompare.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RiaTextFileCompareTest, BasicCompareWithDiff)
{
    RiaRegressionTest regTestConfig;
    regTestConfig.readSettingsFromApplicationStore();
    QString folderContainingDiff = regTestConfig.folderContainingDiffTool();

    QString baseFolder      = QString("%1/TextCompare/base").arg(TEST_DATA_DIR);
    QString referenceFolder = QString("%1/TextCompare/reference").arg(TEST_DATA_DIR);

    RiaTextFileCompare compare(folderContainingDiff);

    bool noDifference = compare.runComparison(baseFolder, referenceFolder);
    EXPECT_FALSE(noDifference);

    QString diffOutput = compare.diffOutput();
    EXPECT_FALSE(diffOutput.isEmpty());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RiaTextFileCompareTest, BasicCompareNoDiff)
{
    RiaRegressionTest regTestConfig;
    regTestConfig.readSettingsFromApplicationStore();
    QString folderContainingDiff = regTestConfig.folderContainingDiffTool();

    QString baseFolder      = QString("%1/TextCompare/base/folderB").arg(TEST_DATA_DIR);
    QString referenceFolder = QString("%1/TextCompare/reference/folderB").arg(TEST_DATA_DIR);

    RiaTextFileCompare compare(folderContainingDiff);

    bool noDifference = compare.runComparison(baseFolder, referenceFolder);
    EXPECT_TRUE(noDifference);

    QString diffOutput = compare.diffOutput();
    EXPECT_TRUE(diffOutput.isEmpty());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RiaTextFileCompareTest, BasicCompareError)
{
    RiaRegressionTest regTestConfig;
    regTestConfig.readSettingsFromApplicationStore();
    QString folderContainingDiff = regTestConfig.folderContainingDiffTool();

    QString baseFolder      = QString("%1/TextCompare/baseDoesNotExist").arg(TEST_DATA_DIR);
    QString referenceFolder = QString("%1/TextCompare/reference/folderB").arg(TEST_DATA_DIR);

    RiaTextFileCompare compare(folderContainingDiff);

    bool noDifference = compare.runComparison(baseFolder, referenceFolder);
    EXPECT_FALSE(noDifference);

    QString error = compare.errorMessage();
    EXPECT_FALSE(error.isEmpty());
}
