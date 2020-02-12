/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfCommandObject.h"

#include <QRegularExpression>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandObject::RicfCommandObject()
    : RicfObjectCapability( this, false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandObject::~RicfCommandObject()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicfCommandObject::pythonHelpString( const QString& existingTooltip, const QString& keyword )
{
    static QRegularExpression re1( "(.)([A-Z][a-z]+)" );
    static QRegularExpression re2( "([a-z0-9])([A-Z])" );

    QString snake_case = keyword;
    snake_case.replace( re1, "\\1_\\2" );
    snake_case.replace( re2, "\\1_\\2" );

    QString helpString = QString( "Available through python/rips with the following object methods:\n"
                                  "  Get: value = %1()\n"
                                  "  Set: set_%1(value)\n" )
                             .arg( snake_case.toLower() );

    if ( !existingTooltip.isEmpty() ) return existingTooltip + "\n\n" + helpString;
    return helpString;
}
