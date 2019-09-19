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

#include "RicSelectPlotTemplateUi.h"
#include "RicSummaryPlotTemplateTools.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaSummaryTools.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "RimDialogData.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimWellPath.h"

#include "RiuPlotMainWindow.h"

#include "cafPdmObject.h"
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
    RiuPlotMainWindow* plotwindow = RiaGuiApplication::instance()->mainPlotWindow();

    RicSelectPlotTemplateUi* ui = RiaGuiApplication::instance()->project()->dialogData()->selectPlotTemplateUi();

    caf::PdmUiPropertyViewDialog propertyDialog( plotwindow, ui, "Select Plot Template", "" );

    if ( propertyDialog.exec() != QDialog::Accepted ) return;

    if ( ui->selectedPlotTemplates().empty() ) return;

    QString                      fileName = ui->selectedPlotTemplates().front()->absoluteFilePath();
    std::vector<RimSummaryCase*> sumCases = selectedSummaryCases();

    RimSummaryPlot* newSummaryPlot = RicSummaryPlotTemplateTools::createPlotFromTemplateFile( fileName );
    if ( newSummaryPlot )
    {
        RimSummaryPlotCollection* plotColl =
            RiaApplication::instance()->project()->mainPlotCollection()->summaryPlotCollection();

        plotColl->summaryPlots.push_back( newSummaryPlot );
        newSummaryPlot->resolveReferencesRecursively();
        newSummaryPlot->initAfterReadRecursively();

        QString nameOfCopy = QString( "Copy of " ) + newSummaryPlot->description();
        newSummaryPlot->setDescription( nameOfCopy );

        auto summaryCurves = newSummaryPlot->summaryCurves();

        for ( const auto& curve : summaryCurves )
        {
            auto fieldHandle = curve->findField( "SummaryCase" );
            if ( fieldHandle )
            {
                auto referenceString = fieldHandle->xmlCapability()->referenceString();
                auto stringList      = referenceString.split( " " );
                if ( stringList.size() == 2 )
                {
                    QString indexAsString = stringList[1];

                    bool conversionOk = false;
                    int  index        = indexAsString.toUInt( &conversionOk );

                    if ( conversionOk && index < sumCases.size() )
                    {
                        curve->setSummaryCaseY( sumCases[index] );
                    }
                }
            }
        }

        plotColl->updateConnectedEditors();

        newSummaryPlot->loadDataAndUpdate();
    }
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
