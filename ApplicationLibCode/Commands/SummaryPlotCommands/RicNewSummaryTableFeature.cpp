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

#include "RifEclipseSummaryAddress.h"

#include "RimMainPlotCollection.h"
#include "RimSummaryAddress.h"
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

    // Summary table collection selection
    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        tableColl = RiaSummaryTools::parentSummaryTableCollection( selObj );
    }
    if ( tableColl ) return true;

    // Summary Address selection - only for enabled categories
    RimSummaryAddress* selectedSummaryAddress = nullptr;
    if ( selObj )
    {
        selObj->firstAncestorOrThisOfType( selectedSummaryAddress );
    }
    if ( selectedSummaryAddress && m_enabledCategories.contains( selectedSummaryAddress->address().category() ) ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryTableFeature::onActionTriggered( bool isChecked )
{
    RimSummaryTableCollection* summaryTableColl = RimMainPlotCollection::current()->summaryTableCollection();
    if ( !summaryTableColl ) return;

    caf::PdmObject*    selObj                 = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    RimSummaryAddress* selectedSummaryAddress = nullptr;
    if ( selObj )
    {
        selObj->firstAncestorOrThisOfType( selectedSummaryAddress );
    }

    RimSummaryTable* summaryTable = nullptr;
    if ( selectedSummaryAddress )
    {
        const auto adrObj = selectedSummaryAddress->address();
        if ( !m_enabledCategories.contains( adrObj.category() ) ) return;

        RimSummaryCase* summaryCase = RiaSummaryTools::summaryCaseById( selectedSummaryAddress->caseId() );
        if ( !summaryCase ) return;

        summaryTable = summaryTableColl->createSummaryTableFromCategoryAndVectorName( summaryCase,
                                                                                      adrObj.category(),
                                                                                      QString::fromStdString( adrObj.vectorName() ) );
    }
    else
    {
        summaryTable = summaryTableColl->createDefaultSummaryTable();
    }

    // Add summary table to collection
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
