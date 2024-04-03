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

#include "Rim3dView.h"
#include "RimCellEdgeColors.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseContourMapViewCollection.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechContourMapViewCollection.h"
#include "RimGeoMechView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInViewCollection.h"
#include "RimSurfaceInViewCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuGuiTheme.h"

#include "RiaColorTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "cafPdmDocument.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewContourMapViewFeature, "RicNewContourMapViewFeature" );

const size_t mediumSamplingThresholdCellCount = 500000u;
const size_t largeSamplingThresholdCellCount  = 5000000u;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewContourMapViewFeature::isCommandEnabled() const
{
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
    RimEclipseContourMapView* contourMap = dynamic_cast<RimEclipseContourMapView*>(
        existingContourMap->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( contourMap );

    contourMap->setEclipseCase( eclipseCase );

    auto col = RiuGuiTheme::getColorByVariableName( "backgroundColor2" );
    contourMap->setBackgroundColor( RiaColorTools::fromQColorTo3f( col ) ); // Ignore original view background

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
RimEclipseContourMapView* RicNewContourMapViewFeature::createEclipseContourMapFrom3dView( RimEclipseCase*       eclipseCase,
                                                                                          const RimEclipseView* sourceView )
{
    RimEclipseContourMapView* contourMap = dynamic_cast<RimEclipseContourMapView*>(
        sourceView->xmlCapability()->copyAndCastByXmlSerialization( RimEclipseContourMapView::classKeywordStatic(),
                                                                    sourceView->classKeyword(),
                                                                    caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( contourMap );

    const RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    size_t                   activeCellCount = activeCellInfo->reservoirActiveCellCount();
    if ( activeCellCount >= largeSamplingThresholdCellCount )
    {
        contourMap->contourMapProjection()->setSampleSpacingFactor( 1.5 );
    }
    else if ( activeCellCount >= mediumSamplingThresholdCellCount )
    {
        contourMap->contourMapProjection()->setSampleSpacingFactor( 1.2 );
    }

    contourMap->setEclipseCase( eclipseCase );

    auto col = RiuGuiTheme::getColorByVariableName( "backgroundColor2" );
    contourMap->setBackgroundColor( RiaColorTools::fromQColorTo3f( col ) ); // Ignore original view background

    contourMap->setDefaultCustomName();
    contourMap->faultCollection()->setActive( false );
    contourMap->wellCollection()->isActive = false;

    // Set default values
    RimRegularLegendConfig* legendConfig = contourMap->cellResult()->legendConfig();
    if ( legendConfig && legendConfig->mappingMode() == RimRegularLegendConfig::MappingType::CATEGORY_INTEGER )
    {
        RicNewContourMapViewFeature::assignDefaultResultAndLegend( contourMap );
    }

    caf::PdmDocument::updateUiIconStateRecursively( contourMap );

    eclipseCase->contourMapCollection()->push_back( contourMap );

    contourMap->synchronizeLocalAnnotationsFromGlobal();

    // Resolve references after contour map has been inserted into Rim structures
    std::vector<caf::PdmFieldHandle*> fieldsWithFailingResolve;
    contourMap->resolveReferencesRecursively( &fieldsWithFailingResolve );

    // TODO: Introduce the assert when code is stable
    // If we have intersections on well paths, the resolving is now failing
    // CVF_ASSERT(fieldsWithFailingResolve.empty());

    contourMap->initAfterReadRecursively();

    eclipseCase->contourMapCollection()->updateConnectedEditors();

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

    const RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    size_t                   activeCellCount = activeCellInfo->reservoirActiveCellCount();
    if ( activeCellCount >= largeSamplingThresholdCellCount )
    {
        contourMap->contourMapProjection()->setSampleSpacingFactor( 1.5 );
    }
    else if ( activeCellCount >= mediumSamplingThresholdCellCount )
    {
        contourMap->contourMapProjection()->setSampleSpacingFactor( 1.2 );
    }

    contourMap->faultCollection()->setActive( false );
    contourMap->wellCollection()->isActive = false;

    eclipseCase->contourMapCollection()->push_back( contourMap );

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
    RimGeoMechContourMapView* contourMap = dynamic_cast<RimGeoMechContourMapView*>(
        existingContourMap->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( contourMap );

    contourMap->setGeoMechCase( geoMechCase );

    auto col = RiuGuiTheme::getColorByVariableName( "backgroundColor2" );
    contourMap->setBackgroundColor( RiaColorTools::fromQColorTo3f( col ) ); // Ignore original view background

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

        if ( RiaPreferences::current()->loadAndShowSoil )
        {
            contourMap->cellResult()->setResultVariable( "SOIL" );
        }
    }
}
