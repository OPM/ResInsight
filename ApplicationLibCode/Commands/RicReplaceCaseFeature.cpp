/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicReplaceCaseFeature.h"

#include "RiaDefines.h"
#include "RiaEclipseFileNameTools.h"
#include "RiaGuiApplication.h"
#include "Summary/RiaSummaryTools.h"

#include "RicImportGeneralDataFeature.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimReloadCaseTools.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimTimeStepFilter.h"

#include "Riu3dSelectionManager.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicReplaceCaseFeature, "RicReplaceCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReplaceCaseFeature::isCommandEnabled() const
{
    const auto objects = caf::SelectionManager::instance()->objectsByType<RimEclipseResultCase>();
    return !objects.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceCaseFeature::onActionTriggered( bool isChecked )
{
    auto eclipseResultCase = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseResultCase>();
    if ( !eclipseResultCase ) return;

    auto summaryCase = RimReloadCaseTools::findSummaryCaseFromEclipseResultCase( eclipseResultCase );

    RiaGuiApplication::clearAllSelections();

    const QStringList fileNames = RicImportGeneralDataFeature::getEclipseFileNamesWithDialog( RiaDefines::ImportFileType::ECLIPSE_RESULT_GRID );
    if ( fileNames.isEmpty() ) return;

    const auto& fileName = fileNames.front();

    eclipseResultCase->setGridFileName( fileName );
    eclipseResultCase->reloadEclipseGridFile();

    std::vector<RimTimeStepFilter*> timeStepFilter = eclipseResultCase->descendantsIncludingThisOfType<RimTimeStepFilter>();
    if ( timeStepFilter.size() == 1 )
    {
        timeStepFilter[0]->clearFilteredTimeSteps();
    }

    RimReloadCaseTools::reloadEclipseGrid( eclipseResultCase );
    eclipseResultCase->updateConnectedEditors();

    // Use the file base name as case user description
    QFileInfo fileInfoNew( fileName );
    eclipseResultCase->setCaseUserDescription( fileInfoNew.baseName() );

    RiaEclipseFileNameTools helper( fileName );
    auto                    summaryFileNames = helper.findSummaryFileCandidates();
    if ( summaryCase && !summaryFileNames.empty() )
    {
        QMessageBox msgBox;
        msgBox.setIcon( QMessageBox::Question );

        QString questionText;
        questionText = QString(
            "Found an open summary case for the same reservoir model.\n\nDo you want to replace the file name for the summary case?" );

        msgBox.setText( questionText );
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );

        int ret = msgBox.exec();
        if ( ret == QMessageBox::Yes )
        {
            summaryCase->setSummaryHeaderFileName( summaryFileNames.front() );

            RiaSummaryTools::reloadSummaryCaseAndUpdateConnectedPlots( summaryCase );
            summaryCase->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Replace" );
    actionToSetup->setIcon( QIcon( ":/ReplaceCase16x16.png" ) );
}
