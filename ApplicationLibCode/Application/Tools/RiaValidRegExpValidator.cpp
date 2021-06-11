/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RiaValidRegExpValidator.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaValidRegExpValidator::RiaValidRegExpValidator( const QString& defaultPattern )
    : QValidator( nullptr )
    , m_defaultPattern( defaultPattern )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaValidRegExpValidator::isValidCharacter( const QChar& character )
{
    return character.isLetterOrNumber() || character == '-' || character == '_' || character == '.' || character == '[';
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QValidator::State RiaValidRegExpValidator::validate( QString& inputString, int& position ) const
{
    QRegExp inputRe( inputString, Qt::CaseInsensitive, QRegExp::Wildcard );
    if ( inputRe.isValid() ) // A valid wildcard pattern is always acceptable
    {
        return QValidator::Acceptable;
    }

    if ( position >= inputString.length() )
    {
        // This should probably never happen, but the Qt-documentation isn't clear on it.
        CAF_ASSERT( false );
        return QValidator::Invalid;
    }

    // Try to decide whether it can be fixed by typing further characters or not.
    if ( !isValidCharacter( inputString[position] ) )
    {
        // Contains a invalid character: the whole regexp is invalid.
        return QValidator::Invalid;
    }

    return QValidator::Intermediate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaValidRegExpValidator::fixup( QString& inputString ) const
{
    inputString = m_defaultPattern;
}