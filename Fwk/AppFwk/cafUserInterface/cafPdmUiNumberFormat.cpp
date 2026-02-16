#include "cafPdmUiNumberFormat.h"

#include "cafAppEnum.h"

template <>
void caf::AppEnum<caf::NumberFormatType>::setUp()
{
    addItem( caf::NumberFormatType::AUTO, "AUTO", "Automatic" );
    addItem( caf::NumberFormatType::FIXED, "FIXED", "Fixed, decimal" );
    addItem( caf::NumberFormatType::SCIENTIFIC, "SCIENTIFIC", "Scientific notation" );
    setDefault( caf::NumberFormatType::FIXED );
};

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmUiNumberFormat::valueToText( double value, NumberFormatType numberFormat, int precision )
{
    switch ( numberFormat )
    {
        case NumberFormatType::FIXED:
            return QString::number( value, 'f', precision );
        case NumberFormatType::SCIENTIFIC:
            return QString::number( value, 'e', precision );
        default:
            return QString::number( value, 'g', precision );
    }
}

} // namespace caf
