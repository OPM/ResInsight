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

#include "RimSurfaceInView.h"

#include "RigFemPartCollection.h"
#include "RigSurface.h"

#include "RimEclipseView.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimRegularLegendConfig.h"
#include "RimSurface.h"
#include "RimSurfaceResultDefinition.h"

#include "RiuViewer.h"

#include "RivSurfacePartMgr.h"

#include "cafPdmUiDoubleSliderEditor.h"

CAF_PDM_SOURCE_INIT( RimSurfaceInView, "SurfaceInView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInView::RimSurfaceInView()
{
    CAF_PDM_InitObject( "Surface", ":/ReservoirSurface16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name", "", "", "" );
    m_name.registerGetMethod( this, &RimSurfaceInView::name );
    m_name.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_surface, "SurfaceRef", "Surface", "", "", "" );
    m_surface.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_resultDefinition, "ResultDefinition", "Result Definition", "", "", "" );
    m_resultDefinition.uiCapability()->setUiHidden( true );
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_resultDefinition = new RimSurfaceResultDefinition;
    m_resultDefinition->setCheckState( false );
    m_resultDefinition->setSurfaceInView( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInView::~RimSurfaceInView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSurfaceInView::name() const
{
    if ( m_surface ) return m_surface->fullName();

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceInView::surface() const
{
    return m_surface();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInView::setSurface( RimSurface* surf )
{
    m_surface = surf;

    if ( surface()->surfaceData() && surface()->surfaceData()->propertyNames().empty() )
    {
        m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
        m_resultDefinition->setCheckState( false );
    }
    else
    {
        m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( false );
        m_resultDefinition->setCheckState( true );

        m_resultDefinition->assignDefaultProperty();
    }

    m_resultDefinition->updateMinMaxValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSurfaceInView::isNativeSurfaceResultsActive() const
{
    return m_resultDefinition->isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceResultDefinition* RimSurfaceInView::surfaceResultDefinition()
{
    return m_resultDefinition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInView::clearGeometry()
{
    m_surfacePartMgr = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSurfacePartMgr* RimSurfaceInView::surfacePartMgr()
{
    if ( m_surfacePartMgr.isNull() ) m_surfacePartMgr = new RivSurfacePartMgr( this );

    return m_surfacePartMgr.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RivIntersectionGeometryGeneratorInterface* RimSurfaceInView::intersectionGeometryGenerator() const
{
    if ( m_surfacePartMgr.notNull() ) return m_surfacePartMgr->intersectionGeometryGenerator();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInView::loadDataAndUpdate()
{
    if ( surface() )
    {
        surface()->loadDataIfRequired();

        if ( surface()->surfaceData() && surface()->surfaceData()->propertyNames().empty() )
        {
            m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
            m_resultDefinition->setCheckState( false );
        }
        else
        {
            m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( false );
            m_resultDefinition->setCheckState( true );
        }

        m_resultDefinition->updateMinMaxValues();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInView::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer )
{
    if ( m_resultDefinition->legendConfig() )
    {
        RimRegularLegendConfig* legendConfig = m_resultDefinition->legendConfig();

        legendConfig->setTitle(
            QString( "Surface : \n%1\n%2" ).arg( this->name() ).arg( m_resultDefinition->propertyName() ) );

        if ( this->isActive() && m_resultDefinition->isChecked() && legendConfig->showLegend() )
        {
            nativeOrOverrideViewer->addColorLegendToBottomLeftCorner( legendConfig->titledOverlayFrame(),
                                                                      isUsingOverrideViewer );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInView::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue )
{
    this->updateUiIconFromToggleField();

    bool scheduleRedraw = false;

    if ( changedField == &m_useSeparateDataSource || changedField == &m_separateDataSource )
    {
        scheduleRedraw = true;
    }
    else if ( changedField == &m_isActive )
    {
        // if ( m_isActive ) clearGeometry();

        scheduleRedraw = true;
    }

    if ( changedField == &m_showInactiveCells )
    {
        clearGeometry();
        scheduleRedraw = true;
    }

    if ( scheduleRedraw )
    {
        RimGridView* ownerView;
        this->firstAncestorOrThisOfTypeAsserted( ownerView );
        ownerView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_showInactiveCells );

    this->defineSeparateDataSourceUi( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultsDefinitionCollection* RimSurfaceInView::findSeparateResultsCollection()
{
    RimGridView* view;
    this->firstAncestorOrThisOfTypeAsserted( view );
    return view->separateSurfaceResultsCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSurfaceInView::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInView::initAfterRead()
{
    this->updateUiIconFromToggleField();
}
