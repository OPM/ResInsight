#pragma once

#include <QVariant>

#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace caf
{

//==================================================================================================
/// pdmToVariant / pdmFromVariant
///
/// ADL-based customization points for converting field values to/from QVariant.
/// The default implementations work for any Qt-metatype-registered type.
/// Add overloads in the caf namespace (or the type's own namespace) to support additional types.
//==================================================================================================

template <typename T>
QVariant pdmToVariant( const T& value )
{
    return QVariant::fromValue( value );
}

template <typename T>
void pdmFromVariant( const QVariant& v, T& out )
{
    out = v.value<T>();
}

//==================================================================================================
/// std::vector overloads — element-wise delegation
//==================================================================================================

template <typename T>
QVariant pdmToVariant( const std::vector<T>& value )
{
    QList<QVariant> list;
    list.reserve( static_cast<int>( value.size() ) );
    for ( const auto& element : value )
    {
        using caf::pdmToVariant;
        list.push_back( pdmToVariant( element ) );
    }
    return list;
}

template <typename T>
void pdmFromVariant( const QVariant& v, std::vector<T>& out )
{
    if ( v.canConvert<QList<QVariant>>() )
    {
        out.clear();
        const QList<QVariant> list = v.toList();
        for ( const auto& item : list )
        {
            T element;
            using caf::pdmFromVariant;
            pdmFromVariant( item, element );
            out.push_back( element );
        }
    }
}

//==================================================================================================
/// std::pair overloads
//==================================================================================================

template <typename T, typename U>
QVariant pdmToVariant( const std::pair<T, U>& value )
{
    using caf::pdmToVariant;
    QList<QVariant> list;
    list.push_back( pdmToVariant( value.first ) );
    list.push_back( pdmToVariant( value.second ) );
    return list;
}

template <typename T, typename U>
void pdmFromVariant( const QVariant& v, std::pair<T, U>& out )
{
    if ( v.canConvert<QList<QVariant>>() )
    {
        const QList<QVariant> list = v.toList();
        if ( list.size() == 2 )
        {
            using caf::pdmFromVariant;
            pdmFromVariant( list[0], out.first );
            pdmFromVariant( list[1], out.second );
        }
    }
}

//==================================================================================================
/// std::optional overloads
/// Convention: nullopt -> invalid QVariant(); has value -> pdmToVariant(*val)
//==================================================================================================

template <typename T>
QVariant pdmToVariant( const std::optional<T>& value )
{
    if ( !value.has_value() ) return QVariant();
    using caf::pdmToVariant;
    return pdmToVariant( *value );
}

template <typename T>
void pdmFromVariant( const QVariant& v, std::optional<T>& out )
{
    if ( !v.isValid() )
    {
        out.reset();
        return;
    }
    T underlying;
    using caf::pdmFromVariant;
    pdmFromVariant( v, underlying );
    out = underlying;
}

//==================================================================================================
/// PdmVariantEqualImpl<T>
///
/// Class template used for comparing two QVariants carrying a field value of type T.
/// Partial specializations handle containers and composites.
/// Full specializations (e.g. for float, double, FilePath, cvf types) provide type-specific logic.
/// Extend by specializing PdmVariantEqualImpl<YourType> in the header that defines YourType.
//==================================================================================================

template <typename T>
struct PdmVariantEqualImpl
{
    static bool equal( const QVariant& a, const QVariant& b ) { return a.value<T>() == b.value<T>(); }
};

template <typename T>
struct PdmVariantEqualImpl<std::vector<T>>
{
    static bool equal( const QVariant& a, const QVariant& b )
    {
        const QList<QVariant> la = a.toList();
        const QList<QVariant> lb = b.toList();
        if ( la.size() != lb.size() ) return false;
        for ( int i = 0; i < la.size(); ++i )
            if ( !PdmVariantEqualImpl<T>::equal( la[i], lb[i] ) ) return false;
        return true;
    }
};

template <typename T, typename U>
struct PdmVariantEqualImpl<std::pair<T, U>>
{
    static bool equal( const QVariant& a, const QVariant& b )
    {
        const QList<QVariant> la = a.toList();
        const QList<QVariant> lb = b.toList();
        if ( la.size() != 2 || lb.size() != 2 ) return false;
        return PdmVariantEqualImpl<T>::equal( la[0], lb[0] ) && PdmVariantEqualImpl<U>::equal( la[1], lb[1] );
    }
};

template <typename T>
struct PdmVariantEqualImpl<std::optional<T>>
{
    static bool equal( const QVariant& a, const QVariant& b )
    {
        // pdmToVariant(nullopt) always produces an invalid QVariant()
        const bool aEmpty = !a.isValid();
        const bool bEmpty = !b.isValid();
        if ( aEmpty && bEmpty ) return true;
        if ( aEmpty != bEmpty ) return false;
        return PdmVariantEqualImpl<T>::equal( a, b );
    }
};

template <>
struct PdmVariantEqualImpl<float>
{
    static bool equal( const QVariant& a, const QVariant& b )
    {
        // See PdmFieldWriter::writeFieldData for the precision used when writing float values
        const float epsilon = 1e-6f;
        return qAbs( a.value<float>() - b.value<float>() ) < epsilon;
    }
};

template <>
struct PdmVariantEqualImpl<double>
{
    static bool equal( const QVariant& a, const QVariant& b )
    {
        // See PdmFieldWriter::writeFieldData for the precision used when writing double values
        const double epsilon = 1e-8;
        return qAbs( a.value<double>() - b.value<double>() ) < epsilon;
    }
};

//==================================================================================================
/// pdmVariantEqual<T> — public API, delegates to PdmVariantEqualImpl<T>
//==================================================================================================

template <typename T>
bool pdmVariantEqual( const QVariant& a, const QVariant& b )
{
    return PdmVariantEqualImpl<T>::equal( a, b );
}

} // namespace caf
