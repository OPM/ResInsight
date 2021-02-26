#include "gtest/gtest.h"

#include "RimWellPathCompletions.h"

#include <QRegExpValidator>
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimWellPathCompletions, WellNameRegExp )
{
    std::vector<QString> validNames   = { "RASASD", "gf0sdf", "sd-ASD12", "1-AA_b" };
    std::vector<QString> invalidNames = { ".AdSD", "+gf0sdf", "sd ASD12", "ABCDEFGHIJKL" };

    QRegExp rx = RimWellPathCompletionSettings::wellNameForExportRegExp();
    EXPECT_TRUE( rx.isValid() );

    for ( QString validName : validNames )
    {
        EXPECT_TRUE( rx.exactMatch( validName ) );
    }
    for ( QString invalidName : invalidNames )
    {
        EXPECT_FALSE( rx.exactMatch( invalidName ) );
    }
}

TEST( RimWellPathCompletions, WellNameRegExpValidator )
{
    std::vector<QString> validNames   = { "RASASD", "gf0sdf", "sd-ASD12", "1-AA_b" };
    std::vector<QString> invalidNames = { ".AdSD", "+gf0sdf", "sd ASD12", "ABCDEFGHIJKL" };
    QString              emptyString  = "";

    QRegExp          rx = RimWellPathCompletionSettings::wellNameForExportRegExp();
    QRegExpValidator validator( nullptr );
    validator.setRegExp( rx );

    for ( QString validName : validNames )
    {
        int dummyPos;
        EXPECT_EQ( QValidator::Acceptable, validator.validate( validName, dummyPos ) );
    }
    for ( QString invalidName : invalidNames )
    {
        int dummyPos;
        EXPECT_EQ( QValidator::Invalid, validator.validate( invalidName, dummyPos ) );
    }

    int dummyPos;
    EXPECT_EQ( QValidator::Intermediate, validator.validate( emptyString, dummyPos ) );
}
