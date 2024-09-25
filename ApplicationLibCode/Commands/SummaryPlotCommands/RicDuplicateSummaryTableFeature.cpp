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

#include "RicDuplicateSummaryTableFeature.h"

#include "RiaSummaryTools.h"

#include "RimMainPlotCollection.h"
#include "RimSummaryTable.h"
#include "RimSummaryTableCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDuplicateSummaryTableFeature, "RicDuplicateSummaryTableFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateSummaryTableFeature::onActionTriggered( bool isChecked )
{
    caf::PdmObject*  selObj       = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    RimSummaryTable* summaryTable = RiaSummaryTools::parentSummaryTable( selObj );
    if ( !summaryTable ) return;

    copyTableAndAddToCollection( summaryTable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateSummaryTableFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Duplicate Summary Table" );
    actionToSetup->setIcon( QIcon( ":/CorrelationMatrixPlot16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateSummaryTableFeature::copyTableAndAddToCollection( RimSummaryTable* sourceTable )
{
    RimSummaryTableCollection* summaryTableColl = RimMainPlotCollection::current()->summaryTableCollection();
    if ( !summaryTableColl ) return;

    auto newSummaryTable = sourceTable->copyObject<RimSummaryTable>();
    CVF_ASSERT( newSummaryTable );

    // Add table to collection
    summaryTableColl->addTable( newSummaryTable );

    // Resolve references after object has been inserted into the data model
    newSummaryTable->resolveReferencesRecursively();
    newSummaryTable->initAfterReadRecursively();

    // Update name
    QString nameOfCopy = QString( "Copy of " ) + sourceTable->description();
    newSummaryTable->setDescription( nameOfCopy );

    summaryTableColl->updateConnectedEditors();
    newSummaryTable->loadDataAndUpdate();
}
