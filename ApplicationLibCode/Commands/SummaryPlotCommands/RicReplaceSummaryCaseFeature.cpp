/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicReplaceSummaryCaseFeature.h"

#include "RiaEclipseFileNameTools.h"
#include "RiaLogging.h"
#include "Summary/RiaSummaryTools.h"

#include "RicImportGeneralDataFeature.h"

#include "RimEclipseCase.h"
#include "RimFileSummaryCase.h"
#include "RimReloadCaseTools.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicReplaceSummaryCaseFeature, "RicReplaceSummaryCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReplaceSummaryCaseFeature::isCommandEnabled() const
{
    auto rimSummaryCase = caf::SelectionManager::instance()->selectedItemOfType<RimFileSummaryCase>();
    return rimSummaryCase != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceSummaryCaseFeature::onActionTriggered( bool isChecked )
{
    auto* summaryCase = caf::SelectionManager::instance()->selectedItemOfType<RimFileSummaryCase>();
    if ( !summaryCase ) return;

    const QStringList fileNames =
        RicImportGeneralDataFeature::getEclipseFileNamesWithDialog( RiaDefines::ImportFileType::ECLIPSE_SUMMARY_FILE );
    if ( fileNames.isEmpty() ) return;

    auto gridModel = RimReloadCaseTools::gridModelFromSummaryCase( summaryCase );

    QString oldSummaryHeaderFilename = summaryCase->summaryHeaderFilename();

    const auto& newFileName = fileNames.front();
    summaryCase->setSummaryHeaderFileName( newFileName );
    RiaSummaryTools::reloadSummaryCaseAndUpdateConnectedPlots( summaryCase );
    summaryCase->updateConnectedEditors();

    RiaLogging::info( QString( "Replaced summary data for %1" ).arg( oldSummaryHeaderFilename ) );

    RiaEclipseFileNameTools helper( newFileName );
    auto                    newGridFileName = helper.findRelatedGridFile();
    if ( gridModel && !newGridFileName.isEmpty() )
    {
        QMessageBox msgBox;
        msgBox.setIcon( QMessageBox::Question );

        QString questionText;
        questionText =
            QString( "Found an open grid case for the same reservoir model.\n\nDo you want to replace the file name in the grid case?" );

        msgBox.setText( questionText );
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );

        int ret = msgBox.exec();
        if ( ret == QMessageBox::Yes )
        {
            auto previousGridFileName = gridModel->gridFileName();

            gridModel->setGridFileName( newGridFileName );

            RimReloadCaseTools::reloadEclipseGrid( gridModel );

            RiaLogging::info( QString( "Replaced grid data for %1" ).arg( previousGridFileName ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceSummaryCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Replace" );
    actionToSetup->setIcon( QIcon( ":/ReplaceCase16x16.png" ) );
}
