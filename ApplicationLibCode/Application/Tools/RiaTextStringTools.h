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

#pragma once

#include <QString>
#include <QStringList>

#include <map>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace RiaTextStringTools
{
bool    compare( const QString& expected, const QString& actual );
QString trimAndRemoveDoubleSpaces( const QString& s );
QString commonRoot( const QStringList& stringList );
QString commonSuffix( const QStringList& stringList );
QString trimNonAlphaNumericCharacters( const QString& s );

QStringList splitSkipEmptyParts( const QString& text, const QString& sep = " " );
QStringList splitSkipEmptyParts( const QString& text, const QRegExp& regExp );

QString replaceTemplateTextWithValues( const QString& templateText, const std::map<QString, QString>& valueMap );

} // namespace RiaTextStringTools

//--------------------------------------------------------------------------------------------------
//
// Add operator<=> for QString to global scope
//
// Example of error message when this operator is not defined:
//
// 'auto RicWellPathFractureReportItem::operator <=>(const RicWellPathFractureReportItem &) const'
//    : function was implicitly deleted because 'RicWellPathFractureReportItem' data member
//      'RicWellPathFractureReportItem::m_wellPathNameForExport' of type 'QString' has no valid
//      'operator<=>'
//
//--------------------------------------------------------------------------------------------------
std::strong_ordering operator<=>( const QString& lhs, const QString& rhs );
