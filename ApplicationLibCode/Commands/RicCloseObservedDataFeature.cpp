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

#include "Summary/RiaSummaryTools.h"

#include "RimMainPlotCollection.h"
#include "RimObservedDataCollection.h"
#include "RimObservedFmuRftData.h"
#include "RimObservedSummaryData.h"
#include "RimRftPlotCollection.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
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
    actionToSetup->setIcon( QIcon( ":/Close.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseObservedDataFeature::deleteObservedSummaryData( const std::vector<RimObservedSummaryData*>& data )
{
    RimSummaryMultiPlotCollection* summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection();

    for ( RimObservedSummaryData* observedData : data )
    {
        for ( RimSummaryMultiPlot* multiPlot : summaryPlotColl->multiPlots() )
        {
            for ( RimSummaryPlot* summaryPlot : multiPlot->summaryPlots() )
            {
                summaryPlot->deleteCurvesAssosiatedWithCase( observedData );
            }
            multiPlot->updateConnectedEditors();
        }

        RimObservedDataCollection* observedDataCollection = observedData->firstAncestorOrThisOfTypeAsserted<RimObservedDataCollection>();

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
    RimRftPlotCollection* rftPlotColl = RimMainPlotCollection::current()->rftPlotCollection();

    for ( RimObservedFmuRftData* observedData : data )
    {
        RimObservedDataCollection* observedDataCollection = observedData->firstAncestorOrThisOfTypeAsserted<RimObservedDataCollection>();

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
bool RicCloseObservedDataFeature::isCommandEnabled() const
{
    const auto summarySelection = caf::SelectionManager::instance()->objectsByType<RimObservedSummaryData>();
    const auto fmuRftSelection  = caf::SelectionManager::instance()->objectsByType<RimObservedFmuRftData>();

    if ( summarySelection.empty() && fmuRftSelection.empty() )
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
    const auto summarySelection = caf::SelectionManager::instance()->objectsByType<RimObservedSummaryData>();
    const auto fmuRftSelection  = caf::SelectionManager::instance()->objectsByType<RimObservedFmuRftData>();

    CVF_ASSERT( !( summarySelection.empty() && fmuRftSelection.empty() ) );

    RicCloseObservedDataFeature::deleteObservedSummaryData( summarySelection );
    RicCloseObservedDataFeature::deleteObservedRmuRftData( fmuRftSelection );
}
