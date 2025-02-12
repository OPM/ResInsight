/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicPasteSummaryPlotFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"
#include "Summary/RiaSummaryPlotTools.h"

#include "RimMultiPlot.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicPasteSummaryPlotFeature, "RicPasteSummaryPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryPlotFeature::copyPlotAndAddToCollection( RimSummaryPlot* sourcePlot )
{
    auto multiPlot = caf::firstAncestorOfTypeFromSelectedObject<RimMultiPlot>();
    if ( multiPlot )
    {
        auto plots = RiaSummaryPlotTools::duplicatePlots( { sourcePlot } );
        RiaSummaryPlotTools::appendPlotsToMultiPlot( multiPlot, plots );

        multiPlot->loadDataAndUpdate();

        // No main window has focus after paste operation, set focus to main plot window
        RiuPlotMainWindowTools::showPlotMainWindow();

        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteSummaryPlotFeature::isCommandEnabled() const
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );

    if ( !destinationObject ) return false;

    auto multiPlot = caf::firstAncestorOfTypeFromSelectedObject<RimMultiPlot>();
    if ( !multiPlot ) return false;

    return !RicPasteSummaryPlotFeature::summaryPlots().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryPlotFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmPointer<RimSummaryPlot>> sourceObjects = RicPasteSummaryPlotFeature::summaryPlots();

    for ( size_t i = 0; i < sourceObjects.size(); i++ )
    {
        copyPlotAndAddToCollection( sourceObjects[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste Summary Plot" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimSummaryPlot>> RicPasteSummaryPlotFeature::summaryPlots()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimSummaryPlot>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    return typedObjects;
}
