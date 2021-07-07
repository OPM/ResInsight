/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimEnsembleSurface.h"

#include "RiaLogging.h"

#include "RigSurfaceResampler.h"
#include "RigSurfaceStatisticsCalculator.h"

#include "RimEnsembleStatisticsSurface.h"
#include "RimFileSurface.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimEnsembleSurface, "EnsembleSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleSurface::RimEnsembleSurface()
{
    CAF_PDM_InitScriptableObject( "Ensemble Surface", ":/ReservoirSurfaces16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fileSurfaces, "FileSurfaces", "", "", "", "" );
    m_fileSurfaces.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_statisticsSurfaces, "StatisticsSurfaces", "", "", "", "" );
    m_statisticsSurfaces.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::removeFileSurface( RimFileSurface* fileSurface )
{
    m_fileSurfaces.removeChildObject( fileSurface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::addFileSurface( RimFileSurface* fileSurface )
{
    m_fileSurfaces.push_back( fileSurface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFileSurface*> RimEnsembleSurface::fileSurfaces() const
{
    return m_fileSurfaces().childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurface*> RimEnsembleSurface::surfaces() const
{
    std::vector<RimSurface*> surfaces;
    for ( auto fs : m_fileSurfaces.childObjects() )
        surfaces.push_back( fs );

    for ( auto s : m_statisticsSurfaces.childObjects() )
        surfaces.push_back( s );

    return surfaces;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::loadDataAndUpdate()
{
    for ( auto& w : m_fileSurfaces )
    {
        if ( !w->onLoadData() )
        {
            RiaLogging::warning( QString( "Failed to load surface: %1" ).arg( w->surfaceFilePath() ) );
        }
    }

    if ( !m_fileSurfaces.empty() )
    {
        cvf::ref<RigSurface> firstSurface = m_fileSurfaces[0]->surfaceData();

        std::vector<cvf::ref<RigSurface>> surfaces;
        for ( auto& w : m_fileSurfaces )
            surfaces.push_back( RigSurfaceResampler::resampleSurface( firstSurface, w->surfaceData() ) );

        m_statisticsSurface = RigSurfaceStatisticsCalculator::computeStatistics( surfaces );
        if ( !m_statisticsSurface.isNull() )
        {
            m_statisticsSurfaces.clear();
            m_statisticsSurfaces.push_back( new RimEnsembleStatisticsSurface );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigSurface* RimEnsembleSurface::statisticsSurface() const
{
    return m_statisticsSurface.p();
}
