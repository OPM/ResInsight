
#include "gtest/gtest.h"

#include "cafPdmUiNumberFormat.h"

using namespace caf;

TEST( PdmUiNumberFormatTest, ValueToText )
{
    struct TestCase
    {
        double           value;
        std::string      expected;
        NumberFormatType format;
        int              precision;
    };

    // clang-format off
    std::vector<TestCase> testCases = {
        // Fixed format
        { 3.14159,      "3.14",         NumberFormatType::FIXED,      2 },
        { 0.0,          "0.000",        NumberFormatType::FIXED,      3 },
        { -1.5,         "-1.5",         NumberFormatType::FIXED,      1 },
        { 1.0 / 3.0,   "0.33333333",    NumberFormatType::FIXED,      8 },
        { 3.7,          "4",            NumberFormatType::FIXED,      0 },
        { -2.3,         "-2",           NumberFormatType::FIXED,      0 },
        { 1234567.89,   "1234567.89",   NumberFormatType::FIXED,      2 },

        // Scientific format
        { 1000.0,       "1.00e+03",     NumberFormatType::SCIENTIFIC, 2 },
        { 0.001,        "1.0e-03",      NumberFormatType::SCIENTIFIC, 1 },
        { -5.67e8,      "-5.670e+08",   NumberFormatType::SCIENTIFIC, 3 },
        { 1.23e-10,     "1.23e-10",     NumberFormatType::SCIENTIFIC, 2 },

        // Auto format
        { 42.0,         "42",           NumberFormatType::AUTO,       6 },
        { 0.000123,     "0.000123",     NumberFormatType::AUTO,       3 },
        { 0.0000001,    "1e-07",        NumberFormatType::AUTO,       3 },
        { 1.23456e8,    "1.235e+08",    NumberFormatType::AUTO,       4 },
        { 3.14159265,   "3.1",          NumberFormatType::AUTO,       2 },
        { 3.14159265,   "3.14159",      NumberFormatType::AUTO,       6 },
    };
    // clang-format on

    for ( const auto& tc : testCases )
    {
        QString result = PdmUiNumberFormat::valueToText( tc.value, tc.format, tc.precision );
        EXPECT_EQ( result.toStdString(), tc.expected )
            << "Failed for value=" << tc.value << " format=" << static_cast<int>( tc.format )
            << " precision=" << tc.precision;
    }
}
