/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicNewContourMapViewFeature.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigFemPartCollection.h"

#include "ContourMap/RimEclipseContourMapProjection.h"
#include "ContourMap/RimEclipseContourMapView.h"
#include "ContourMap/RimEclipseContourMapViewCollection.h"
#include "ContourMap/RimStatisticsContourMapView.h"
#include "Polygons/RimPolygonInView.h"
#include "Rim3dView.h"
#include "RimCellEdgeColors.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechContourMapViewCollection.h"
#include "RimGeoMechView.h"
#include "RimOilField.h"
#include "RimPolygonFilter.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInViewCollection.h"
#include "RimSurfaceInViewCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuGuiTheme.h"

#include "RiaColorTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesGrid.h"

#include "cafPdmDocument.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewContourMapViewFeature, "RicNewContourMapViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewContourMapViewFeature::isCommandEnabled() const
{
    if ( caf::SelectionManager::instance()->selectedItemOfType<RimStatisticsContourMapView>() != nullptr ) return false;

    bool selectedView = caf::SelectionManager::instance()->selectedItemOfType<RimGridView>() != nullptr;
    bool selectedCase = caf::SelectionManager::instance()->selectedItemOfType<RimCase>() != nullptr;

    RimGeoMechView* gmView = caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechView>();
    if ( gmView && gmView->femParts() )
    {
        // if we have more than one geomech part, contour maps does not work with the current implementation
        if ( gmView->femParts()->partCount() > 1 ) return false;
    }

    bool selectedEclipseContourMapCollection = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseContourMapViewCollection>();
    bool selectedGeoMechContourMapCollection = caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechContourMapViewCollection>();
    return selectedView || selectedCase || selectedEclipseContourMapCollection || selectedGeoMechContourMapCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewContourMapViewFeature::onActionTriggered( bool isChecked )
{
    RimEclipseView*           reservoirView             = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>();
    RimEclipseContourMapView* existingEclipseContourMap = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseContourMapView>();
    RimEclipseCase*           eclipseCase               = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseCase>();
    RimEclipseContourMapView* eclipseContourMap         = nullptr;

    RimGeoMechView*           geoMechView               = caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechView>();
    RimGeoMechContourMapView* existingGeoMechContourMap = caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechContourMapView>();
    RimGeoMechCase*           geoMechCase               = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimGeoMechCase>();
    RimGeoMechContourMapView* geoMechContourMap         = nullptr;

    // Find case to insert into
    if ( existingEclipseContourMap )
    {
        eclipseCase = existingEclipseContourMap->eclipseCase();
        if ( eclipseCase )
        {
            eclipseContourMap = createEclipseContourMapFromExistingContourMap( eclipseCase, existingEclipseContourMap );
        }
    }
    else if ( reservoirView )
    {
        eclipseCase = reservoirView->eclipseCase();
        if ( eclipseCase )
        {
            eclipseContourMap = createEclipseContourMapFrom3dView( eclipseCase, reservoirView );
        }
    }
    else if ( eclipseCase )
    {
        eclipseContourMap = createEclipseContourMap( eclipseCase );
    }
    else if ( existingGeoMechContourMap )
    {
        geoMechContourMap = createGeoMechContourMapFromExistingContourMap( geoMechCase, existingGeoMechContourMap );
    }
    else if ( geoMechView )
    {
        geoMechContourMap = createGeoMechContourMapFrom3dView( geoMechCase, geoMechView );
    }
    else if ( geoMechCase )
    {
        geoMechContourMap = createGeoMechContourMap( geoMechCase );
    }

    if ( eclipseContourMap )
    {
        // Must be run before buildViewItems, as wells are created in this function
        eclipseContourMap->loadDataAndUpdate();

        // make sure no surfaces are shown in the view when the contourmap is generated
        if ( eclipseContourMap->surfaceInViewCollection() ) eclipseContourMap->surfaceInViewCollection()->setCheckState( Qt::Unchecked );

        if ( eclipseCase )
        {
            eclipseCase->updateConnectedEditors();
            eclipseContourMap->cellFilterCollection()->setCase( eclipseCase );
        }
        caf::SelectionManager::instance()->setSelectedItem( eclipseContourMap );

        eclipseContourMap->createDisplayModelAndRedraw();
        eclipseContourMap->zoomAll();

        RimProject* project = RimProject::current();

        RimOilField* oilField = project->activeOilField();

        oilField->eclipseContourMapCollection()->updateConnectedEditors();

        Riu3DMainWindowTools::setExpanded( eclipseContourMap );
        Riu3DMainWindowTools::selectAsCurrentItem( eclipseContourMap );
    }
    else if ( geoMechContourMap )
    {
        geoMechContourMap->loadDataAndUpdate();
        // make sure no surfaces are shown in the view when the contourmap is generated
        if ( geoMechContourMap->surfaceInViewCollection() ) geoMechContourMap->surfaceInViewCollection()->setCheckState( Qt::Unchecked );

        if ( geoMechCase )
        {
            geoMechCase->updateConnectedEditors();
            geoMechContourMap->cellFilterCollection()->setCase( geoMechCase );
            caf::SelectionManager::instance()->setSelectedItem( geoMechContourMap );
            geoMechContourMap->createDisplayModelAndRedraw();
            geoMechContourMap->zoomAll();
            Riu3DMainWindowTools::setExpanded( geoMechContourMap );
            Riu3DMainWindowTools::selectAsCurrentItem( geoMechContourMap );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewContourMapViewFeature::setupActionLook( QAction* actionToSetup )
{
    bool contourMapSelected = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseContourMapView>() != nullptr ||
                              caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechContourMapView>() != nullptr;

    bool viewSelected = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>() != nullptr ||
                        caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechView>() != nullptr;

    if ( contourMapSelected )
    {
        actionToSetup->setText( "Duplicate Contour Map" );
    }
    else if ( viewSelected )
    {
        actionToSetup->setText( "New Contour Map From 3d View" );
    }
    else
    {
        actionToSetup->setText( "New Contour Map" );
    }
    actionToSetup->setIcon( QIcon( ":/2DMap16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapView*
    RicNewContourMapViewFeature::createEclipseContourMapFromExistingContourMap( RimEclipseCase*           eclipseCase,
                                                                                RimEclipseContourMapView* existingContourMap )
{
    auto contourMap = existingContourMap->copyObject<RimEclipseContourMapView>();
    CVF_ASSERT( contourMap );
    contourMap->setEclipseCase( eclipseCase );

    auto col = RiuGuiTheme::getColorByVariableName( "backgroundColor2" );
    contourMap->setBackgroundColor( RiaColorTools::fromQColorTo3f( col ) ); // Ignore original view background

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    size_t i = eclipseCase->contourMapCollection()->views().size();
    contourMap->setName( QString( "Contour Map %1" ).arg( i + 1 ) );
    eclipseCase->contourMapCollection()->addView( contourMap );

    // Resolve references after contour map has been inserted into Rim structures
    contourMap->resolveReferencesRecursively();
    contourMap->initAfterReadRecursively();

    return contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapView* RicNewContourMapViewFeature::createEclipseContourMapFrom3dView( RimEclipseCase*       eclipseCase,
                                                                                          const RimEclipseView* sourceView )
{
    RimEclipseContourMapView* contourMap = dynamic_cast<RimEclipseContourMapView*>(
        sourceView->xmlCapability()->copyAndCastByXmlSerialization( RimEclipseContourMapView::classKeywordStatic(),
                                                                    sourceView->classKeyword(),
                                                                    caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( contourMap );

    contourMap->setEclipseCase( eclipseCase );

    auto col = RiuGuiTheme::getColorByVariableName( "backgroundColor2" );
    contourMap->setBackgroundColor( RiaColorTools::fromQColorTo3f( col ) ); // Ignore original view background

    contourMap->setDefaultCustomName();
    contourMap->faultCollection()->setActive( false );
    contourMap->wellCollection()->isActive = false;

    contourMap->setCompatibleDrawStyle();

    // Set default values
    RimRegularLegendConfig* legendConfig = contourMap->cellResult()->legendConfig();
    if ( legendConfig && legendConfig->mappingMode() == RimRegularLegendConfig::MappingType::CATEGORY_INTEGER )
    {
        RicNewContourMapViewFeature::assignDefaultResultAndLegend( contourMap );
    }

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    eclipseCase->contourMapCollection()->addView( contourMap );

    contourMap->synchronizeLocalAnnotationsFromGlobal();

    contourMap->initAfterReadRecursively();

    eclipseCase->contourMapCollection()->updateConnectedEditors();

    // Polygon cell filters are not working due to different organization of the project tree for Eclipse view and contour map view
    // Manually assign pointers to polygon objects
    //
    // TODO: Find a more robust solution, perhaps introduce a root token making it possible to have a reference string pointing to objects
    // relative root "$root$ PolygonCollection 0 Polygon 2"
    // Make sure this concept works for geomech contour maps

    auto sourceFilters      = sourceView->cellFilterCollection()->filters();
    auto destinationFilters = contourMap->cellFilterCollection()->filters();
    if ( sourceFilters.size() == destinationFilters.size() )
    {
        for ( size_t i = 0; i < sourceFilters.size(); ++i )
        {
            auto sourcePolygonFilter      = dynamic_cast<RimPolygonFilter*>( sourceFilters[i] );
            auto destinationPolygonFilter = dynamic_cast<RimPolygonFilter*>( destinationFilters[i] );

            if ( sourcePolygonFilter && sourcePolygonFilter->polygonInView() && destinationPolygonFilter )
            {
                destinationPolygonFilter->setPolygon( sourcePolygonFilter->polygonInView()->polygon() );
                destinationPolygonFilter->configurePolygonEditor();
            }
        }
    }

    return contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapView* RicNewContourMapViewFeature::createEclipseContourMap( RimEclipseCase* eclipseCase )
{
    // Make sure case is open before accessing active cell info
    eclipseCase->ensureReservoirCaseIsOpen();
    if ( !eclipseCase->eclipseCaseData() ) return nullptr;

    RimEclipseContourMapView* contourMap = new RimEclipseContourMapView();
    contourMap->setEclipseCase( eclipseCase );

    assignDefaultResultAndLegend( contourMap );

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    size_t i = eclipseCase->contourMapCollection()->views().size();
    contourMap->setName( QString( "Contour Map %1" ).arg( i + 1 ) );

    contourMap->faultCollection()->setActive( false );
    contourMap->wellCollection()->isActive = false;

    eclipseCase->contourMapCollection()->addView( contourMap );

    auto col = RiuGuiTheme::getColorByVariableName( "backgroundColor2" );
    contourMap->setBackgroundColor( RiaColorTools::fromQColorTo3f( col ) ); // Ignore original view background

    contourMap->initAfterReadRecursively();

    return contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapView*
    RicNewContourMapViewFeature::createGeoMechContourMapFromExistingContourMap( RimGeoMechCase*           geoMechCase,
                                                                                RimGeoMechContourMapView* existingContourMap )
{
    auto contourMap = existingContourMap->copyObject<RimGeoMechContourMapView>();
    CVF_ASSERT( contourMap );
    contourMap->setGeoMechCase( geoMechCase );

    auto col = RiuGuiTheme::getColorByVariableName( "backgroundColor2" );
    contourMap->setBackgroundColor( RiaColorTools::fromQColorTo3f( col ) ); // Ignore original view background

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    size_t i = geoMechCase->contourMapCollection()->views().size();
    contourMap->setName( QString( "Contour Map %1" ).arg( i + 1 ) );
    geoMechCase->contourMapCollection()->addView( contourMap );

    // Resolve references after contour map has been inserted into Rim structures
    contourMap->resolveReferencesRecursively();
    contourMap->initAfterReadRecursively();

    return contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapView* RicNewContourMapViewFeature::createGeoMechContourMapFrom3dView( RimGeoMechCase*       geoMechCase,
                                                                                          const RimGeoMechView* sourceView )
{
    RimGeoMechContourMapView* contourMap = dynamic_cast<RimGeoMechContourMapView*>(
        sourceView->xmlCapability()->copyAndCastByXmlSerialization( RimGeoMechContourMapView::classKeywordStatic(),
                                                                    sourceView->classKeyword(),
                                                                    caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( contourMap );

    contourMap->setGeoMechCase( geoMechCase );

    auto col = RiuGuiTheme::getColorByVariableName( "backgroundColor2" );
    contourMap->setBackgroundColor( RiaColorTools::fromQColorTo3f( col ) ); // Ignore original view background

    contourMap->setDefaultCustomName();
    contourMap->setCompatibleDrawStyle();

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    geoMechCase->contourMapCollection()->addView( contourMap );

    // Resolve references after contour map has been inserted into Rim structures
    std::vector<caf::PdmFieldHandle*> fieldsWithFailingResolve;
    contourMap->resolveReferencesRecursively( &fieldsWithFailingResolve );
    contourMap->initAfterReadRecursively();

    return contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapView* RicNewContourMapViewFeature::createGeoMechContourMap( RimGeoMechCase* geoMechCase )
{
    RimGeoMechContourMapView* contourMap = new RimGeoMechContourMapView();
    contourMap->setGeoMechCase( geoMechCase );

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    size_t i = geoMechCase->contourMapCollection()->views().size();
    contourMap->setName( QString( "Contour Map %1" ).arg( i + 1 ) );
    geoMechCase->contourMapCollection()->addView( contourMap );

    auto col = RiuGuiTheme::getColorByVariableName( "backgroundColor2" );
    contourMap->setBackgroundColor( RiaColorTools::fromQColorTo3f( col ) ); // Ignore original view background

    contourMap->initAfterReadRecursively();

    return contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewContourMapViewFeature::assignDefaultResultAndLegend( RimEclipseContourMapView* contourMap )
{
    if ( contourMap->cellResult() )
    {
        contourMap->cellResult()->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );

        if ( RiaPreferencesGrid::current()->loadAndShowSoil() )
        {
            contourMap->cellResult()->setResultVariable( "SOIL" );
        }
    }
}
