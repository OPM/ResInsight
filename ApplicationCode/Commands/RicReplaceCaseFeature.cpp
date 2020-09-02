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

#include "RiaGuiApplication.h"
#include "RiaSummaryTools.h"

#include "RicImportGeneralDataFeature.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimGridSummaryCase.h"
#include "RimReloadCaseTools.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimTimeStepFilter.h"

#include "Riu3dSelectionManager.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicReplaceCaseFeature, "RicReplaceCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReplaceCaseFeature::isCommandEnabled()
{
    std::vector<caf::PdmObject*> selectedFormationNamesCollObjs;
    caf::SelectionManager::instance()->objectsByType( &selectedFormationNamesCollObjs );
    for ( caf::PdmObject* pdmObject : selectedFormationNamesCollObjs )
    {
        if ( dynamic_cast<RimEclipseResultCase*>( pdmObject ) )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceCaseFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimEclipseResultCase*> selectedEclipseCases;
    caf::SelectionManager::instance()->objectsByType( &selectedEclipseCases );

    RiaGuiApplication::clearAllSelections();

    const QStringList fileNames =
        RicImportGeneralDataFeature::getEclipseFileNamesWithDialog( RiaDefines::ImportFileType::ECLIPSE_RESULT_GRID );
    if ( fileNames.isEmpty() ) return;

    const QString fileName = fileNames[0];

    for ( RimEclipseResultCase* selectedCase : selectedEclipseCases )
    {
        selectedCase->setGridFileName( fileName );
        selectedCase->reloadEclipseGridFile();

        std::vector<RimTimeStepFilter*> timeStepFilter;
        selectedCase->descendantsIncludingThisOfType( timeStepFilter );
        if ( timeStepFilter.size() == 1 )
        {
            timeStepFilter[0]->clearFilteredTimeSteps();
        }

        RimReloadCaseTools::reloadAllEclipseData( selectedCase );
        selectedCase->updateConnectedEditors();

        // Use the file base name as case user description
        QFileInfo fi( fileName );
        selectedCase->caseUserDescription = fi.baseName();

        // Find and update attached grid summary cases.
        RimSummaryCaseMainCollection* sumCaseColl = RiaSummaryTools::summaryCaseMainCollection();
        if ( sumCaseColl )
        {
            RimGridSummaryCase* gridSummaryCase =
                dynamic_cast<RimGridSummaryCase*>( sumCaseColl->findSummaryCaseFromEclipseResultCase( selectedCase ) );
            if ( gridSummaryCase )
            {
                gridSummaryCase->setAssociatedEclipseCase( selectedCase );
                gridSummaryCase->updateAutoShortName();
                gridSummaryCase->createSummaryReaderInterface();
                gridSummaryCase->createRftReaderInterface();

                RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();
                for ( RimSummaryPlot* summaryPlot : summaryPlotColl->summaryPlots )
                {
                    summaryPlot->loadDataAndUpdate();
                }
            }
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
