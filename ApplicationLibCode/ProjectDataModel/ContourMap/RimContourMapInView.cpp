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

#include "RimContourMapInView.h"

#include "ContourMap/RigContourMapGrid.h"
#include "ContourMap/RigContourMapTopography.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "Surface/RigSurface.h"

#include "ContourMap/RimContourMapProjection.h"
#include "ContourMap/RimEclipseContourMapView.h"
#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGridView.h"
#include "RimRegularLegendConfig.h"
#include "Surfaces/RimSurface.h"
#include "Surfaces/RimSurfaceInView.h"
#include "Surfaces/RimSurfaceInViewCollection.h"

#include "RivContourMapElevationProvider.h"
#include "RivContourMapProjectionPartMgr.h"

#include "RiuViewer.h"

#include "cvfCamera.h"
#include "cvfModelBasicList.h"

CAF_PDM_SOURCE_INIT( RimContourMapInView, "RimContourMapInView" );

namespace caf
{
template <>
void AppEnum<RimContourMapInView::MapPosition>::setUp()
{
    addItem( RimContourMapInView::MapPosition::TOP_OF_CASE, "TOP_OF_CASE", "Top of Case" );
    addItem( RimContourMapInView::MapPosition::BOTTOM_OF_CASE, "BOTTOM_OF_CASE", "Bottom of Case" );
    addItem( RimContourMapInView::MapPosition::USER_DEFINED_DEPTH, "USER_DEFINED_DEPTH", "User Defined Depth" );
    setDefault( RimContourMapInView::MapPosition::TOP_OF_CASE );
}

template <>
void AppEnum<RimContourMapInView::LineColorMode>::setUp()
{
    addItem( RimContourMapInView::LineColorMode::CONTRAST_TO_MAP, "CONTRAST_TO_MAP", "Contrast to Map" );
    addItem( RimContourMapInView::LineColorMode::SINGLE_COLOR, "SINGLE_COLOR", "Single Color" );
    setDefault( RimContourMapInView::LineColorMode::CONTRAST_TO_MAP );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapInView::RimContourMapInView()
{
    CAF_PDM_InitObject( "Contour Map", ":/2DMap16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_contourMapView, "ContourMapView", "Contour Map" );
    m_contourMapView.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "ContourMapName", "Name" );
    m_nameProxy.registerGetMethod( this, &RimContourMapInView::name );
    m_nameProxy.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_mapPosition, "MapPosition", "Map Position" );

    CAF_PDM_InitField( &m_depthOffset, "DepthOffset", 0.0, "Depth Offset" );
    m_depthOffset.uiCapability()->setUiToolTip( "Positive values move the map upwards." );

    CAF_PDM_InitField( &m_userDefinedDepth, "UserDefinedDepth", 0.0, "Depth" );

    CAF_PDM_InitField( &m_showMapSurface, "ShowMapSurface", true, "Show Map Surface" );
    CAF_PDM_InitField( &m_showContourLines, "ShowContourLines", true, "Show Contour Lines" );

    CAF_PDM_InitField( &m_projectSurfaceOnGeometry, "ProjectSurfaceOnGeometry", false, "Project Surface on Geometry" );
    m_projectSurfaceOnGeometry.uiCapability()->setUiToolTip(
        "Follow the top of the visible geometry instead of drawing the map surface on the map plane." );

    CAF_PDM_InitField( &m_projectLinesOnGeometry, "ProjectLinesOnGeometry", false, "Project Lines on Geometry" );
    m_projectLinesOnGeometry.uiCapability()->setUiToolTip(
        "Follow the top of the visible geometry instead of drawing the contour lines on the map plane." );

    CAF_PDM_InitField( &m_showContourLabels, "ShowContourLabels", false, "Show Contour Labels" );

    CAF_PDM_InitField( &m_labelFontSize, "LabelFontSize", RiaFontCache::FontSizeEnum( RiaFontCache::FontSize::FONT_SIZE_10 ), "Label Font Size" );

    CAF_PDM_InitField( &m_lineColorMode, "LineColorMode", caf::AppEnum<LineColorMode>( LineColorMode::CONTRAST_TO_MAP ), "Line Color Mode" );
    m_lineColorMode.uiCapability()->setUiToolTip( "Contrast to Map gives every contour level a color contrasting the map underneath it." );

    CAF_PDM_InitField( &m_lineColor, "LineColor", cvf::Color3f( cvf::Color3f::BLACK ), "Line Color" );

    CAF_PDM_InitField( &m_lineThickness, "LineThickness", 1, "Line Thickness" );
    m_lineThickness.setRange( 1, 10 );

    nameField()->uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapInView::~RimContourMapInView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapView* RimContourMapInView::contourMapView() const
{
    return m_contourMapView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapView* RimContourMapInView::sourceItem() const
{
    return contourMapView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInView::setContourMapView( RimEclipseContourMapView* contourMapView )
{
    m_contourMapView = contourMapView;

    clearGeometry();
    updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimContourMapInView::name() const
{
    if ( !m_contourMapView() ) return "Contour Map";

    return m_contourMapView()->name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimContourMapInView::userDescriptionField()
{
    return &m_nameProxy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimContourMapProjection* RimContourMapInView::contourMapProjection() const
{
    if ( !m_contourMapView() ) return nullptr;

    return m_contourMapView()->contourMapProjection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivContourMapProjectionPartMgr* RimContourMapInView::partMgr()
{
    if ( m_partMgr.isNull() ) m_partMgr = new RivContourMapProjectionPartMgr( contourMapProjection() );

    return m_partMgr.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInView::clearPartMgr()
{
    m_partMgr = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInView::clearGeometry()
{
    clearPartMgr();

    m_topography               = nullptr;
    m_topographyMapGrid        = nullptr;
    m_topographyMainGrid       = nullptr;
    m_topographyCellVisibility = nullptr;
    m_topographySurfaces.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> RimContourMapInView::mapElevation() const
{
    auto projection = contourMapProjection();
    if ( !projection ) return {};

    const RigContourMapGrid* mapGrid = projection->mapGrid();
    if ( !mapGrid ) return {};

    double elevation = 0.0;
    switch ( m_mapPosition() )
    {
        case MapPosition::TOP_OF_CASE:
            elevation = mapGrid->expandedBoundingBox().max().z();
            break;
        case MapPosition::BOTTOM_OF_CASE:
            elevation = mapGrid->origin3d().z();
            break;
        case MapPosition::USER_DEFINED_DEPTH:
            elevation = -m_userDefinedDepth();
            break;
    }

    return elevation + m_depthOffset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapInView::mapSampleSpacing() const
{
    auto projection = contourMapProjection();
    if ( !projection || !projection->mapGrid() ) return 0.0;

    return projection->mapGrid()->sampleSpacing();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapInView::surfaceDrapeLift() const
{
    // Draped straight onto the geometry the map surface would be coplanar with it and the two would
    // z-fight. Lift it clear instead. Scales with the model, and is small enough to be invisible at any
    // reasonable zoom level.
    return 0.01 * mapSampleSpacing();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimContourMapInView::contourLineLift() const
{
    // Twice the lift of the map surface, so the lines stay on top of it whether it is draped or flat
    return 2.0 * surfaceDrapeLift();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<const RigContourMapTopography> RimContourMapInView::topography()
{
    auto projection = contourMapProjection();
    if ( !projection || !projection->mapGrid() ) return nullptr;

    // The drape follows the geometry of the view the contour map is shown in, not the geometry of the
    // case the contour map was computed from.
    auto hostView = firstAncestorOfType<RimEclipseView>();
    if ( !hostView || !hostView->eclipseCase() || !hostView->eclipseCase()->eclipseCaseData() ) return nullptr;

    RigMainGrid* mainGrid = hostView->eclipseCase()->eclipseCaseData()->mainGrid();
    if ( !mainGrid ) return nullptr;

    const RigContourMapGrid*  mapGrid        = projection->mapGrid();
    cvf::ref<cvf::UByteArray> cellVisibility = hostView->currentTotalCellVisibility();

    // A view can be showing surfaces and no grid cells at all, so both are drape targets
    std::vector<RigSurface*> surfaces;
    if ( auto surfaceCollection = hostView->surfaceInViewCollection() )
    {
        for ( RimSurfaceInView* surfaceInView : surfaceCollection->visibleSurfacesInView() )
        {
            if ( !surfaceInView->surface() ) continue;

            if ( RigSurface* surfaceData = surfaceInView->surface()->surfaceData() ) surfaces.push_back( surfaceData );
        }
    }

    // Cell filters and property filters make the view produce a new visibility array, a resolution
    // change makes the projection produce a new map grid, reloading the case produces a new main grid,
    // and surfaces come and go as they are checked. Any of those invalidates the raster.
    if ( m_topography && m_topographyMapGrid == mapGrid && m_topographyMainGrid == mainGrid &&
         m_topographyCellVisibility.p() == cellVisibility.p() && m_topographySurfaces == surfaces )
    {
        return m_topography;
    }

    m_topography               = std::make_shared<const RigContourMapTopography>( *mapGrid, *mainGrid, cellVisibility.p(), surfaces );
    m_topographyMapGrid        = mapGrid;
    m_topographyMainGrid       = mainGrid;
    m_topographyCellVisibility = cellVisibility.p();
    m_topographySurfaces       = surfaces;

    return m_topography;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInView::appendPartsToModel( cvf::ModelBasicList*              model,
                                              const caf::DisplayCoordTransform* displayCoordTransform,
                                              const cvf::Camera*                camera )
{
    if ( !isChecked() || !model || !displayCoordTransform ) return;

    auto sourceView = m_contourMapView();
    if ( !sourceView || !sourceView->eclipseCase() || !sourceView->eclipseCase()->eclipseCaseData() ) return;

    auto projection = contourMapProjection();
    if ( !projection || !projection->isChecked() ) return;

    // The projection is owned by the contour map view and shared with it, so it is generated for the
    // time step that view is on. Driving it to the host view's time step instead would leave the contour
    // map view holding results for a step it is not showing, and make the two views recompute the whole
    // projection against each other on every redraw. Generating is still needed here, since the contour
    // map view may never have been opened.
    projection->generateResultsIfNecessary( sourceView->currentTimeStep() );
    projection->generateGeometryIfNecessary();

    if ( !projection->mapGrid() ) return;

    auto* mapLegendConfig = projection->legendConfig();
    if ( !mapLegendConfig || !mapLegendConfig->scalarMapper() ) return;

    auto elevation = mapElevation();
    if ( !elevation ) return;

    // Falls back to the map plane when the topography cannot be built, for instance when the host view
    // has no loaded grid
    std::shared_ptr<const RigContourMapTopography> mapTopography;
    if ( m_projectSurfaceOnGeometry() || m_projectLinesOnGeometry() ) mapTopography = topography();

    RivContourMapFlatElevation flatElevation( *elevation );

    if ( m_showMapSurface() )
    {
        std::unique_ptr<RivContourMapTopographyElevation> surfaceTopography;
        if ( m_projectSurfaceOnGeometry() && mapTopography )
        {
            // Lifted clear of the geometry it follows, otherwise the two are coplanar and z-fight
            surfaceTopography = std::make_unique<RivContourMapTopographyElevation>( mapTopography, m_depthOffset() + surfaceDrapeLift() );
        }

        const RivContourMapElevationProvider* surfaceElevation =
            surfaceTopography ? static_cast<const RivContourMapElevationProvider*>( surfaceTopography.get() ) : &flatElevation;

        partMgr()->appendProjectionToModel( model,
                                            displayCoordTransform,
                                            projection->trianglesWithVertexValues(),
                                            *projection->mapGrid(),
                                            sourceView->backgroundColor(),
                                            mapLegendConfig->scalarMapper(),
                                            surfaceElevation );
    }

    if ( !m_showContourLines() || !camera ) return;

    // The lines are drawn as line primitives, which OpenGL polygon offset does not apply to, so they
    // have to be lifted above the surface they follow to win the depth test against it.
    const double lineLift = contourLineLift();

    std::unique_ptr<RivContourMapTopographyElevation> lineTopography;
    if ( m_projectLinesOnGeometry() && mapTopography )
    {
        lineTopography = std::make_unique<RivContourMapTopographyElevation>( mapTopography, m_depthOffset() + lineLift );
    }

    RivContourMapFlatElevation flatLineElevation( *elevation + lineLift );

    const RivContourMapElevationProvider* lineElevation =
        lineTopography ? static_cast<const RivContourMapElevationProvider*>( lineTopography.get() ) : &flatLineElevation;

    RivContourLineAppearance lineAppearance;
    if ( m_lineColorMode() == LineColorMode::SINGLE_COLOR ) lineAppearance.color = m_lineColor();
    lineAppearance.lineWidth = static_cast<float>( m_lineThickness() );

    lineAppearance.labelFontSize = m_labelFontSize();

    // The camera moves freely here, unlike in the 2d contour map views, so labels are kept upright
    lineAppearance.alignLabelsWithCamera = true;

    // The labels are anchored in 3d and re-projected to the screen every frame, so they stay readable
    // and correctly placed while navigating. Only the layout decisions, which labels were dropped for
    // overlapping and how the text is rotated, are made from the camera as it is right now.
    partMgr()->appendContourLinesToModel( camera,
                                          model,
                                          displayCoordTransform,
                                          projection->contourPolygons(),
                                          *projection->mapGrid(),
                                          mapLegendConfig->scalarMapper(),
                                          true,
                                          m_showContourLabels(),
                                          mapLegendConfig->tickNumberFormat(),
                                          mapLegendConfig->significantDigitsInData(),
                                          lineElevation,
                                          lineAppearance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimContourMapInView::legendConfig() const
{
    auto projection = contourMapProjection();
    if ( !projection ) return nullptr;

    return projection->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInView::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer )
{
    if ( !isChecked() || !nativeOrOverrideViewer ) return;

    auto projection = contourMapProjection();
    if ( !projection || !projection->isChecked() ) return;

    // Sets both the ranges and the title
    projection->updateLegend();

    auto* mapLegendConfig = projection->legendConfig();
    if ( !mapLegendConfig || !mapLegendConfig->showLegend() ) return;

    nativeOrOverrideViewer->addColorLegendToBottomLeftCorner( mapLegendConfig->titledOverlayFrame(), isUsingOverrideViewer );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_nameProxy );

    caf::PdmUiGroup* positionGroup = uiOrdering.addNewGroup( "Position" );
    positionGroup->add( &m_mapPosition );
    if ( m_mapPosition() == MapPosition::USER_DEFINED_DEPTH )
    {
        positionGroup->add( &m_userDefinedDepth );
    }
    positionGroup->add( &m_depthOffset );

    caf::PdmUiGroup* surfaceGroup = uiOrdering.addNewGroup( "Map Surface" );
    surfaceGroup->add( &m_showMapSurface );
    surfaceGroup->add( &m_projectSurfaceOnGeometry );
    m_projectSurfaceOnGeometry.uiCapability()->setUiReadOnly( !m_showMapSurface() );

    caf::PdmUiGroup* contourLineGroup = uiOrdering.addNewGroup( "Contour Lines" );
    contourLineGroup->add( &m_showContourLines );
    contourLineGroup->add( &m_projectLinesOnGeometry );
    contourLineGroup->add( &m_lineColorMode );
    if ( m_lineColorMode() == LineColorMode::SINGLE_COLOR )
    {
        contourLineGroup->add( &m_lineColor );
    }
    contourLineGroup->add( &m_lineThickness );
    contourLineGroup->add( &m_showContourLabels );
    contourLineGroup->add( &m_labelFontSize );

    // The toggle itself stays editable, everything it controls does not
    const bool linesDisabled = !m_showContourLines();
    m_projectLinesOnGeometry.uiCapability()->setUiReadOnly( linesDisabled );
    m_lineColorMode.uiCapability()->setUiReadOnly( linesDisabled );
    m_lineColor.uiCapability()->setUiReadOnly( linesDisabled );
    m_lineThickness.uiCapability()->setUiReadOnly( linesDisabled );
    m_showContourLabels.uiCapability()->setUiReadOnly( linesDisabled );
    m_labelFontSize.uiCapability()->setUiReadOnly( linesDisabled || !m_showContourLabels() );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    updateUiIconFromToggleField();

    if ( changedField == &m_mapPosition || changedField == &m_projectSurfaceOnGeometry || changedField == &m_projectLinesOnGeometry ||
         changedField == &m_lineColorMode || changedField == &m_showContourLines || changedField == &m_showMapSurface ||
         changedField == &m_showContourLabels )
    {
        updateConnectedEditors();
    }

    if ( auto view = firstAncestorOfType<Rim3dView>() )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapInView::initAfterRead()
{
    updateUiIconFromToggleField();
}
