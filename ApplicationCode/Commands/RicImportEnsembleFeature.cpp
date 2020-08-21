/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicImportEnsembleFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicCreateSummaryCaseCollectionFeature.h"
#include "RicImportSummaryCasesFeature.h"

#include "RifSummaryCaseRestartSelector.h"

#include "RimEnsembleCurveSet.h"
#include "RimGridSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include "SummaryPlotCommands/RicNewSummaryEnsembleCurveSetFeature.h"
#include "SummaryPlotCommands/RicNewSummaryPlotFeature.h"

#include <QAction>
#include <QInputDialog>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicImportEnsembleFeature, "RicImportEnsembleFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportEnsembleFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app           = RiaApplication::instance();
    QString         pathCacheName = "ENSEMBLE_FILES";
    QStringList     fileNames =
        RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialog( "Import Ensemble", pathCacheName );

    if ( fileNames.isEmpty() ) return;

    QString ensembleName = askForEnsembleName();
    if ( ensembleName.isEmpty() ) return;

    std::vector<RimSummaryCase*> cases;
    RicImportSummaryCasesFeature::createSummaryCasesFromFiles( fileNames, &cases, true );

    RicImportSummaryCasesFeature::addSummaryCases( cases );
    RimSummaryCaseCollection* ensemble =
        RicCreateSummaryCaseCollectionFeature::groupSummaryCases( cases, ensembleName, true );

    if ( ensemble )
    {
        RicNewSummaryEnsembleCurveSetFeature::createPlotForCurveSetsAndUpdate( {ensemble} );
    }

    std::vector<RimCase*> allCases;
    app->project()->allCases( allCases );

    if ( allCases.size() == 0 )
    {
        RiuMainWindow::instance()->close();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/SummaryEnsemble24x24.png" ) );
    actionToSetup->setText( "Import Ensemble" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportEnsembleFeature::askForEnsembleName()
{
    RimProject*                            project = RimProject::current();
    std::vector<RimSummaryCaseCollection*> groups  = project->summaryGroups();
    int ensembleCount = std::count_if( groups.begin(), groups.end(), []( RimSummaryCaseCollection* group ) {
        return group->isEnsemble();
    } );
    ensembleCount += 1;

    QInputDialog dialog;
    dialog.setInputMode( QInputDialog::TextInput );
    dialog.setWindowTitle( "Ensemble Name" );
    dialog.setLabelText( "Ensemble Name" );
    dialog.setTextValue( QString( "Ensemble %1" ).arg( ensembleCount ) );
    dialog.resize( 300, 50 );
    dialog.exec();
    return dialog.result() == QDialog::Accepted ? dialog.textValue() : QString( "" );
}
