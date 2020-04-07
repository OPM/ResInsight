/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimcDataContainerTime.h"

#include "cafPdmFieldIOScriptability.h"

#include "cafPdmObjectScriptability.h"

CAF_PDM_SOURCE_INIT( RimcDataContainerTime, "DataContainerTime" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcDataContainerTime::RimcDataContainerTime()
{
    CAF_PDM_InitScriptableObject( "Data Container Time", "", "", "" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_timeValues, "values", "Time Values", "", "", "" );
}
