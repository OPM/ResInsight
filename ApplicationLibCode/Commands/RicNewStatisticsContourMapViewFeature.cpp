/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RicNewStatisticsContourMapViewFeature.h"

#include "ContourMap/RimEclipseContourMapViewCollection.h"
#include "ContourMap/RimStatisticsContourMap.h"
#include "ContourMap/RimStatisticsContourMapView.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimFaultInViewCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellInViewCollection.h"
#include "RimSurfaceInViewCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuGuiTheme.h"

#include "RiaColorTools.h"

#include "cafPdmDocument.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewStatisticsContourMapViewFeature, "RicNewStatisticsContourMapViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewStatisticsContourMapViewFeature::isCommandEnabled() const
{
    auto contourMap = caf::SelectionManager::instance()->selectedItemOfType<RimStatisticsContourMap>();
    return contourMap != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsContourMapViewFeature::onActionTriggered( bool isChecked )
{
    auto contourMap = caf::SelectionManager::instance()->selectedItemOfType<RimStatisticsContourMap>();
    if ( !contourMap ) return;

    contourMap->ensureResultsComputed();
    auto view = createAndAddView( contourMap );
    contourMap->updateConnectedEditors();

    Riu3DMainWindowTools::selectAsCurrentItem( view );
    Riu3DMainWindowTools::setExpanded( contourMap );
    Riu3DMainWindowTools::setExpanded( view );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsContourMapViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create View" );
    actionToSetup->setIcon( QIcon( ":/2DMap16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsContourMapView* RicNewStatisticsContourMapViewFeature::createStatisticsContourMapView( RimStatisticsContourMap* statisticsContourMap )
{
    RimEclipseCase* eclipseCase = statisticsContourMap->eclipseCase();
    if ( !eclipseCase ) return nullptr;

    RimStatisticsContourMapView* contourMapView = new RimStatisticsContourMapView;
    contourMapView->setStatisticsContourMap( statisticsContourMap );
    contourMapView->setEclipseCase( eclipseCase );

    caf::PdmDocument::updateUiIconStateRecursively( contourMapView );

    contourMapView->faultCollection()->setActive( false );
    contourMapView->wellCollection()->isActive = false;

    statisticsContourMap->addView( contourMapView );

    auto col = RiuGuiTheme::getColorByVariableName( "backgroundColor2" );
    contourMapView->setBackgroundColor( RiaColorTools::fromQColorTo3f( col ) ); // Ignore original view background
    contourMapView->initAfterReadRecursively();

    return contourMapView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsContourMapView* RicNewStatisticsContourMapViewFeature::createAndAddView( RimStatisticsContourMap* statisticsContourMap )
{
    if ( auto contourMapView = createStatisticsContourMapView( statisticsContourMap ) )
    {
        // Must be run before buildViewItems, as wells are created in this function
        contourMapView->loadDataAndUpdate();

        // make sure no surfaces are shown in the view when the contourmap is generated
        if ( contourMapView->surfaceInViewCollection() ) contourMapView->surfaceInViewCollection()->setCheckState( Qt::Unchecked );

        if ( auto eclipseCase = statisticsContourMap->eclipseCase() )
        {
            eclipseCase->updateConnectedEditors();
            contourMapView->cellFilterCollection()->setCase( eclipseCase );
        }

        contourMapView->createDisplayModelAndRedraw();
        contourMapView->zoomAll();

        statisticsContourMap->updateConnectedEditors();

        return contourMapView;
    }

    return nullptr;
}
