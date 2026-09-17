/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimCloudPolygon.h"

CAF_PDM_SOURCE_INIT( RimCloudPolygon, "RimCloudPolygon" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCloudPolygon::RimCloudPolygon()
{
    CAF_PDM_InitObject( "Sumo Polygon" );

    CAF_PDM_InitFieldNoDefault( &m_caseId, "SumoCaseId", "Case Id" );
    m_caseId.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_ensembleName, "SumoEnsembleName", "Ensemble Name" );
    m_ensembleName.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_realization, "SumoRealization", -1, "Realization" );
    m_realization.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_polygonResult, "SumoPolygonResult", "Polygon Result" );
    m_polygonResult.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_sumoName, "SumoPolygonName", "Sumo Name" );
    m_sumoName.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_contactType, "SumoContactType", "Fluid Contact Type" );
    m_contactType.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCloudPolygon::setSumoIdentity( const QString& caseId,
                                       const QString& ensembleName,
                                       int            realization,
                                       const QString& polygonResult,
                                       const QString& sumoName,
                                       const QString& contactType )
{
    m_caseId        = caseId;
    m_ensembleName  = ensembleName;
    m_realization   = realization;
    m_polygonResult = polygonResult;
    m_sumoName      = sumoName;
    m_contactType   = contactType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCloudPolygon::caseId() const
{
    return m_caseId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCloudPolygon::ensembleName() const
{
    return m_ensembleName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCloudPolygon::realization() const
{
    return m_realization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCloudPolygon::polygonResult() const
{
    return m_polygonResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCloudPolygon::sumoName() const
{
    return m_sumoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCloudPolygon::contactType() const
{
    return m_contactType();
}
