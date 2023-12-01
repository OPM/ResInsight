#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifSummaryCalculation.h"
#include "RifSummaryCalculationExporter.h"
#include "RifSummaryCalculationImporter.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifSummaryCalculationIO, importAndExport )
{
    RifSummaryCalculation calc1;
    calc1.description = "My first calculation";
    calc1.expression  = "answer = a + b";
    calc1.unit        = "meter";
    RifSummaryCalculationVariable variable1;
    variable1.name           = "a";
    variable1.address = "PRESSURE";
    RifSummaryCalculationVariable variable2;
    variable2.name           = "b";
    variable2.address = "PORO";
    calc1.variables          = { variable1, variable2 };

    RifSummaryCalculation calc2;
    calc2.description = "My second calculation";
    calc2.expression  = "answer = x + y";
    calc2.unit        = "meter";
    RifSummaryCalculationVariable variable3;
    variable3.name           = "x";
    variable3.address = "PRESSURE";
    RifSummaryCalculationVariable variable4;
    variable4.name           = "y";
    variable4.address = "PORO";
    calc2.variables          = { variable3, variable4 };

    std::vector<RifSummaryCalculation> calculations = { calc1, calc2 };

    std::stringstream stream;
    auto [isOk, errorMessage] = RifSummaryCalculationExporter::writeToStream( calculations, stream );
    EXPECT_TRUE( isOk );
    EXPECT_TRUE( errorMessage.empty() );

    auto [importedCalculations, importErrorMessage] = RifSummaryCalculationImporter::readFromStream( stream );
    ASSERT_EQ( calculations.size(), importedCalculations.size() );

    ASSERT_EQ( calculations[0].description, importedCalculations[0].description );
    ASSERT_EQ( calculations[0].expression, importedCalculations[0].expression );
    ASSERT_EQ( calculations[0].unit, importedCalculations[0].unit );
    ASSERT_EQ( calculations[0].variables.size(), importedCalculations[0].variables.size() );

    for ( size_t v = 0; v < calculations[0].variables.size(); v++ )
    {
        ASSERT_EQ( calculations[0].variables[v].name, importedCalculations[0].variables[v].name );
        ASSERT_EQ( calculations[0].variables[v].address, importedCalculations[0].variables[v].address );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifSummaryCalculationIO, importEmptyStream )
{
    std::stringstream stream;
    auto [calculations, errorMessage] = RifSummaryCalculationImporter::readFromStream( stream );
    ASSERT_EQ( 0u, calculations.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifSummaryCalculationIO, importNotToml )
{
    std::stringstream stream;
    stream << "this is not valid toml";

    auto [calculations, errorMessage] = RifSummaryCalculationImporter::readFromStream( stream );
    ASSERT_EQ( 0u, calculations.size() );
    ASSERT_FALSE( errorMessage.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifSummaryCalculationIO, importWrongToml )
{
    std::stringstream stream;
    stream << "[library]\n"
           << "book = \"book name\"\n"
           << "authors = [\"Author Name\"]\n"
           << "isbn = \"1234567\"\n";

    auto [calculations, errorMessage] = RifSummaryCalculationImporter::readFromStream( stream );
    ASSERT_EQ( 0u, calculations.size() );
    ASSERT_FALSE( errorMessage.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifSummaryCalculationIO, importMissingDescriptionToml )
{
    std::stringstream stream;
    stream << "[[grid-calculation]]\n"
           << "description = 'MY_ASNWER ( NORNE_ATW2013_RFTPLT_V2 : PRESSURE, NORNE_ATW2013_RFTPLT_V2 : PORO )'\n"
           << "unit = ''\n";

    auto [calculations, errorMessage] = RifSummaryCalculationImporter::readFromStream( stream );
    ASSERT_EQ( 0u, calculations.size() );
    ASSERT_FALSE( errorMessage.empty() );
}
