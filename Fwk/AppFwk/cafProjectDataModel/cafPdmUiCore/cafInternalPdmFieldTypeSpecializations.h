#pragma once

#include "cafAppEnum.h"
#include "cafPdmFieldTraits.h"
#include "cafPdmPointer.h"

namespace caf
{
template <typename T>
class PdmDataValueField;

//==================================================================================================
/// Helper base class for types that delegate all operations to pdmToVariant/pdmFromVariant/pdmVariantEqual.
/// Inherit from this to avoid repeating the delegation boilerplate.
//==================================================================================================
template <typename T>
struct PdmUiFieldSpecializationForValueSpec : public PdmUiFieldSpecializationDefaults
{
    static QVariant convert( const T& value )
    {
        using caf::pdmToVariant;
        return pdmToVariant( value );
    }

    static void setFromVariant( const QVariant& variantValue, T& value )
    {
        using caf::pdmFromVariant;
        pdmFromVariant( variantValue, value );
    }

    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return caf::pdmVariantEqual<T>( variantValue, variantValue2 );
    }
};

//==================================================================================================
/// Primary template - delegates to pdmToVariant/pdmFromVariant/pdmVariantEqual.
/// Types with custom overloads of those functions will automatically get correct behavior.
//==================================================================================================
template <typename T>
class PdmUiFieldSpecialization : public PdmUiFieldSpecializationForValueSpec<T>
{
};

//==================================================================================================
/// Partial specialization for PdmField< std::list<T> >
//==================================================================================================

template <typename T>
class PdmUiFieldSpecialization<std::list<T>> : public PdmUiFieldSpecializationDefaults
{
public:
    static QVariant convert( const std::list<T>& value )
    {
        QList<QVariant> returnList;
        for ( const auto& item : value )
        {
            using caf::pdmToVariant;
            returnList.push_back( pdmToVariant( item ) );
        }
        return returnList;
    }

    static void setFromVariant( const QVariant& variantValue, std::list<T>& value )
    {
        if ( variantValue.canConvert<QList<QVariant>>() )
        {
            value.clear();
            const QList<QVariant> lst = variantValue.toList();
            for ( const auto& item : lst )
            {
                T element;
                using caf::pdmFromVariant;
                pdmFromVariant( item, element );
                value.push_back( element );
            }
        }
    }

    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return caf::pdmVariantEqual<T>( variantValue, variantValue2 );
    }
};

//==================================================================================================
/// Partial specialization for PdmField< std::vector<T> >
//==================================================================================================

template <typename T>
class PdmUiFieldSpecialization<std::vector<T>> : public PdmUiFieldSpecializationDefaults
{
public:
    static QVariant convert( const std::vector<T>& value )
    {
        using caf::pdmToVariant;
        return pdmToVariant( value );
    }

    static void setFromVariant( const QVariant& variantValue, std::vector<T>& value )
    {
        using caf::pdmFromVariant;
        pdmFromVariant( variantValue, value );
    }

    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return caf::pdmVariantEqual<T>( variantValue, variantValue2 );
    }
};

//==================================================================================================
/// Partial specialization for PdmField<  caf::AppEnum<T> >
//==================================================================================================
template <typename T>
class PdmUiFieldSpecialization<caf::AppEnum<T>> : public PdmUiFieldSpecializationDefaults
{
public:
    static QVariant convert( const caf::AppEnum<T>& value ) { return caf::pdmToVariant( value ); }

    static void setFromVariant( const QVariant& variantValue, caf::AppEnum<T>& value )
    {
        caf::pdmFromVariant( variantValue, value );
    }

    static QList<PdmOptionItemInfo> valueOptions( PdmFieldHandle* fieldHandle, const caf::AppEnum<T>& appEnum )
    {
        QList<PdmOptionItemInfo> optionList;

        // If a subset of the enum is defined, use that subset
        auto enumValues = caf::AppEnum<T>::enumSubset( fieldHandle );
        if ( enumValues.empty() )
        {
            // If no subset is defined, use all values
            for ( size_t i = 0; i < caf::AppEnum<T>::size(); ++i )
            {
                enumValues.push_back( caf::AppEnum<T>::fromIndex( i ) );
            }
        }
        for ( auto enumValue : enumValues )
        {
            optionList.push_back( PdmOptionItemInfo( caf::AppEnum<T>::uiText( enumValue ), static_cast<int>( enumValue ) ) );
        }

        return optionList;
    }
};

//==================================================================================================
/// Partial specialization for PdmField<std::pair<T, U>>>
//==================================================================================================
template <typename T, typename U>
class PdmUiFieldSpecialization<std::pair<T, U>> : public PdmUiFieldSpecializationDefaults
{
public:
    static QVariant convert( const std::pair<T, U>& value )
    {
        using caf::pdmToVariant;
        return pdmToVariant( value );
    }

    static void setFromVariant( const QVariant& variantValue, std::pair<T, U>& value )
    {
        using caf::pdmFromVariant;
        pdmFromVariant( variantValue, value );
    }
};

} // End namespace caf
