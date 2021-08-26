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

// TODO: Use the alias concept prototyped below when the alias concept for class is ready
// CAF_PDM_SOURCE_INIT( RimEnsembleStatisticsSurface, "EnsembleStatisticsSurface", "Surface" );
// CAF_PDM_SOURCE_INIT( <class>       ,  <keyword>   , <alias>);
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
    RimEnsembleStatisticsSurface* newSurface = dynamic_cast<RimEnsembleStatisticsSurface*>(
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
bool RimEnsembleStatisticsSurface ::updateSurfaceData()
{
    RimEnsembleSurface* ensembleSurface;
    firstAncestorOrThisOfType( ensembleSurface );

    if ( ensembleSurface )
    {
        const RigSurface* surface = ensembleSurface->statisticsSurface();

        if ( surface )
        {
            const std::vector<unsigned int>& indices  = surface->triangleIndices();
            const std::vector<cvf::Vec3d>&   vertices = surface->vertices();

            const std::vector<float>& meanValues = surface->propertyValues(
                caf::AppEnum<RigSurfaceStatisticsCalculator::StatisticsType>::text( m_statisticsType.v() ) );

            std::vector<cvf::Vec3d> verts;
            for ( size_t i = 0; i < vertices.size(); i++ )
            {
                verts.push_back( cvf::Vec3d( vertices[i].x(), vertices[i].y(), meanValues[i] ) );
            }

            m_tringleIndices = indices;
            m_vertices       = verts;

            m_surfaceData = new RigSurface;

            m_surfaceData->setTriangleData( m_tringleIndices, m_vertices );

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsSurface ::clearCachedNativeData()
{
}
