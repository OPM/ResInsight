/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimContourMapInViewCollection.h"

#include "RiaDefines.h"

#include "ContourMap/RimContourMapInView.h"
#include "ContourMap/RimEclipseContourMapView.h"
#include "Rim3dView.h"
#include "RimGridView.h"
#include "RimProject.h"

#include "cafPdmUiTreeOrdering.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimContourMapInViewCollection, "RimContourMapInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapInViewCollection::RimContourMapInViewCollection()
{
    CAF_PDM_InitObject( "Contour Maps" + RiaDefines::betaFeaturePostfix(), ":/2DMap16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_contourMapsInView, "ContourMapsInView", "Contour Maps" );

    // The name field is the user description field, and is what the project tree displays
    setName( "Contour Maps" + RiaDefines::betaFeaturePostfix() );
    nameField()->uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapInViewCollection::~RimContourMapInViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapInView* RimContourMapInViewCollection::findContourMapInViewForSource( const RimEclipseContourMapView* contourMapView ) const
{
    for ( RimContourMapInView* contourMapInView : m_contourMapsInView )
    {
        if ( contourMapInView && contourMapInView->contourMapView() == contourMapView ) return contourMapInView;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseContourMapView*> RimContourMapInViewCollection::allContourMapViewsInProject()
{
    std::vector<RimEclipseContourMapView*> contourMapViews;

    RimProject* project = RimProject::current();
    if ( !project ) return contourMapViews;

    // The ensemble statistics contour maps are owned by their ensemble and are not part of the oil field
    // collection, but they are views of the same type and show up here as well.
    for ( Rim3dView* view : project->allViews() )
    {
        if ( auto contourMapView = dynamic_cast<RimEclipseContourMapView*>( view ) ) contourMapViews.push_back( contourMapView );
    }

    return contourMapViews;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInViewCollection::updateFromContourMapCollection()
{
    // Sweep the wrappers whose contour map has been deleted. The pointer field is set to null by the
    // project data model when that happens.
    for ( RimContourMapInView* contourMapInView : m_contourMapsInView.childrenByType() )
    {
        if ( !contourMapInView->contourMapView() )
        {
            m_contourMapsInView.removeChild( contourMapInView );
            delete contourMapInView;
        }
    }

    // Find or create a wrapper per contour map, then reorder to match the project
    std::vector<RimContourMapInView*> orderedContourMapsInView;
    for ( RimEclipseContourMapView* contourMapView : allContourMapViewsInProject() )
    {
        RimContourMapInView* contourMapInView = findContourMapInViewForSource( contourMapView );
        if ( !contourMapInView )
        {
            contourMapInView = new RimContourMapInView;
            contourMapInView->setContourMapView( contourMapView );

            // A new contour map is not shown until the user asks for it
            contourMapInView->setCheckState( false );
        }

        orderedContourMapsInView.push_back( contourMapInView );
    }

    // Anything left over points at a contour map that is no longer part of the project
    for ( RimContourMapInView* contourMapInView : m_contourMapsInView.childrenByType() )
    {
        if ( std::find( orderedContourMapsInView.begin(), orderedContourMapsInView.end(), contourMapInView ) == orderedContourMapsInView.end() )
        {
            m_contourMapsInView.removeChild( contourMapInView );
            delete contourMapInView;
        }
    }

    m_contourMapsInView.clearWithoutDelete();
    for ( RimContourMapInView* contourMapInView : orderedContourMapsInView )
    {
        m_contourMapsInView.push_back( contourMapInView );
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimContourMapInView*> RimContourMapInViewCollection::allContourMapsInView() const
{
    return m_contourMapsInView.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimContourMapInView*> RimContourMapInViewCollection::visibleContourMapsInView() const
{
    if ( !isChecked() ) return {};

    std::vector<RimContourMapInView*> visibleContourMaps;
    for ( RimContourMapInView* contourMapInView : m_contourMapsInView )
    {
        if ( contourMapInView && contourMapInView->isChecked() ) visibleContourMaps.push_back( contourMapInView );
    }

    return visibleContourMaps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInViewCollection::appendPartsToModel( cvf::ModelBasicList*              model,
                                                        const caf::DisplayCoordTransform* displayCoordTransform,
                                                        const cvf::Camera*                camera )
{
    for ( RimContourMapInView* contourMapInView : visibleContourMapsInView() )
    {
        contourMapInView->appendPartsToModel( model, displayCoordTransform, camera );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimRegularLegendConfig*> RimContourMapInViewCollection::legendConfigs() const
{
    std::vector<RimRegularLegendConfig*> configs;
    for ( RimContourMapInView* contourMapInView : visibleContourMapsInView() )
    {
        if ( auto* legendConfig = contourMapInView->legendConfig() ) configs.push_back( legendConfig );
    }

    return configs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInViewCollection::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer )
{
    for ( RimContourMapInView* contourMapInView : visibleContourMapsInView() )
    {
        contourMapInView->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer, isUsingOverrideViewer );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInViewCollection::updateViewTreeItemsInAllViews()
{
    RimProject* project = RimProject::current();
    if ( !project ) return;

    for ( Rim3dView* view : project->allViews() )
    {
        if ( view ) view->updateViewTreeItems( RiaDefines::ItemIn3dView::CONTOUR_MAP );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInViewCollection::scheduleRedrawOfViewsShowing( const RimEclipseContourMapView* contourMapView )
{
    if ( !contourMapView ) return;

    RimProject* project = RimProject::current();
    if ( !project ) return;

    for ( Rim3dView* view : project->allViews() )
    {
        auto gridView = dynamic_cast<RimGridView*>( view );
        if ( !gridView ) continue;

        auto contourMaps = gridView->contourMapInViewCollection();
        if ( !contourMaps ) continue;

        bool showsContourMap = false;
        for ( RimContourMapInView* contourMapInView : contourMaps->visibleContourMapsInView() )
        {
            if ( contourMapInView->contourMapView() != contourMapView ) continue;

            // Only the drawables are stale. The topography raster follows the host view, which has not
            // changed, and rebuilding it here would re-raster on every time step change of the contour
            // map view.
            contourMapInView->clearPartMgr();
            showsContourMap = true;
        }

        if ( showsContourMap )
        {
            // The contour map parts belong to the frame scene, and are appended from
            // onUpdateDisplayModelForCurrentTimeStep. A scheduled redraw only reaches
            // createDisplayModelAndRedraw, which does not call that, so ask for the time step update
            // directly. The contour map has finished generating its data by the time we get here, so the
            // views pick up real geometry.
            gridView->updateDisplayModelForCurrentTimeStepAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimCheckableNamedObject::fieldChangedByUi( changedField, oldValue, newValue );

    updateUiIconFromToggleField();

    if ( changedField == &m_isChecked )
    {
        if ( auto view = firstAncestorOfType<Rim3dView>() )
        {
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInViewCollection::initAfterRead()
{
    updateUiIconFromToggleField();
}
