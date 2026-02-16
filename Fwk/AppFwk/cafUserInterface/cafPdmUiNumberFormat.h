#pragma once

#include <QString>

namespace caf
{
enum class NumberFormatType
{
    AUTO,
    SCIENTIFIC,
    FIXED
};

class PdmUiNumberFormat
{
public:
    static QString valueToText( double value, NumberFormatType numberFormat, int precision );
};
} // namespace caf
