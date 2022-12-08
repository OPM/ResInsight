#include "gtest/gtest.h"

#include "Tools/RiaVariableMapper.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaVariableMapperTest, BasicUsage )
{
    QString inputText = R"(
        $MyVar$ /path/to/file;
        $PathId_001$ myFile.txt;
    )";

    RiaVariableMapper mapper( inputText );

    bool    isFound = false;
    QString value;

    value = mapper.valueForVariable( "$MyVar$", &isFound );
    EXPECT_TRUE( isFound );
    EXPECT_STREQ( value.toStdString().data(), "/path/to/file" );

    value = mapper.valueForVariable( "$PathId_001$", &isFound );
    EXPECT_TRUE( isFound );
    EXPECT_STREQ( value.toStdString().data(), "myFile.txt" );

    mapper.valueForVariable( "not present", &isFound );
    EXPECT_FALSE( isFound );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaVariableMapperTest, UserDefinedVariables )
{
    QString inputText = R"(
        $MyVar$ /path/to/file;
        $TestVar$ a different variable;
    )";

    RiaVariableMapper mapper( inputText );

    mapper.addPathAndGetId( "/path/to/file/myFile.txt" );
    mapper.addPathAndGetId( "/path/to/file/myFile2.txt" );

    mapper.replaceVariablesInValues();

    QString table = mapper.variableTableAsText();

    QString expectedText = R"(
        $MyVar$ /path/to/file;
        $TestVar$ a different variable;
        $PathId_001$ $MyVar$/myFile.txt;
        $PathId_002$ $MyVar$/myFile2.txt;
    )";

    EXPECT_TRUE( table == expectedText );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaVariableMapperTest, UserDefinedVariablesRecursive )
{
    QString inputText = R"(
        $MyVar2$ $MyVar1$/to/file;
        $MyVar1$ /path;
        $TestVar$ a different variable;
    )";

    RiaVariableMapper mapper( inputText );

    mapper.addPathAndGetId( "/path/to/file/myFile.txt" );
    mapper.addPathAndGetId( "/path/to/file/myFile2.txt" );

    mapper.replaceVariablesInValues();

    QString table = mapper.variableTableAsText();

    QString expectedText = R"(
        $MyVar1$ /path;
        $MyVar2$ $MyVar1$/to/file;
        $TestVar$ a different variable;
        $PathId_001$ $MyVar2$/myFile.txt;
        $PathId_002$ $MyVar2$/myFile2.txt;
    )";

    EXPECT_TRUE( table == expectedText );

    RiaVariableMapper otherMapper( table );
    bool              isFound = false;

    QString value = otherMapper.valueForVariable( "$MyVar1$", &isFound );
    EXPECT_TRUE( value == "/path" );

    value = otherMapper.valueForVariable( "$MyVar2$", &isFound );
    EXPECT_TRUE( value == "/path/to/file" );

    value = otherMapper.valueForVariable( "$PathId_001$", &isFound );
    EXPECT_TRUE( value == "/path/to/file/myFile.txt" );
}
