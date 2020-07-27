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

#include "Rim3dView.h"
#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseContourMapViewCollection.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechContourMapViewCollection.h"
#include "RimGeoMechView.h"
#include "RimRegularLegendConfig.h"

#include "RimFaultInViewCollection.h"
#include "RimSimWellInViewCollection.h"
#include "RimSurfaceInViewCollection.h"

#include "Riu3DMainWindowTools.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "cafPdmDocument.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewContourMapViewFeature, "RicNewContourMapViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewContourMapViewFeature::isCommandEnabled()
{
    bool selectedView = caf::SelectionManager::instance()->selectedItemOfType<RimGridView>() != nullptr;
    bool selectedCase = caf::SelectionManager::instance()->selectedItemOfType<RimCase>() != nullptr;
    bool selectedEclipseContourMapCollection =
        caf::SelectionManager::instance()->selectedItemOfType<RimEclipseContourMapViewCollection>();
    bool selectedGeoMechContourMapCollection =
        caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechContourMapViewCollection>();
    return selectedView || selectedCase || selectedEclipseContourMapCollection || selectedGeoMechContourMapCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewContourMapViewFeature::onActionTriggered( bool isChecked )
{
    RimEclipseView*           reservoirView = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>();
    RimEclipseContourMapView* existingEclipseContourMap =
        caf::SelectionManager::instance()->selectedItemOfType<RimEclipseContourMapView>();
    RimEclipseCase* eclipseCase = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimEclipseCase>();
    RimEclipseContourMapView* eclipseContourMap = nullptr;

    RimGeoMechView*           geoMechView = caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechView>();
    RimGeoMechContourMapView* existingGeoMechContourMap =
        caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechContourMapView>();
    RimGeoMechCase* geoMechCase = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimGeoMechCase>();
    RimGeoMechContourMapView* geoMechContourMap = nullptr;

    // Find case to insert into
    if ( existingEclipseContourMap )
    {
        eclipseContourMap = createEclipseContourMapFromExistingContourMap( eclipseCase, existingEclipseContourMap );
    }
    else if ( reservoirView )
    {
        eclipseContourMap = createEclipseContourMapFrom3dView( eclipseCase, reservoirView );
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

        if ( eclipseCase )
        {
            eclipseCase->updateConnectedEditors();
        }
        caf::SelectionManager::instance()->setSelectedItem( eclipseContourMap );

        eclipseContourMap->createDisplayModelAndRedraw();
        eclipseContourMap->zoomAll();

        Riu3DMainWindowTools::setExpanded( eclipseContourMap );
    }
    else if ( geoMechContourMap )
    {
        geoMechContourMap->loadDataAndUpdate();
        if ( geoMechCase )
        {
            geoMechCase->updateConnectedEditors();
            caf::SelectionManager::instance()->setSelectedItem( geoMechContourMap );
            geoMechContourMap->createDisplayModelAndRedraw();
            geoMechContourMap->zoomAll();
            Riu3DMainWindowTools::setExpanded( geoMechContourMap );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewContourMapViewFeature::setupActionLook( QAction* actionToSetup )
{
    bool contourMapSelected =
        caf::SelectionManager::instance()->selectedItemOfType<RimEclipseContourMapView>() != nullptr ||
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
    RimEclipseContourMapView* contourMap = dynamic_cast<RimEclipseContourMapView*>(
        existingContourMap->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( contourMap );

    contourMap->setEclipseCase( eclipseCase );
    contourMap->setBackgroundColor( cvf::Color3f( 1.0f, 1.0f, 0.98f ) ); // Ignore original view background

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    size_t i = eclipseCase->contourMapCollection()->views().size();
    contourMap->setName( QString( "Contour Map %1" ).arg( i + 1 ) );
    eclipseCase->contourMapCollection()->push_back( contourMap );

    // Resolve references after contour map has been inserted into Rim structures
    contourMap->resolveReferencesRecursively();
    contourMap->initAfterReadRecursively();

    return contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapView* RicNewContourMapViewFeature::createEclipseContourMapFrom3dView( RimEclipseCase* eclipseCase,
                                                                                          const RimEclipseView* sourceView )
{
    RimEclipseContourMapView* contourMap = dynamic_cast<RimEclipseContourMapView*>(
        sourceView->xmlCapability()->copyAndCastByXmlSerialization( RimEclipseContourMapView::classKeywordStatic(),
                                                                    sourceView->classKeyword(),
                                                                    caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( contourMap );

    contourMap->setEclipseCase( eclipseCase );
    contourMap->setBackgroundColor( cvf::Color3f( 1.0f, 1.0f, 0.98f ) ); // Ignore original view background
    contourMap->setDefaultCustomName();
    contourMap->faultCollection()->showFaultCollection = false;
    contourMap->wellCollection()->isActive             = false;

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    eclipseCase->contourMapCollection()->push_back( contourMap );

    contourMap->syncronizeLocalAnnotationsFromGlobal();

    // Resolve references after contour map has been inserted into Rim structures
    std::vector<caf::PdmFieldHandle*> fieldsWithFailingResolve;
    contourMap->resolveReferencesRecursively( &fieldsWithFailingResolve );

    // TODO: Introduce the assert when code is stable
    // If we have intersections on well paths, the resolving is now failing
    // CVF_ASSERT(fieldsWithFailingResolve.empty());

    contourMap->initAfterReadRecursively();

    return contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapView* RicNewContourMapViewFeature::createEclipseContourMap( RimEclipseCase* eclipseCase )
{
    RimEclipseContourMapView* contourMap = new RimEclipseContourMapView();
    contourMap->setEclipseCase( eclipseCase );

    // Set default values
    {
        contourMap->cellResult()->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );

        if ( RiaApplication::instance()->preferences()->loadAndShowSoil )
        {
            contourMap->cellResult()->setResultVariable( "SOIL" );
        }

        RimRegularLegendConfig* legendConfig = contourMap->cellResult()->legendConfig();
        if ( legendConfig )
        {
            RimColorLegend* legend = legendConfig->mapToColorLegend( RimRegularLegendConfig::RAINBOW );
            legendConfig->setColorLegend( legend );
        }
    }

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    size_t i = eclipseCase->contourMapCollection()->views().size();
    contourMap->setName( QString( "Contour Map %1" ).arg( i + 1 ) );
    eclipseCase->contourMapCollection()->push_back( contourMap );

    contourMap->hasUserRequestedAnimation = true;
    contourMap->setBackgroundColor( cvf::Color3f( 1.0f, 1.0f, 0.98f ) );
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
    RimGeoMechContourMapView* contourMap = dynamic_cast<RimGeoMechContourMapView*>(
        existingContourMap->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( contourMap );

    contourMap->setGeoMechCase( geoMechCase );
    contourMap->setBackgroundColor( cvf::Color3f( 1.0f, 1.0f, 0.98f ) ); // Ignore original view background

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    size_t i = geoMechCase->contourMapCollection()->views().size();
    contourMap->setName( QString( "Contour Map %1" ).arg( i + 1 ) );
    geoMechCase->contourMapCollection()->push_back( contourMap );

    // Resolve references after contour map has been inserted into Rim structures
    contourMap->resolveReferencesRecursively();
    contourMap->initAfterReadRecursively();

    return contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapView* RicNewContourMapViewFeature::createGeoMechContourMapFrom3dView( RimGeoMechCase* geoMechCase,
                                                                                          const RimGeoMechView* sourceView )
{
    RimGeoMechContourMapView* contourMap = dynamic_cast<RimGeoMechContourMapView*>(
        sourceView->xmlCapability()->copyAndCastByXmlSerialization( RimGeoMechContourMapView::classKeywordStatic(),
                                                                    sourceView->classKeyword(),
                                                                    caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( contourMap );

    contourMap->setGeoMechCase( geoMechCase );
    contourMap->setBackgroundColor( cvf::Color3f( 1.0f, 1.0f, 0.98f ) ); // Ignore original view background
    contourMap->setDefaultCustomName();

    // make sure no surfaces are shown in the view when the contourmap is generated
    if ( contourMap->surfaceInViewCollection() ) contourMap->surfaceInViewCollection()->setCheckState( Qt::Unchecked );

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    geoMechCase->contourMapCollection()->push_back( contourMap );

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
    geoMechCase->contourMapCollection()->push_back( contourMap );

    contourMap->hasUserRequestedAnimation = true;
    contourMap->setBackgroundColor( cvf::Color3f( 1.0f, 1.0f, 0.98f ) );
    contourMap->initAfterReadRecursively();

    return contourMap;
}
