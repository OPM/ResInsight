#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifGridCalculation.h"
#include "RifGridCalculationExporter.h"
#include "RifGridCalculationImporter.h"

static const std::string GRID_CALCULATION_DIR = std::string( TEST_DATA_DIR ) + std::string( "/RifGridCalculationIO/" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifGridCalculationIO, importAndExport )
{
    std::string fileName = GRID_CALCULATION_DIR + "calculations.toml";

    RifGridCalculation calc1;
    calc1.description = "My first calculation";
    calc1.expression  = "answer = a + b";
    calc1.unit        = "meter";
    RifGridCalculationVariable variable1;
    variable1.name           = "a";
    variable1.resultVariable = "PRESSURE";
    variable1.resultType     = "DYNAMIC_NATIVE";
    RifGridCalculationVariable variable2;
    variable2.name           = "b";
    variable2.resultVariable = "PORO";
    variable2.resultType     = "STATIC_NATIVE";
    calc1.variables          = { variable1, variable2 };

    RifGridCalculation calc2;
    calc2.description = "My second calculation";
    calc2.expression  = "answer = x + y";
    calc2.unit        = "meter";
    RifGridCalculationVariable variable3;
    variable3.name           = "x";
    variable3.resultVariable = "PRESSURE";
    variable3.resultType     = "DYNAMIC_NATIVE";
    RifGridCalculationVariable variable4;
    variable4.name           = "y";
    variable4.resultVariable = "PORO";
    variable4.resultType     = "STATIC_NATIVE";
    calc2.variables          = { variable3, variable4 };

    std::vector<RifGridCalculation> calculations = { calc1, calc2 };

    std::stringstream stream;
    auto [isOk, errorMessage] = RifGridCalculationExporter::writeToStream( calculations, stream );
    EXPECT_TRUE( isOk );
    EXPECT_TRUE( errorMessage.empty() );

    auto [importedCalculations, importErrorMessage] = RifGridCalculationImporter::readFromStream( stream );
    ASSERT_EQ( calculations.size(), importedCalculations.size() );

    ASSERT_EQ( calculations[0].description, importedCalculations[0].description );
    ASSERT_EQ( calculations[0].expression, importedCalculations[0].expression );
    ASSERT_EQ( calculations[0].unit, importedCalculations[0].unit );
    ASSERT_EQ( calculations[0].variables.size(), importedCalculations[0].variables.size() );

    for ( size_t v = 0; v < calculations[0].variables.size(); v++ )
    {
        ASSERT_EQ( calculations[0].variables[v].name, importedCalculations[0].variables[v].name );
        ASSERT_EQ( calculations[0].variables[v].resultType, importedCalculations[0].variables[v].resultType );
        ASSERT_EQ( calculations[0].variables[v].resultVariable, importedCalculations[0].variables[v].resultVariable );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifGridCalculationIO, importEmptyStream )
{
    std::stringstream stream;
    auto [calculations, errorMessage] = RifGridCalculationImporter::readFromStream( stream );
    ASSERT_EQ( 0, calculations.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifGridCalculationIO, importNotToml )
{
    std::stringstream stream;
    stream << "this is not valid toml";

    auto [calculations, errorMessage] = RifGridCalculationImporter::readFromStream( stream );
    ASSERT_EQ( 0, calculations.size() );
    ASSERT_FALSE( errorMessage.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifGridCalculationIO, importWrongToml )
{
    std::stringstream stream;
    stream << "[library]\n"
           << "book = \"book name\"\n"
           << "authors = [\"Author Name\"]\n"
           << "isbn = \"1234567\"\n";

    auto [calculations, errorMessage] = RifGridCalculationImporter::readFromStream( stream );
    ASSERT_EQ( 0, calculations.size() );
    ASSERT_FALSE( errorMessage.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifGridCalculationIO, importMissingDescriptionToml )
{
    std::stringstream stream;
    stream << "[[grid-calculation]]\n"
           << "description = 'MY_ASNWER ( NORNE_ATW2013_RFTPLT_V2 : PRESSURE, NORNE_ATW2013_RFTPLT_V2 : PORO )'\n"
           << "unit = ''\n";

    auto [calculations, errorMessage] = RifGridCalculationImporter::readFromStream( stream );
    ASSERT_EQ( 0, calculations.size() );
    ASSERT_FALSE( errorMessage.empty() );
}
