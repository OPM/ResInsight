/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RicImportRevealSummaryCaseFeature.h"

#include "RiaApplication.h"

#include "RimCsvSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"

#include "RiuFileDialogTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicImportRevealSummaryCaseFeature, "RicImportRevealSummaryCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportRevealSummaryCaseFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app     = RiaApplication::instance();
    RimProject*     project = app->project();

    RimSummaryCaseMainCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection() : nullptr;
    if ( !sumCaseColl ) return;

    QString     pattern    = "Reveal Summary Files (*.csv)";
    QString     defaultDir = app->lastUsedDialogDirectory( "SUMMARY_CASE_DIR" );
    QStringList filePaths  = RiuFileDialogTools::getOpenFileNames( nullptr, "Import Data File", defaultDir, pattern );
    for ( const QString& filePath : filePaths )
    {
        auto newSumCase = new RimCsvSummaryCase();

        newSumCase->setSummaryHeaderFileName( filePath );
        project->assignCaseIdToSummaryCase( newSumCase );

        sumCaseColl->addCase( newSumCase );
    }

    sumCaseColl->loadAllSummaryCaseData();
    sumCaseColl->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportRevealSummaryCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/SummaryCase.svg" ) );
    actionToSetup->setText( "Import Reveal Summary Case" );
}
