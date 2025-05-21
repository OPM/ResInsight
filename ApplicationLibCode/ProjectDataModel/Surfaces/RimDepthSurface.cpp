/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimDepthSurface.h"

#include "Surface/RigSurface.h"

#include "RimSurfaceCollection.h"

#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiDoubleValueEditor.h"

#include "cvfVector3.h"

CAF_PDM_SOURCE_INIT( RimDepthSurface, "DepthSurface", "RimDepthSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDepthSurface::RimDepthSurface()
{
    CAF_PDM_InitScriptableObject( "DepthSurface", ":/ReservoirSurface16x16.png" );

    CAF_PDM_InitField( &m_minX, "MinX", 0.0, "Min X" );
    CAF_PDM_InitField( &m_maxX, "MaxX", 0.0, "Max X" );
    CAF_PDM_InitField( &m_minY, "MinY", 0.0, "Min Y" );
    CAF_PDM_InitField( &m_maxY, "MaxY", 0.0, "Min Y" );
    CAF_PDM_InitField( &m_depth, "Depth", 0.0, "Depth" );

    CAF_PDM_InitField( &m_depthLowerLimit, "DepthLowerLimit", 0.0, "Lower Limit" );
    m_depthLowerLimit.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_depthUpperLimit, "DepthUpperLimit", 100000.0, "Upper Limit" );
    m_depthUpperLimit.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    m_minX.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_maxX.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_minY.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_maxY.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_depth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_areaOfInterestMin, "AreaOfInterestMin", "Area Of Interest Min" );
    CAF_PDM_InitFieldNoDefault( &m_areaOfInterestMax, "AreaOfInterestMax", "Area Of Interest Max" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDepthSurface::~RimDepthSurface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDepthSurface::onLoadData()
{
    return updateSurfaceData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimDepthSurface::createCopy()
{
    return copyObject<RimDepthSurface>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDepthSurface::showIntersectionCellResults()
{
    // Avoid use of cell intersection results color for depth surfaces
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthSurface::setPlaneExtent( double minX, double minY, double maxX, double maxY )
{
    m_minX = minX;
    m_minY = minY;
    m_maxX = maxX;
    m_maxY = maxY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthSurface::setDepth( double depth )
{
    m_depth = depth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthSurface::setDepthSliderLimits( double lower, double upper )
{
    m_depthLowerLimit = lower;
    m_depthUpperLimit = upper;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthSurface::setAreaOfInterest( cvf::Vec3d min, cvf::Vec3d max )
{
    m_areaOfInterestMin = min;
    m_areaOfInterestMax = max;

    auto lowerDepthLimit = -max.z();
    auto upperDepthLimit = -min.z();
    setDepthSliderLimits( lowerDepthLimit, upperDepthLimit );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimSurface::fieldChangedByUi( changedField, oldValue, newValue );

    clearCachedNativeData();
    updateSurfaceData();

    auto surfColl = firstAncestorOrThisOfTypeAsserted<RimSurfaceCollection>();
    surfColl->updateViews( { this } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthSurface::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    RimSurface::defineEditorAttribute( field, uiConfigName, attribute );

    caf::PdmUiDoubleValueEditorAttribute::testAndSetFixedWithTwoDecimals( attribute );

    if ( field == &m_depth )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            attr->m_minimum = m_depthLowerLimit;
            attr->m_maximum = m_depthUpperLimit;
        }
    }

    if ( field == &m_minX || field == &m_maxX )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            attr->m_minimum = m_areaOfInterestMin().x();
            attr->m_maximum = m_areaOfInterestMax().x();
        }
    }

    if ( field == &m_minY || field == &m_maxY )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            attr->m_minimum = m_areaOfInterestMin().y();
            attr->m_maximum = m_areaOfInterestMax().y();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthSurface::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_depth );

    {
        auto group = uiOrdering.addNewGroup( "Depth Limits" );
        group->setCollapsedByDefault();
        group->add( &m_depthLowerLimit );
        group->add( &m_depthUpperLimit );
    }

    {
        auto group = uiOrdering.addNewGroup( "Extent" );
        group->add( &m_minX );
        group->add( &m_maxX );
        group->add( &m_minY );
        group->add( &m_maxY );
    }

    {
        // Fields from RimSurface

        auto group = uiOrdering.addNewGroup( "Appearance" );
        group->add( &m_userDescription );
        group->add( &m_color );

        group->add( &m_enableOpacity );
        group->add( &m_opacity );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthSurface::clearCachedNativeData()
{
    m_vertices.clear();
    m_triangleIndices.clear();
}

//--------------------------------------------------------------------------------------------------
/// Returns false for fatal failure
//--------------------------------------------------------------------------------------------------
bool RimDepthSurface::updateSurfaceData()
{
    auto displayZ = -m_depth;

    cvf::Vec3d a( m_minX, m_minY, displayZ );
    cvf::Vec3d b( m_maxX, m_minY, displayZ );
    cvf::Vec3d c( m_maxX, m_maxY, displayZ );
    cvf::Vec3d d( m_minX, m_maxY, displayZ );

    m_vertices.push_back( a );
    m_vertices.push_back( b );
    m_vertices.push_back( c );
    m_vertices.push_back( d );

    m_triangleIndices.push_back( 0 );
    m_triangleIndices.push_back( 1 );
    m_triangleIndices.push_back( 2 );

    m_triangleIndices.push_back( 0 );
    m_triangleIndices.push_back( 2 );
    m_triangleIndices.push_back( 3 );

    if ( !m_triangleIndices.empty() )
    {
        std::vector<unsigned>   tringleIndices{ m_triangleIndices };
        std::vector<cvf::Vec3d> vertices{ m_vertices };

        cvf::Vec3d offset;
        offset.z() += depthOffset();

        RimSurface::applyDepthOffset( offset, &vertices );
        auto surfaceData = new RigSurface;
        surfaceData->setTriangleData( tringleIndices, vertices );

        setSurfaceData( surfaceData );
    }
    else
    {
        setSurfaceData( nullptr );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Return the name to show in the project tree
//--------------------------------------------------------------------------------------------------
QString RimDepthSurface::fullName() const
{
    QString retval = RimSurface::fullName();
    if ( !retval.isEmpty() ) retval += " - ";
    retval += "Depth: ";
    retval += QString::number( m_depth );
    return retval;
}
