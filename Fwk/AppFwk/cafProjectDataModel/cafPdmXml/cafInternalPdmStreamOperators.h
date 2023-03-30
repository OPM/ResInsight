#pragma once

#include <QTextStream>

//==================================================================================================
/// QTextStream Stream operator overloading for bool`s
/// Prints bool`s as "True"/"False", and reads them too
//==================================================================================================

QTextStream& operator>>( QTextStream& str, bool& value );
QTextStream& operator<<( QTextStream& str, const bool& value );

//==================================================================================================
/// QTextStream Stream operator overloading for QDateTimes`s
///
//==================================================================================================
// class QDateTime;
QTextStream& operator>>( QTextStream& str, QDateTime& value );
QTextStream& operator<<( QTextStream& str, const QDateTime& value );

//==================================================================================================
/// QTextStream Stream operator overloading for QDates
///
//==================================================================================================
// class QDate;
QTextStream& operator>>( QTextStream& str, QDate& value );
QTextStream& operator<<( QTextStream& str, const QDate& value );

//==================================================================================================
/// QTextStream Stream operator overloading for QTimes
///
//==================================================================================================
// class QTime;
QTextStream& operator>>( QTextStream& str, QTime& value );
QTextStream& operator<<( QTextStream& str, const QTime& value );

//==================================================================================================
/// QTextStream Stream operator overloading for std::vector of things.
/// Makes automated IO of PdmField< std::vector< Whatever > possible as long as
/// the type will print as one single word
//==================================================================================================

template <typename T>
QTextStream& operator<<( QTextStream& str, const std::vector<T>& sobj )
{
    size_t i;
    for ( i = 0; i < sobj.size(); ++i )
    {
        str << sobj[i] << " ";
    }
    return str;
}

template <typename T>
QTextStream& operator>>( QTextStream& str, std::vector<T>& sobj )
{
    while ( str.status() == QTextStream::Ok )
    {
        T d;
        str >> d;
        if ( str.status() == QTextStream::Ok ) sobj.push_back( d );
    }
    return str;
}

//==================================================================================================
/// QTextStream Stream operator overloading for std::pair<bool, T>
//==================================================================================================

template <typename T, typename U>
QTextStream& operator<<( QTextStream& str, const std::pair<U, T>& sobj )
{
    str << sobj.first;
    str << " ";
    str << sobj.second;

    return str;
}

template <typename T, typename U>
QTextStream& operator>>( QTextStream& str, std::pair<U, T>& sobj )
{
    U first( U() );
    T second( T() );

    str >> first;
    str >> second;

    sobj = std::make_pair( first, second );

    return str;
}
