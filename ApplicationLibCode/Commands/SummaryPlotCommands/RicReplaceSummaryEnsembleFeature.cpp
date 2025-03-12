/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RicReplaceSummaryEnsembleFeature.h"

#include "Summary/RiaSummaryTools.h"

#include "RicImportSummaryCasesFeature.h"

#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReplaceSummaryEnsembleFeature, "RicReplaceSummaryEnsembleFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReplaceSummaryEnsembleFeature::isCommandEnabled() const
{
    auto summaryEnsemble = caf::SelectionManager::instance()->selectedItemOfType<RimSummaryEnsemble>();
    return summaryEnsemble != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceSummaryEnsembleFeature::onActionTriggered( bool isChecked )
{
    auto summaryEnsemble = caf::SelectionManager::instance()->selectedItemOfType<RimSummaryEnsemble>();
    if ( !summaryEnsemble ) return;

    QString pathCacheName = "ENSEMBLE_FILES";
    auto    result = RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialogWithGrouping( "Import Ensemble", pathCacheName );
    QStringList          fileNames = result.files;
    RiaDefines::FileType fileType  = RicRecursiveFileSearchDialog::mapSummaryFileType( result.fileType );

    if ( fileNames.isEmpty() ) return;
    if ( fileType != RiaDefines::FileType::SMSPEC ) return;

    RicImportSummaryCasesFeature::CreateConfig createConfig{ .fileType = fileType, .ensembleOrGroup = false, .allowDialogs = false };
    auto [isOk, newCases] = RicImportSummaryCasesFeature::createSummaryCasesFromFiles( fileNames, createConfig );
    if ( !isOk ) return;

    // Remove cases first, delegate delete to a separate thread
    auto casesToBeDeleted = summaryEnsemble->allSummaryCases();
    for ( auto summaryCase : casesToBeDeleted )
    {
        summaryEnsemble->removeCase( summaryCase );
    }

    // Add the summary case
    for ( auto summaryCase : newCases )
    {
        summaryEnsemble->addCase( summaryCase );
    }

    // Update name of cases and ensemble after all cases are added
    for ( auto summaryCase : newCases )
    {
        summaryCase->updateAutoShortName();
    }

    summaryEnsemble->ensureNameIsUpdated();

    if ( auto sumCaseMainColl = RiaSummaryTools::summaryCaseMainCollection() )
    {
        sumCaseMainColl->updateAllRequiredEditors();
    }

    // Delete previous cases in a separate thread, as this operation can be time-consuming
    caf::AsyncPdmObjectVectorDeleter<RimSummaryCase> summaryCaseDeleter( casesToBeDeleted );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceSummaryEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Replace" );
    actionToSetup->setIcon( QIcon( ":/ReplaceCase16x16.png" ) );
}
