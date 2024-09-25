/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RicNewCustomVfpPlotFeature.h"

#include "RimMainPlotCollection.h"

#include "VerticalFlowPerformance/RimCustomVfpPlot.h"
#include "VerticalFlowPerformance/RimVfpDataCollection.h"
#include "VerticalFlowPerformance/RimVfpPlotCollection.h"
#include "VerticalFlowPerformance/RimVfpTable.h"
#include "VerticalFlowPerformance/RimVfpTableData.h"

#include "cafSelectionManagerTools.h"

#include "RiuPlotMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewCustomVfpPlotFeature, "RicNewCustomVfpPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCustomVfpPlotFeature::onActionTriggered( bool isChecked )
{
    RimVfpPlotCollection* vfpPlotColl = RimMainPlotCollection::current()->vfpPlotCollection();
    if ( !vfpPlotColl ) return;

    auto selectedTables = RicNewCustomVfpPlotFeature::selectedTables();
    if ( selectedTables.empty() ) return;

    auto                      mainDataSource = selectedTables.front();
    std::vector<RimVfpTable*> additionalDataSources;
    std::copy( selectedTables.begin() + 1, selectedTables.end(), std::back_inserter( additionalDataSources ) );

    auto firstPlot = vfpPlotColl->createAndAppendPlots( mainDataSource, additionalDataSources );
    vfpPlotColl->updateConnectedEditors();
    RiuPlotMainWindowTools::onObjectAppended( firstPlot, firstPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCustomVfpPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create VFP Plot" );
    actionToSetup->setIcon( QIcon( ":/VfpPlot.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimVfpTable*> RicNewCustomVfpPlotFeature::selectedTables()
{
    auto tables            = caf::selectedObjectsByTypeStrict<RimVfpTable*>();
    auto selectedTableData = caf::selectedObjectsByTypeStrict<RimVfpTableData*>();
    for ( auto tableData : selectedTableData )
    {
        auto tableDataSources = tableData->tableDataSources();
        tables.insert( tables.end(), tableDataSources.begin(), tableDataSources.end() );
    }

    return tables;
}
