/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimCompletionData.h"

#include "RigCompletionData.h"

#include "RimCompdatData.h"
#include "RimWelspecsData.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimCompletionData, "CompletionData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCompletionData::RimCompletionData()
{
    CAF_PDM_InitScriptableObject( "CompletionData", ":/Well.png" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_wellName, "WellName", "Name of well" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_welspecs, "Welspecs", "Welspecs Data" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_compdat, "Compdat", "Compdat Data" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCompletionData::~RimCompletionData()
{
}

void RimCompletionData::addCompletionData( RigCompletionData* completionData )
{
    if ( !completionData )
    {
        return;
    }

    RimCompdatData* compdatData = new RimCompdatData();
    compdatData->m_wellname     = completionData->wellName();
    compdatData->m_I            = (int)completionData->completionDataGridCell().localCellIndexI();
    compdatData->m_J            = (int)completionData->completionDataGridCell().localCellIndexJ();
    compdatData->m_upperK       = (int)completionData->completionDataGridCell().localCellIndexK();
    compdatData->m_lowerK       = (int)completionData->completionDataGridCell().localCellIndexK();
    compdatData->m_openShutFlag = "OPEN";

    if ( !RigCompletionData::isDefaultValue( completionData->transmissibility() ) )
    {
        compdatData->m_transmissibility = completionData->transmissibility();
    }
    if ( !RigCompletionData::isDefaultValue( completionData->diameter() ) )
    {
        compdatData->m_diameter = completionData->diameter();
    }
    if ( !RigCompletionData::isDefaultValue( completionData->kh() ) )
    {
        compdatData->m_kh = completionData->kh();
    }
    if ( !RigCompletionData::isDefaultValue( completionData->skinFactor() ) )
    {
        compdatData->m_skinFactor = completionData->skinFactor();
    }
    if ( !RigCompletionData::isDefaultValue( completionData->dFactor() ) )
    {
        compdatData->m_dFactor = completionData->dFactor();
    }

    compdatData->m_direction = completionData->directionString();
    compdatData->m_startMD   = completionData->startMD();
    compdatData->m_endMD     = completionData->endMD();
    compdatData->m_comment   = completionData->metaDataString();

    m_compdat.push_back( compdatData );
}