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

#include "Ensemble/RiaEnsembleImportTools.h"
#include "RiaLogging.h"
#include "Summary/RiaSummaryTools.h"

#include "RicImportSummaryCasesFeature.h"

#include "EnsembleFileSet/RimEnsembleFileSet.h"
#include "EnsembleFileSet/RimEnsembleFileSetCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"
#include "Summary/Ensemble/RimSummaryFileSetEnsemble.h"

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

    RiaLogging::info( QString( "Starting replace ensemble for '" ) + summaryEnsemble->name() + "'" );

    QString pathCacheName = "ENSEMBLE_FILES";
    auto    result = RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialogWithGrouping( "Import Ensemble", pathCacheName );
    QStringList          fileNames = result.files;
    RiaDefines::FileType fileType  = RicRecursiveFileSearchDialog::mapSummaryFileType( result.fileType );

    if ( fileNames.isEmpty() ) return;
    if ( fileType != RiaDefines::FileType::SMSPEC ) return;

    if ( auto fileSetEnsemble = dynamic_cast<RimSummaryFileSetEnsemble*>( summaryEnsemble ) )
    {
        if ( auto fileSet = fileSetEnsemble->ensembleFileSet() )
        {
            auto collection = RimProject::current()->ensembleFileSetCollection();

            fileSet->findAndSetPathPatternAndRangeString( fileNames );

            collection->updateFileSetNames();
            collection->updateAllRequiredEditors();

            RiaLogging::info( QString( "Completed replace ensemble, new name is '" ) + summaryEnsemble->name() + "'" );

            return;
        }

        RiaLogging::error( "Failed to get ensemble file set from summary ensemble." );
        return;
    }

    RiaEnsembleImportTools::CreateConfig createConfig{ .fileType              = fileType,
                                                       .ensembleOrGroup       = false,
                                                       .allowDialogs          = false,
                                                       .buildSummaryAddresses = false };

    auto newCases = RiaEnsembleImportTools::createSummaryCasesFromFiles( fileNames, createConfig );
    if ( newCases.empty() )
    {
        RiaLogging::warning( "No new cases are created." );
        return;
    }

    summaryEnsemble->replaceCases( newCases );

    // Update name of cases and ensemble after all cases are added
    for ( auto summaryCase : newCases )
    {
        summaryCase->setDisplayNameOption( RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME );
        summaryCase->updateAutoShortName();
    }

    if ( auto sumCaseMainColl = RiaSummaryTools::summaryCaseMainCollection() )
    {
        sumCaseMainColl->updateEnsembleNames();
        sumCaseMainColl->updateAllRequiredEditors();
    }

    RiaSummaryTools::updateConnectedPlots( summaryEnsemble );

    RiaLogging::info( QString( "Completed replace ensemble, new name is '" ) + summaryEnsemble->name() + "'" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceSummaryEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Replace" );
    actionToSetup->setIcon( QIcon( ":/ReplaceCase16x16.png" ) );
}
