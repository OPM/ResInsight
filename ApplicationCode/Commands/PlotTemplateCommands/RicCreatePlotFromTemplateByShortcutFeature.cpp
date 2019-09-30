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

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreatePlotFromTemplateByShortcutFeature, "RicCreatePlotFromTemplateByShortcutFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreatePlotFromTemplateByShortcutFeature::isCommandEnabled()
{
    return !selectedSummaryCases().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreatePlotFromTemplateByShortcutFeature::onActionTriggered( bool isChecked )
{
    QString fileName = RiaApplication::instance()->preferences()->defaultPlotTemplateAbsolutePath();
    std::vector<RimSummaryCase*> sumCases = selectedSummaryCases();

    RimSummaryPlot* newSummaryPlot = RicSummaryPlotTemplateTools::createPlotFromTemplateFile( fileName );
    RicSummaryPlotTemplateTools::appendSummaryPlotToPlotCollection( newSummaryPlot, sumCases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreatePlotFromTemplateByShortcutFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Plot from Template" );
    actionToSetup->setIcon( QIcon( ":/SummaryTemplate16x16.png" ) );

    QKeySequence keySeq( Qt::CTRL + Qt::Key_T );
    actionToSetup->setShortcut( keySeq );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RicCreatePlotFromTemplateByShortcutFeature::selectedSummaryCases() const
{
    std::vector<RimSummaryCase*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    return objects;
}
