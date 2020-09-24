/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicCloseObservedDataFeature.h"

#include "RiaSummaryTools.h"

#include "RimMainPlotCollection.h"
#include "RimObservedDataCollection.h"
#include "RimObservedFmuRftData.h"
#include "RimObservedSummaryData.h"
#include "RimProject.h"
#include "RimRftPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimWellRftPlot.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCloseObservedDataFeature, "RicCloseObservedDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseObservedDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Close" );
    actionToSetup->setIcon( QIcon( ":/Erase.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseObservedDataFeature::deleteObservedSummaryData( const std::vector<RimObservedSummaryData*>& data )
{
    RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();

    for ( RimObservedSummaryData* observedData : data )
    {
        for ( RimSummaryPlot* summaryPlot : summaryPlotColl->plots() )
        {
            summaryPlot->deleteCurvesAssosiatedWithCase( observedData );
        }
        summaryPlotColl->updateConnectedEditors();

        RimObservedDataCollection* observedDataCollection = nullptr;
        observedData->firstAncestorOrThisOfTypeAsserted( observedDataCollection );

        observedDataCollection->removeObservedSummaryData( observedData );
        delete observedData;
        observedDataCollection->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseObservedDataFeature::deleteObservedRmuRftData( const std::vector<RimObservedFmuRftData*>& data )
{
    RimProject*           proj        = RimProject::current();
    RimRftPlotCollection* rftPlotColl = proj->mainPlotCollection()->rftPlotCollection();

    for ( RimObservedFmuRftData* observedData : data )
    {
        RimObservedDataCollection* observedDataCollection = nullptr;
        observedData->firstAncestorOrThisOfTypeAsserted( observedDataCollection );

        for ( RimWellRftPlot* plot : rftPlotColl->rftPlots() )
        {
            plot->deleteCurvesAssosicatedWithObservedData( observedData );
        }
        observedDataCollection->removeObservedFmuRftData( observedData );
        delete observedData;
        observedDataCollection->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCloseObservedDataFeature::isCommandEnabled()
{
    std::vector<RimObservedSummaryData*> summarySelection;
    caf::SelectionManager::instance()->objectsByType( &summarySelection );

    std::vector<RimObservedFmuRftData*> fmuRftSelection;
    caf::SelectionManager::instance()->objectsByType( &fmuRftSelection );

    if ( summarySelection.size() == 0 && fmuRftSelection.size() == 0 )
    {
        return false;
    }
    for ( RimObservedSummaryData* data : summarySelection )
    {
        if ( !data->isObservedData() )
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseObservedDataFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimObservedSummaryData*> summarySelection;
    caf::SelectionManager::instance()->objectsByType( &summarySelection );

    std::vector<RimObservedFmuRftData*> fmuRftSelection;
    caf::SelectionManager::instance()->objectsByType( &fmuRftSelection );

    CVF_ASSERT( !( summarySelection.empty() && fmuRftSelection.empty() ) );

    RicCloseObservedDataFeature::deleteObservedSummaryData( summarySelection );
    RicCloseObservedDataFeature::deleteObservedRmuRftData( fmuRftSelection );
}
