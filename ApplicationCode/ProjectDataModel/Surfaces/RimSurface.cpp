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

#include "RimSurface.h"

#include "RimSurfaceCollection.h"

#include "RigSurface.h"

#include "RifSurfaceReader.h"

#include "cafPdmUiDoubleSliderEditor.h"

#include <cmath>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimSurface, "SurfaceInterface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface::RimSurface()
{
    CAF_PDM_InitObject( "Surface", ":/ReservoirSurface16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_userDescription, "SurfaceUserDecription", "Name", "", "", "" );
    CAF_PDM_InitField( &m_color, "SurfaceColor", cvf::Color3f( 0.5f, 0.3f, 0.2f ), "Color", "", "", "" );

    CAF_PDM_InitField( &m_depthOffset, "DepthOffset", 0.0, "Depth Offset", "", "", "" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface::~RimSurface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::setColor( const cvf::Color3f& color )
{
    m_color = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimSurface::color() const
{
    return m_color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSurface::userDescription()
{
    return m_userDescription();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::loadDataIfRequired()
{
    if ( m_surfaceData.isNull() )
    {
        onLoadData();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::setUserDescription( const QString& description )
{
    m_userDescription = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::setSurfaceData( RigSurface* surface )
{
    m_surfaceData = surface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::applyDepthOffsetIfNeeded( std::vector<cvf::Vec3d>* vertices ) const
{
    double epsilon = 1.0e-10;

    if ( std::fabs( m_depthOffset ) > epsilon )
    {
        cvf::Vec3d offset = cvf::Vec3d::ZERO;

        offset.z() += m_depthOffset;

        RimSurface::applyDepthOffset( offset, vertices );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSurface::depthOffset() const
{
    return m_depthOffset;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::applyDepthOffset( const cvf::Vec3d& offset, std::vector<cvf::Vec3d>* vertices )
{
    if ( vertices )
    {
        for ( auto& v : *vertices )
        {
            v += offset;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSurface* RimSurface::surfaceData()
{
    return m_surfaceData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSurface::userDescriptionField()
{
    return &m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    bool updateViews = false;

    if ( changedField == &m_color )
    {
        updateViews = true;
    }
    else if ( changedField == &m_userDescription )
    {
        this->updateConnectedEditors();
    }
    else if ( changedField == &m_depthOffset )
    {
        this->onLoadData();

        updateViews = true;
    }

    if ( updateViews )
    {
        RimSurfaceCollection* surfColl;
        this->firstAncestorOrThisOfTypeAsserted( surfColl );
        surfColl->updateViews( { this } );
    }
}

//--------------------------------------------------------------------------------------------------
/// Make the surface clear its internal data and reload them from the source data (i.e. file or grid)
//--------------------------------------------------------------------------------------------------
void RimSurface::reloadData()
{
    clearCachedNativeData();
    updateSurfaceData();
}
