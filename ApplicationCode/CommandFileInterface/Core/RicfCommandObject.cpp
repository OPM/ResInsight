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

#include "cafPdmPythonGenerator.h"

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
    QString snake_case = caf::PdmPythonGenerator::camelToSnakeCase( keyword );

    QString helpString = QString( "Available through python/rips as the attribute '%1'" ).arg( snake_case );

    if ( !existingTooltip.isEmpty() ) return existingTooltip + "\n\n" + helpString;
    return helpString;
}
