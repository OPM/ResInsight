/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 - Equinor ASA
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

#include "RimResultNameAlias.h"

CAF_PDM_SOURCE_INIT( RimResultNameAlias, "ResultNameAlias" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimResultNameAlias::RimResultNameAlias()
{
    CAF_PDM_InitObject( "ResultNameAlias" );

    CAF_PDM_InitField( &m_resultName, "ResultName", QString(), "Result Name" );
    CAF_PDM_InitField( &m_aliasName, "AliasName", QString(), "Alias Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimResultNameAlias::~RimResultNameAlias()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimResultNameAlias::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_resultName );
    uiOrdering.add( &m_aliasName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimResultNameAlias::setResultNameAndAlias( const QString& resultName, const QString& aliasName )
{
    m_resultName = resultName;
    m_aliasName  = aliasName;
}
