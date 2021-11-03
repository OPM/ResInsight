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

#include "RimEnsembleStatisticsSurface.h"

#include "RigSurface.h"
#include "RigSurfaceStatisticsCalculator.h"
#include "RimEnsembleSurface.h"
#include "RimSurfaceCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimEnsembleStatisticsSurface, "EnsembleStatisticsSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleStatisticsSurface::RimEnsembleStatisticsSurface()
{
    CAF_PDM_InitScriptableObject( "Surface", ":/ReservoirSurface16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_statisticsType, "StatisticsType", "StatisticsType", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleStatisticsSurface::~RimEnsembleStatisticsSurface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsSurface::setStatisticsType( RigSurfaceStatisticsCalculator::StatisticsType statisticsType )
{
    m_statisticsType = statisticsType;

    setUserDescription( fullName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSurfaceStatisticsCalculator::StatisticsType RimEnsembleStatisticsSurface::statisticsType() const
{
    return m_statisticsType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleStatisticsSurface::fullName() const
{
    return caf::AppEnum<RigSurfaceStatisticsCalculator::StatisticsType>::uiText( m_statisticsType.v() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleStatisticsSurface::onLoadData()
{
    return updateSurfaceData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimEnsembleStatisticsSurface::createCopy()
{
    auto* newSurface = dynamic_cast<RimEnsembleStatisticsSurface*>(
        xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

    if ( !newSurface->onLoadData() )
    {
        delete newSurface;
        return nullptr;
    }

    return newSurface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleStatisticsSurface::updateSurfaceData()
{
    RimEnsembleSurface* ensembleSurface;
    firstAncestorOrThisOfType( ensembleSurface );

    if ( ensembleSurface )
    {
        const RigSurface* surface = ensembleSurface->statisticsSurface();

        if ( surface )
        {
            const auto indices = surface->triangleIndices();
            const auto verts   = extractStatisticalDepthForVertices( surface );

            m_surfaceData = new RigSurface;
            m_surfaceData->setTriangleData( indices, verts );

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimEnsembleStatisticsSurface::extractStatisticalDepthForVertices( const RigSurface* surface ) const

{
    CVF_ASSERT( surface );
    std::vector<cvf::Vec3d> verts = surface->vertices();

    const auto& meanValues = surface->propertyValues(
        caf::AppEnum<RigSurfaceStatisticsCalculator::StatisticsType>::text( m_statisticsType.v() ) );

    for ( size_t i = 0; i < verts.size(); i++ )
    {
        verts[i].z() = meanValues[i];
    }

    return verts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsSurface::clearCachedNativeData()
{
}
