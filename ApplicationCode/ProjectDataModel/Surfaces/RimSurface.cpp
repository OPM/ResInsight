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
#include "RimSurfaceCollection.h"

#include "RigSurface.h"

#include "RifSurfaceImporter.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfBoundingBox.h"

#include <algorithm>
#include <cmath>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimSurface, "SurfaceInterface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface::RimSurface()
{
    CAF_PDM_InitScriptableObject( "Surface", ":/ReservoirSurface16x16.png", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_userDescription, "SurfaceUserDecription", "Name", "", "", "" );
    CAF_PDM_InitField( &m_color, "SurfaceColor", cvf::Color3f( 0.5f, 0.3f, 0.2f ), "Color", "", "", "" );

    CAF_PDM_InitScriptableField( &m_depthOffset, "DepthOffset", 0.0, "Depth Offset", "", "", "" );
    m_depthOffset.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_depthOffset.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( true );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name Proxy", "", "", "" );
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

//--------------------------------------------------------------------------------------------------
/// Return the name to show in the tree selector, including the depth offset if not 0
//--------------------------------------------------------------------------------------------------
QString RimSurface::fullName() const
{
    if ( depthOffset() != 0.0 )
    {
        return QString( "%1 - Offset:%2" ).arg( m_userDescription, QString::number( depthOffset() ) );
    }

    return QString( "%1" ).arg( m_userDescription );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurface::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                        QString                    uiConfigName,
                                        caf::PdmUiEditorAttribute* attribute )
{
    auto doubleSliderAttrib = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
    if ( doubleSliderAttrib )
    {
        if ( field == &m_depthOffset )
        {
            RiaApplication*       app = RiaApplication::instance();
            std::vector<RimCase*> cases;
            app->project()->allCases( cases );

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
}
