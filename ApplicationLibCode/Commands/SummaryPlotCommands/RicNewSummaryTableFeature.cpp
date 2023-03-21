/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RicNewSummaryTableFeature.h"

#include "RiaSummaryTools.h"

//#include "RicEditSummaryPlotFeature.h"
//#include "RicSummaryPlotEditorDialog.h"
//#include "RicSummaryPlotEditorUi.h"

#include "RimMainPlotCollection.h"
//#include "RimSummaryCase.h"
//#include "RimSummaryCaseCollection.h"
//#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryTable.h"
#include "RimSummaryTableCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSummaryTableFeature, "RicNewSummaryTableFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryTableFeature::isCommandEnabled()
{
    RimSummaryTableCollection* tableColl = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        tableColl = RiaSummaryTools::parentSummaryTableCollection( selObj );
    }

    if ( tableColl ) return true;

    // Multiple case selections
    /*std::vector<caf::PdmUiItem*> selectedItems = caf::selectedObjectsByTypeStrict<caf::PdmUiItem*>();

    for ( auto item : selectedItems )
    {
        RimSummaryCase*           sumCase  = dynamic_cast<RimSummaryCase*>( item );
        RimSummaryCaseCollection* sumGroup = dynamic_cast<RimSummaryCaseCollection*>( item );

        if ( sumGroup && sumGroup->isEnsemble() ) sumGroup = nullptr;
        if ( !sumCase && !sumGroup ) return false;
    }
    return true;*/
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryTableFeature::onActionTriggered( bool isChecked )
{
    RimSummaryTableCollection* summaryTableColl = RimMainPlotCollection::current()->summaryTableCollection();
    if ( !summaryTableColl ) return;

    RimSummaryTable* summaryTable = summaryTableColl->createSummaryTable();
    if ( summaryTable )
    {
        summaryTableColl->addTable( summaryTable );
        summaryTableColl->updateConnectedEditors();
        summaryTable->loadDataAndUpdate();

        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::onObjectAppended( summaryTable );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryTableFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Summary Table" );
    actionToSetup->setIcon( QIcon( ":/MultiPlot16x16.png" ) );
}
