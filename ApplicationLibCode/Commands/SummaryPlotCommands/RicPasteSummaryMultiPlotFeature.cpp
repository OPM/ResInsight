/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RicPasteSummaryMultiPlotFeature.h"

#include "Summary/RiaSummaryTools.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicPasteSummaryMultiPlotFeature, "RicPasteSummaryMultiPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteSummaryMultiPlotFeature::isCommandEnabled() const
{
    if ( !caf::selectedObjectsByType<RimSummaryMultiPlotCollection*>().empty() ) return true;

    return !caf::selectedObjectsByTypeStrict<RimSummaryMultiPlot*>().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryMultiPlotFeature::onActionTriggered( bool isChecked )
{
    auto sourceObjects = RicPasteSummaryMultiPlotFeature::summaryMultiPlots();

    for ( const auto& sourceObject : sourceObjects )
    {
        RiaSummaryTools::summaryMultiPlotCollection()->duplicatePlot( sourceObject );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryMultiPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste Summary Plot" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimSummaryMultiPlot>> RicPasteSummaryMultiPlotFeature::summaryMultiPlots()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimSummaryMultiPlot>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    return typedObjects;
}
