/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaTextStringTools.h"
#include "RiaStdStringTools.h"

#include <QRegularExpression>
#include <QString>
#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaTextStringTools::compare( const QString& expected, const QString& actual )
{
    // Suggestions for improvement
    // 1. report line number for first change
    // 2. report line numbers for all changes
    // 3. add support for compare with content of a text file on disk

    return expected.compare( actual ) == 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextStringTools::trimAndRemoveDoubleSpaces( const QString& s )
{
    int     length;
    QString trimmed = s.trimmed();

    do
    {
        length  = trimmed.size();
        trimmed = trimmed.replace( "  ", " " );
    } while ( trimmed.size() < length );

    return trimmed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextStringTools::commonRoot( const QStringList& stringList )
{
    QString root;
    if ( !stringList.isEmpty() )
    {
        root = stringList.front();
        for ( const auto& item : stringList )
        {
            if ( root.length() > item.length() )
            {
                root.truncate( item.length() );
            }

            for ( int i = 0; i < root.length(); ++i )
            {
                if ( root[i] != item[i] )
                {
                    root.truncate( i );
                    break;
                }
            }
        }
    }
    return root;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextStringTools::commonSuffix( const QStringList& stringList )
{
    QString suffix;
    if ( !stringList.isEmpty() )
    {
        suffix = stringList.back();
        for ( const auto& item : stringList )
        {
            if ( suffix.length() > item.length() )
            {
                suffix = suffix.right( item.length() );
            }

            for ( int i = 0; i < suffix.length(); i++ )
            {
                int suffixIndex = suffix.length() - i - 1;
                int itemIndex   = item.length() - i - 1;
                if ( suffix[suffixIndex] != item[itemIndex] )
                {
                    suffix = suffix.right( i );
                    break;
                }
            }
        }
    }
    return suffix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextStringTools::trimNonAlphaNumericCharacters( const QString& s )
{
    QString            trimmedString = s;
    QRegularExpression trimRe( "[^a-zA-Z0-9]+$" );
    trimmedString.replace( trimRe, "" );
    return trimmedString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaTextStringTools::splitSkipEmptyParts( const QString& text, const QString& sep /*= " " */ )
{
    bool skipEmptyParts = true;
    return splitString( text, sep, skipEmptyParts );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaTextStringTools::splitSkipEmptyParts( const QString& text, const QRegularExpression& regularExpression )
{
    bool skipEmptyParts = true;
    return splitString( text, regularExpression, skipEmptyParts );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaTextStringTools::splitString( const QString& text, const QString& sep, bool skipEmptyParts )
{
    return text.split( sep, skipEmptyParts ? Qt::SkipEmptyParts : Qt::KeepEmptyParts, Qt::CaseInsensitive );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaTextStringTools::splitString( const QString& text, const QRegularExpression& regularExpression, bool skipEmptyParts )
{
    return text.split( regularExpression, skipEmptyParts ? Qt::SkipEmptyParts : Qt::KeepEmptyParts );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaTextStringTools::replaceTemplateTextWithValues( const QString& templateText, const std::map<QString, QString>& valueMap )
{
    QString resolvedText = templateText;

    // Use a regular expression to find all occurrences of ${key} in the text and replace with the value

    for ( const auto& [key, value] : valueMap )
    {
        QString regexString = key;
        regexString.replace( "$", "\\$" );
        regexString += "\\b";

        QRegularExpression rx( regexString );

        resolvedText.replace( rx, value );
    }

    return resolvedText;
}

//--------------------------------------------------------------------------------------------------
/// Qt recommends pass-by-value instead of pass-by-const-ref for QStringView
/// https://doc.qt.io/qt-6/qstringview.html
//--------------------------------------------------------------------------------------------------
bool RiaTextStringTools::isTextEqual( QStringView text, QStringView compareText )
{
    return text.compare( compareText, Qt::CaseInsensitive ) == 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaTextStringTools::isNumber( const QString& text, const QString& decimalPoint )
{
    if ( text.isEmpty() || decimalPoint.isEmpty() )
    {
        return false;
    }

    auto stdString   = text.toStdString();
    auto decimalChar = decimalPoint.toLatin1()[0];

    return RiaStdStringTools::isNumber( stdString, decimalChar );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVector<double> RiaTextStringTools::parseDoubleValues( const QString& input )
{
    // Flexible regex to extract three double numbers with any surrounding text
    QRegularExpression              coordRegex( R"([-+]?\d+\.?\d*)" );
    QRegularExpressionMatchIterator it = coordRegex.globalMatch( input );

    QVector<double> coords;
    while ( it.hasNext() )
    {
        QRegularExpressionMatch match = it.next();
        coords.append( match.captured().toDouble() );
    }

    return coords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
#if QT_VERSION < QT_VERSION_CHECK( 6, 8, 0 )
std::strong_ordering operator<=>( const QString& lhs, const QString& rhs )
{
    return lhs.compare( rhs ) <=> 0;
}
#endif
