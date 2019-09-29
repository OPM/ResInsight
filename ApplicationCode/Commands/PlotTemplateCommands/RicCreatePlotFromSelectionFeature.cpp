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

#include "RicCreatePlotFromSelectionFeature.h"

#include "RiaGuiApplication.h"

#include "RicSelectPlotTemplateUi.h"
#include "RicSummaryPlotTemplateTools.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "RimDialogData.h"
#include "RimProject.h"
#include "RimSummaryCase.h"

#include "RiuPlotMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicCreatePlotFromSelectionFeature, "RicCreatePlotFromSelectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreatePlotFromSelectionFeature::isCommandEnabled()
{
    return !selectedSummaryCases().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreatePlotFromSelectionFeature::onActionTriggered( bool isChecked )
{
    RiuPlotMainWindow*       plotwindow = RiaGuiApplication::instance()->mainPlotWindow();
    RicSelectPlotTemplateUi* ui = RiaGuiApplication::instance()->project()->dialogData()->selectPlotTemplateUi();

    caf::PdmUiPropertyViewDialog propertyDialog( plotwindow, ui, "Select Plot Template", "" );

    if ( propertyDialog.exec() != QDialog::Accepted ) return;

    if ( ui->selectedPlotTemplates().empty() ) return;

    QString                      fileName = ui->selectedPlotTemplates().front()->absoluteFilePath();
    std::vector<RimSummaryCase*> sumCases = selectedSummaryCases();

    RimSummaryPlot* newSummaryPlot = RicSummaryPlotTemplateTools::createPlotFromTemplateFile( fileName );
    RicSummaryPlotTemplateTools::appendSummaryPlotToPlotCollection( newSummaryPlot, sumCases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreatePlotFromSelectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Plot from Template" );
    actionToSetup->setIcon( QIcon( ":/SummaryTemplate16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RicCreatePlotFromSelectionFeature::selectedSummaryCases() const
{
    std::vector<RimSummaryCase*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    return objects;
}
