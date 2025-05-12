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

#include "RicImportSummaryGroupFeature.h"

#include "Ensemble/RiaEnsembleImportTools.h"
#include "RiaGuiApplication.h"
#include "Summary/RiaSummaryDefines.h"

#include "RicImportEnsembleFeature.h"
#include "RicImportSummaryCasesFeature.h"

#include "RimProject.h"
#include "RimSummaryCase.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicImportSummaryGroupFeature, "RicImportSummaryGroupFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSummaryGroupFeature::onActionTriggered( bool isChecked )
{
    RiaGuiApplication* app           = RiaGuiApplication::instance();
    QString            pathCacheName = "INPUT_FILES";
    auto result = RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialog( "Import Summary Case Group", pathCacheName );
    QStringList fileNames = result.files;
    if ( fileNames.isEmpty() ) return;

    RiaDefines::FileType fileType = RicRecursiveFileSearchDialog::mapSummaryFileType( result.fileType );

    RiaEnsembleImportTools::CreateConfig createConfig{ .fileType = fileType, .ensembleOrGroup = true, .allowDialogs = true };
    auto                                 cases = RiaEnsembleImportTools::createSummaryCasesFromFiles( fileNames, createConfig );

    RicImportSummaryCasesFeature::addSummaryCases( cases );
    RicImportEnsembleFeature::groupSummaryCases( cases, "", RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE, false );

    RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
    if ( mainPlotWindow && !cases.empty() )
    {
        mainPlotWindow->selectAsCurrentItem( cases.back() );
        mainPlotWindow->updateMultiPlotToolBar();
    }

    std::vector<RimCase*> allCases = app->project()->allGridCases();
    if ( allCases.empty() )
    {
        RiuMainWindow::closeIfOpen();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSummaryGroupFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/SummaryGroup16x16.png" ) );
    actionToSetup->setText( "Import Summary Case Group" );
}
