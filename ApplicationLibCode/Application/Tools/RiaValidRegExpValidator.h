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
#pragma once

#include <QRegExp>
#include <QString>
#include <QValidator>

//--------------------------------------------------------------------------------------------------
// Validates a wild card pattern (simpler for user)
// Has to use the older QRegExp because QRegularExpression didn't add any
// support for wild cards until Qt 5.12.
//--------------------------------------------------------------------------------------------------
class RiaValidRegExpValidator : public QValidator
{
public:
    RiaValidRegExpValidator( const QString& defaultPattern );
    static bool isValidCharacter( const QChar& character );
    State       validate( QString& inputString, int& position ) const override;
    void        fixup( QString& inputString ) const override;

private:
    QString m_defaultPattern;
};