/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RicPasteGridCrossPlotDataSetFeature.h"

#include "RiaGuiApplication.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotDataSet.h"
#include "RiuPlotMainWindowTools.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicPasteGridCrossPlotDataSetFeature, "RicPasteGridCrossPlotDataSetFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteGridCrossPlotDataSetFeature::isCommandEnabled()
{
    auto curvesOnClipboard = gridCrossPlotDataSetsOnClipboard();
    if ( curvesOnClipboard.empty() ) return false;

    return caf::SelectionManager::instance()->selectedItemAncestorOfType<RimGridCrossPlot>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteGridCrossPlotDataSetFeature::onActionTriggered( bool isChecked )
{
    RimGridCrossPlot* crossPlot = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimGridCrossPlot>();

    if ( crossPlot )
    {
        auto curvesOnClipboard = gridCrossPlotDataSetsOnClipboard();
        if ( !curvesOnClipboard.empty() )
        {
            RimGridCrossPlotDataSet* objectToSelect = nullptr;

            for ( RimGridCrossPlotDataSet* dataSet : gridCrossPlotDataSetsOnClipboard() )
            {
                RimGridCrossPlotDataSet* newDataSet = dynamic_cast<RimGridCrossPlotDataSet*>(
                    dataSet->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

                crossPlot->addDataSet( newDataSet );
                newDataSet->resolveReferencesRecursively();
                newDataSet->initAfterReadRecursively();

                objectToSelect = newDataSet;
            }

            RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
            crossPlot->updateAllRequiredEditors();
            crossPlot->loadDataAndUpdate();

            RiuPlotMainWindowTools::selectAsCurrentItem( objectToSelect );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteGridCrossPlotDataSetFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste Data Set" );
    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimGridCrossPlotDataSet>> RicPasteGridCrossPlotDataSetFeature::gridCrossPlotDataSetsOnClipboard()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimGridCrossPlotDataSet>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    return typedObjects;
}
