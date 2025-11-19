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

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSurfaceCollection.h"

#include "RigStatisticsMath.h"
#include "Surface/RigSurface.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfBoundingBox.h"

#include <algorithm>
#include <cmath>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimSurface, "SurfaceInterface" );

template <>
void caf::AppEnum<RimSurface::SurfaceType>::setUp()
{
    addItem( RimSurface::SurfaceType::DEFAULT, "DEFAULT", "Default" );
    addItem( RimSurface::SurfaceType::ENSEMBLE_SOURCE, "ENSEMBLE_SOURCE", "Ensemble Source" );
    addItem( RimSurface::SurfaceType::ENSEMBLE_STATISTICS, "ENSEMBLE_STATISTICS", "Ensemble Statistics" );

    setDefault( RimSurface::SurfaceType::DEFAULT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface::RimSurface()
{
    CAF_PDM_InitScriptableObject( "Surface", ":/ReservoirSurface16x16.png" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_userDescription, "SurfaceUserDescription", "Name" );
    m_userDescription.registerKeywordAlias( "SurfaceUserDecription" );

    CAF_PDM_InitField( &m_color, "SurfaceColor", cvf::Color3f( 0.5f, 0.3f, 0.2f ), "Color" );
    CAF_PDM_InitField( &m_enableOpacity, "EnableOpacity", false, "Enable Opacity" );
    CAF_PDM_InitField( &m_opacity, "Opacity", 0.6, "Opacity Value [0..1]" );
    m_opacity.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_depthOffset, "DepthOffset", 0.0, "Depth Offset" );
    m_depthOffset.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_depthOffset.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( true );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name Proxy" );
    m_nameProxy.registerGetMethod( this, &RimSurface::fullName );
    m_nameProxy.uiCapability()->setUiReadOnly( true );
    m_nameProxy.uiCapability()->setUiHidden( true );
    m_nameProxy.xmlCapability()->disableIO();

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
std::pair<bool, float> RimSurface::opacity() const
{
    return std::make_pair( m_enableOpacity(), m_opacity() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::setOpacity( bool useOpacity, float opacity )
{
    m_enableOpacity.setValue( useOpacity );
    m_opacity.setValue( opacity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSurface::showIntersectionCellResults()
{
    return true;
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
caf::PdmFieldHandle* RimSurface::userDescriptionField()
{
    return &m_nameProxy;
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
void RimSurface::setDepthOffset( double depthoffset )
{
    m_depthOffset.setValue( depthoffset );
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
bool RimSurface::loadSurfaceDataForTimeStep( int timeStep )
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSurface::timeStepCount() const
{
    return 1;
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
        updateConnectedEditors();
    }
    else if ( changedField == &m_depthOffset )
    {
        onLoadData();

        updateViews = true;
    }

    if ( updateViews )
    {
        auto surfColl = firstAncestorOrThisOfTypeAsserted<RimSurfaceCollection>();
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

//--------------------------------------------------------------------------------------------------
/// Return the name to show in the tree selector, including the depth offset if not 0
//--------------------------------------------------------------------------------------------------
QString RimSurface::fullName() const
{
    if ( depthOffset() != 0.0 )
    {
        QString name = m_userDescription;
        if ( !name.isEmpty() ) name += " - ";
        name += QString( "Offset:%1" ).arg( QString::number( depthOffset() ) );

        return name;
    }

    return QString( "%1" ).arg( m_userDescription );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    auto doubleSliderAttrib = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
    if ( doubleSliderAttrib )
    {
        if ( field == &m_depthOffset )
        {
            RiaApplication*       app   = RiaApplication::instance();
            std::vector<RimCase*> cases = app->project()->allGridCases();

            cvf::BoundingBox bb;

            for ( auto c : cases )
            {
                bb.add( c->allCellsBoundingBox() );
            }

            double extentFromCase = std::fabs( bb.extent().z() ) * 2.0;
            extentFromCase        = std::floor( extentFromCase / 1000.0 ) * 1000.0;

            double minimumExtent = std::max( 1000.0, extentFromCase );

            doubleSliderAttrib->m_minimum = -minimumExtent;
            doubleSliderAttrib->m_maximum = minimumExtent;
        }
    }

    if ( field == &m_opacity )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            attr->m_minimum = 0.0;
            attr->m_maximum = 1.0;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::updateMinMaxValues( RimRegularLegendConfig* legend, const QString& propertyName, int currentTimeStep ) const
{
    if ( !m_surfaceData.isNull() )
    {
        MinMaxAccumulator minMaxAccumulator;
        PosNegAccumulator posNegAccumulator;

        auto values = m_surfaceData->propertyValues( propertyName );
        minMaxAccumulator.addData( values );
        posNegAccumulator.addData( values );

        double globalPosClosestToZero = posNegAccumulator.pos;
        double globalNegClosestToZero = posNegAccumulator.neg;
        double globalMin              = minMaxAccumulator.min;
        double globalMax              = minMaxAccumulator.max;

        legend->setClosestToZeroValues( globalPosClosestToZero, globalNegClosestToZero, globalPosClosestToZero, globalNegClosestToZero );
        legend->setAutomaticRanges( globalMin, globalMax, globalMin, globalMax );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSurface::isMeshLinesEnabledDefault() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimSurface::boundingBoxInDomainCoords() const
{
    cvf::BoundingBox boundingBox;

    if ( m_surfaceData.notNull() )
    {
        for ( const auto& vertex : m_surfaceData->vertices() )
            boundingBox.add( vertex );
    }

    return boundingBox;
}
