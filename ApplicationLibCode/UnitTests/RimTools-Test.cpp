#include "gtest/gtest.h"

#include "RimTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimToolsTest, ReplacePathPattern )
{
    QString previousProjectPath = "C:/path/to/projectFileFolder";

    QString realization = "realization-*/MY_CASE-*";

    QString originalPattern = "C:/path/to/" + realization;
    QString newProjectPath  = "/mnt/new/path/to/projectFileFolder";
    QString expectedPattern = "/mnt/new/path/to/" + realization;

    QString resultPattern = RimTools::relocatePathPattern( originalPattern, newProjectPath, previousProjectPath );
    EXPECT_EQ( resultPattern, expectedPattern );
}
