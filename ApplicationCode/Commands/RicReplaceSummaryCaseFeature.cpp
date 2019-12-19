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

#include "RiaLogging.h"
#include "RiaSummaryTools.h"

#include "RicImportGeneralDataFeature.h"

#include "RimFileSummaryCase.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReplaceSummaryCaseFeature, "RicReplaceSummaryCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReplaceSummaryCaseFeature::isCommandEnabled()
{
    RimSummaryCase* rimSummaryCase = caf::SelectionManager::instance()->selectedItemOfType<RimFileSummaryCase>();
    return rimSummaryCase != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReplaceSummaryCaseFeature::onActionTriggered( bool isChecked )
{
    RimFileSummaryCase* summaryCase = caf::SelectionManager::instance()->selectedItemOfType<RimFileSummaryCase>();
    if ( !summaryCase ) return;

    const QStringList fileNames = RicImportGeneralDataFeature::getEclipseFileNamesWithDialog(
        RiaDefines::ECLIPSE_SUMMARY_FILE );
    if ( fileNames.isEmpty() ) return;

    QString oldSummaryHeaderFilename = summaryCase->summaryHeaderFilename();
    summaryCase->setSummaryHeaderFileName( fileNames[0] );
    summaryCase->resetAutoShortName();
    summaryCase->createSummaryReaderInterface();
    summaryCase->createRftReaderInterface();
    RiaLogging::info( QString( "Replaced summary data for %1" ).arg( oldSummaryHeaderFilename ) );

    RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();
    for ( RimSummaryPlot* summaryPlot : summaryPlotColl->summaryPlots )
    {
        summaryPlot->loadDataAndUpdate();
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
