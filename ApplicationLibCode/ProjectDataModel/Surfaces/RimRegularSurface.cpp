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

#include "RimRegularSurface.h"

#include "Surface/RigSurface.h"

#include "RimSurfaceCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfVector3.h"

#include <numbers>

CAF_PDM_SOURCE_INIT( RimRegularSurface, "RegularSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularSurface::RimRegularSurface()
{
    CAF_PDM_InitScriptableObject( "RegularSurface", ":/ReservoirSurface16x16.png" );

    CAF_PDM_InitScriptableField( &m_originX, "OriginX", 0.0, "Origin X" );
    CAF_PDM_InitScriptableField( &m_originY, "OriginY", 0.0, "Origin Y" );
    CAF_PDM_InitScriptableField( &m_depth, "Depth", 0.0, "Depth" );
    m_depth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_nx, "Nx", 10, "Nx" );
    CAF_PDM_InitScriptableField( &m_ny, "Ny", 10, "Ny" );
    CAF_PDM_InitScriptableField( &m_incrementX, "IncrementX", 20.0, "Increment X" );
    CAF_PDM_InitScriptableField( &m_incrementY, "IncrementY", 20.0, "Increment Y" );

    CAF_PDM_InitScriptableField( &m_rotation, "Rotation", 0.0, "Rotation" );

    CAF_PDM_InitScriptableField( &m_depthProperty, "DepthProperty", QString( "" ), "Depth Property" );
    m_rotation.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimRegularSurface::onLoadData()
{
    clearCachedNativeData();
    return updateSurfaceData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimRegularSurface::createCopy()
{
    return copyObject<RimRegularSurface>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimSurface::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_nx || changedField == &m_ny )
    {
        // Invalidate all properties when dimensions changes
        m_properties.clear();
        m_depthProperty = "";
    }

    clearCachedNativeData();
    updateSurfaceData();

    auto surfColl = firstAncestorOrThisOfTypeAsserted<RimSurfaceCollection>();
    surfColl->updateViews( { this } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    RimSurface::defineEditorAttribute( field, uiConfigName, attribute );

    if ( field == &m_rotation )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            attr->m_minimum = 0.0;
            attr->m_maximum = 360.0;
        }
    }

    if ( field == &m_depth )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            attr->m_minimum = 0.0;
            attr->m_maximum = 10000.0;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto locationGroup = uiOrdering.addNewGroup( "Location" );
    locationGroup->add( &m_originX );
    locationGroup->add( &m_originY );
    locationGroup->add( &m_depth );

    auto extentGroup = uiOrdering.addNewGroup( "Extent" );
    extentGroup->add( &m_nx );
    extentGroup->add( &m_ny );
    extentGroup->add( &m_incrementX );
    extentGroup->add( &m_incrementY );

    uiOrdering.add( &m_rotation );
    uiOrdering.add( &m_depthOffset );

    // Fields from RimSurface
    auto appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    appearanceGroup->add( &m_userDescription );
    appearanceGroup->add( &m_color );

    appearanceGroup->add( &m_enableOpacity );
    appearanceGroup->add( &m_opacity );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::clearCachedNativeData()
{
    m_vertices.clear();
    m_triangleIndices.clear();
}

//--------------------------------------------------------------------------------------------------
/// Returns false for fatal failure
//--------------------------------------------------------------------------------------------------
bool RimRegularSurface::updateSurfaceData()
{
    // Adapted from xtgeo surf_xyz_from_ij
    auto computeXyzFromIj =
        []( int i, int j, double depth, double xori, double yori, double xinc, double yinc, double rotationDegrees ) -> cvf::Vec3d
    {
        CAF_ASSERT( i >= 0 );
        CAF_ASSERT( j >= 0 );

        double z = -depth;

        if ( i == 0 && j == 0 ) return cvf::Vec3d( xori, yori, z );

        // Surface rotation: this should be the usual angle, anti-clock from x axis,
        // radians, positive
        const double angle = rotationDegrees * std::numbers::pi / 180.0;

        const double xdist = xinc * i;
        const double ydist = yinc * j;

        // Distance of point from "origo"
        const double dist = sqrt( xdist * xdist + ydist * ydist );

        const double beta = acos( xdist / dist );

        if ( beta < 0 || beta > std::numbers::pi / 2.0 || std::isnan( beta ) ) return cvf::Vec3d::UNDEFINED;

        // The difference in rotated coord system
        double gamma = angle + beta;

        double dxrot = dist * cos( gamma );
        double dyrot = dist * sin( gamma );

        return cvf::Vec3d( xori + dxrot, yori + dyrot, z );
    };

    for ( int j = 0; j < m_ny(); j++ )
    {
        for ( int i = 0; i < m_nx(); i++ )
        {
            double depth = m_depth;
            if ( !m_depthProperty().isEmpty() && m_properties.contains( m_depthProperty ) )
            {
                depth = m_properties[m_depthProperty][j * m_nx + i];
            }
            m_vertices.push_back( computeXyzFromIj( i, j, depth, m_originX, m_originY, m_incrementX, m_incrementY, m_rotation ) );
        }
    }

    // Iterate through the entire grid
    for ( int j = 0; j < m_ny() - 1; j++ )
    {
        for ( int i = 0; i < m_nx() - 1; i++ )
        {
            // Calculate the indices of the four vertices in the current cell
            // Top-left vertex
            size_t idx00 = j * m_nx() + i;
            // Top-right vertex
            size_t idx01 = j * m_nx() + ( i + 1 );
            // Bottom-left vertex
            size_t idx10 = ( j + 1 ) * m_nx() + i;
            // Bottom-right vertex
            size_t idx11 = ( j + 1 ) * m_nx() + ( i + 1 );

            // First triangle (top-left, top-right, bottom-left)
            m_triangleIndices.push_back( static_cast<unsigned>( idx00 ) );
            m_triangleIndices.push_back( static_cast<unsigned>( idx01 ) );
            m_triangleIndices.push_back( static_cast<unsigned>( idx10 ) );

            // Second triangle (top-right, bottom-right, bottom-left)
            m_triangleIndices.push_back( static_cast<unsigned>( idx01 ) );
            m_triangleIndices.push_back( static_cast<unsigned>( idx11 ) );
            m_triangleIndices.push_back( static_cast<unsigned>( idx10 ) );
        }
    }

    if ( !m_triangleIndices.empty() )
    {
        std::vector<unsigned>   tringleIndices{ m_triangleIndices };
        std::vector<cvf::Vec3d> vertices{ m_vertices };

        cvf::Vec3d offset;
        offset.z() += depthOffset();

        RimSurface::applyDepthOffset( offset, &vertices );
        auto surfaceData = new RigSurface;
        surfaceData->setTriangleData( tringleIndices, vertices );

        for ( auto& [key, vec] : m_properties )
        {
            surfaceData->addVertexResult( key, vec );
        }

        setSurfaceData( surfaceData );
    }
    else
    {
        setSurfaceData( nullptr );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::setOriginX( double originX )
{
    m_originX = originX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::setOriginY( double originY )
{
    m_originY = originY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::setNx( int nx )
{
    CAF_ASSERT( nx > 0 );
    m_nx = nx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::setNy( int ny )
{
    CAF_ASSERT( ny > 0 );
    m_ny = ny;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::setDepth( double depth )
{
    m_depth = depth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::setIncrementX( double incrementX )
{
    CAF_ASSERT( incrementX > 0.0 );
    m_incrementX = incrementX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::setIncrementY( double incrementY )
{
    CAF_ASSERT( incrementY > 0.0 );
    m_incrementY = incrementY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::setRotation( double rotation )
{
    CAF_ASSERT( rotation >= 0.0 && rotation <= 360.0 );
    m_rotation = rotation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularSurface::setProperty( const QString& key, const std::vector<float>& values )
{
    CAF_ASSERT( values.size() == static_cast<size_t>( m_nx() * m_ny() ) );
    m_properties[key] = values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimRegularSurface::setPropertyAsDepth( const QString& key )
{
    if ( !m_properties.contains( key ) ) return false;

    m_depthProperty = key;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimRegularSurface::nx() const
{
    return m_nx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimRegularSurface::ny() const
{
    return m_ny;
}
