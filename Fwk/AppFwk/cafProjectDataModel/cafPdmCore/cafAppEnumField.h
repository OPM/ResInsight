
#pragma once

#include "cafAppEnum.h"
#include "cafAssert.h"

#include "cafPdmValueField.h"
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <vector>

namespace caf
{

template <typename DataType>
class AppEnumField : public PdmValueField
{
public:
    using FieldDataType = caf::AppEnum<DataType>;

    AppEnumField() { static_assert( std::is_enum_v<DataType> == true ); };

    QVariant toQVariant() const override
    {
        auto enumValue = static_cast<std::underlying_type_t<DataType>>( m_fieldValue );
        return enumValue;
    }

    void setFromQVariant( const QVariant& variant ) override
    {
        m_fieldValue = static_cast<DataType>( variant.toInt() );
    }

    bool isReadOnly() const override { return false; }

    void setValue( const DataType& fieldValue ) { m_fieldValue = fieldValue; }

    DataType value() const { return m_fieldValue; }

private:
    caf::AppEnum<DataType> m_fieldValue;
};

} // namespace caf
