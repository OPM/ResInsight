////////////////////////////////////////////////////////////////////////////////
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

#include "RicCreatePlotFromTemplateByShortcutFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicSummaryPlotTemplateTools.h"

#include "RimSummaryCase.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFile>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicCreatePlotFromTemplateByShortcutFeature, "RicCreatePlotFromTemplateByShortcutFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreatePlotFromTemplateByShortcutFeature::isCommandEnabled()
{
    bool anySummaryCases           = !RicSummaryPlotTemplateTools::selectedSummaryCases().empty();
    bool anySummaryCaseCollections = !RicSummaryPlotTemplateTools::selectedSummaryCaseCollections().empty();

    return ( anySummaryCases || anySummaryCaseCollections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreatePlotFromTemplateByShortcutFeature::onActionTriggered( bool isChecked )
{
    QString fileName = RiaApplication::instance()->preferences()->defaultPlotTemplateAbsolutePath();

    if ( !QFile::exists( fileName ) )
    {
        auto mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();

        auto reply = QMessageBox::question( mainPlotWindow,
                                            QString( "No last used plot template found." ),
                                            QString( "Do you want to select plot template? " ),
                                            QMessageBox::Yes | QMessageBox::No );

        if ( reply == QMessageBox::No ) return;

        QString fileNameSelectedInUi = RicSummaryPlotTemplateTools::selectPlotTemplatePath();
        if ( fileNameSelectedInUi.isEmpty() ) return;

        fileName = fileNameSelectedInUi;
    }

    auto sumCases           = RicSummaryPlotTemplateTools::selectedSummaryCases();
    auto sumCaseCollections = RicSummaryPlotTemplateTools::selectedSummaryCaseCollections();

    RimSummaryPlot* newSummaryPlot = RicSummaryPlotTemplateTools::createPlotFromTemplateFile( fileName );
    RicSummaryPlotTemplateTools::appendSummaryPlotToPlotCollection( newSummaryPlot, sumCases, sumCaseCollections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreatePlotFromTemplateByShortcutFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Plot from Last Used Template" );
    actionToSetup->setIcon( QIcon( ":/SummaryTemplate16x16.png" ) );

    QKeySequence keySeq( Qt::CTRL + Qt::Key_T );

    applyShortcutWithHintToAction( actionToSetup, keySeq );
}
