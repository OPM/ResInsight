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

#include "RicImportStimPlanSummaryCaseFeature.h"

#include "RiaApplication.h"

#include "RimCsvSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"

#include "RiuFileDialogTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicImportStimPlanSummaryCaseFeature, "RicImportStimPlanSummaryCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportStimPlanSummaryCaseFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app     = RiaApplication::instance();
    RimProject*     project = app->project();

    RimSummaryCaseMainCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection() : nullptr;
    if ( !sumCaseColl ) return;

    QString     pattern    = "StimPlan Summary Files (*.csv)";
    QString     defaultDir = app->lastUsedDialogDirectory( "SUMMARY_CASE_DIR" );
    QStringList filePaths  = RiuFileDialogTools::getOpenFileNames( nullptr, "Import Data File", defaultDir, pattern );
    for ( const QString& filePath : filePaths )
    {
        auto newSumCase = new RimCsvSummaryCase();
        newSumCase->setFileType( RimCsvSummaryCase::FileType::STIMPLAN );
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
void RicImportStimPlanSummaryCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/SummaryCase.svg" ) );
    actionToSetup->setText( "Import StimPlan Summary Case" );
}
