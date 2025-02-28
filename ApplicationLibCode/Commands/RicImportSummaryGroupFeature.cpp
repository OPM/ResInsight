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

#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "RicCreateSummaryCaseCollectionFeature.h"
#include "RicImportSummaryCasesFeature.h"
#include "Summary/RiaSummaryDefines.h"

#include "RifSummaryCaseRestartSelector.h"

#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include <QAction>
#include <QInputDialog>

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

    RicImportSummaryCasesFeature::CreateConfig createConfig{ .fileType = fileType, .ensembleOrGroup = true, .allowDialogs = true };
    auto [isOk, cases] = RicImportSummaryCasesFeature::createSummaryCasesFromFiles( fileNames, createConfig );

    RicImportSummaryCasesFeature::addSummaryCases( cases );
    RicCreateSummaryCaseCollectionFeature::groupSummaryCases( cases, "", RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE, false );

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
