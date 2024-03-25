
#pragma once

#include "cafAppEnum.h"
#include "cafPdmValueField.h"

namespace caf
{
template <typename T>
class PdmEnumField : public PdmValueField
{
public:
    // FieldDataType is differently than in PdmDataValueField, where the FieldDataType is defined as T.
    // Similar concept is use in PdmPtrField and others.
    using FieldDataType = caf::AppEnum<T>;

    PdmEnumField() { static_assert( std::is_enum_v<T> == true ); };

    QVariant toQVariant() const override
    {
        auto enumValue = static_cast<std::underlying_type_t<T>>( m_fieldValue.value() );
        return enumValue;
    }

    void setFromQVariant( const QVariant& variant ) override { m_fieldValue = static_cast<T>( variant.toInt() ); }

    bool isReadOnly() const override { return false; }

    void setValue( const T& fieldValue ) { m_fieldValue = fieldValue; }

    T value() const { return m_fieldValue; }

    // Required to be able to assign a enum value to a PdmEnumField
    PdmEnumField& operator=( T value )
    {
        m_fieldValue = value;
        return *this;
    }

private:
    caf::AppEnum<T> m_fieldValue;
};

} // namespace caf
