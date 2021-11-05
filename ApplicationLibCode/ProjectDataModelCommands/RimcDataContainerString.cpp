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

#include "RimcDataContainerString.h"

#include "cafPdmFieldScriptingCapability.h"

#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimcDataContainerString, "DataContainerString" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcDataContainerString::RimcDataContainerString()
{
    CAF_PDM_InitScriptableObject( "Data Container String" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_stringValues, "values", "String Values" );
}
