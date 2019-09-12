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
#include "RiaLogging.h"
#include "RiaSummaryTools.h"

#include "RicSelectPlotTemplateUi.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
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
RicCreatePlotFromSelectionFeature::RicCreatePlotFromSelectionFeature() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreatePlotFromSelectionFeature::isCommandEnabled()
{
    if ( selectedSummaryCases().size() == 2 ) return true;
    if ( selectedWellPaths().size() == 2 ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreatePlotFromSelectionFeature::onActionTriggered( bool isChecked )
{
    {
        RiuPlotMainWindow* plotwindow = RiaGuiApplication::instance()->mainPlotWindow();

        RicSelectPlotTemplateUi ui;

        caf::PdmUiPropertyViewDialog propertyDialog( plotwindow,
                                                     &ui,
                                                     "Select Case to create Pressure Saturation plots",
                                                     "" );

        if ( propertyDialog.exec() != QDialog::Accepted ) return;

        if ( ui.selectedPlotTemplates().empty() ) return;

        QString fileName = ui.selectedPlotTemplates().front()->absoluteFilePath();
        {
            auto sumCases = selectedSummaryCases();
            if ( sumCases.size() == 2 )
            {
                //            QString fileName = "d:/projects/ri-plot-templates/one_well_two_cases.rpt";

                RimSummaryPlot* newSummaryPlot = createPlotFromTemplateFile( fileName );
                if ( newSummaryPlot )
                {
                    RimSummaryPlotCollection* plotColl =
                        RiaApplication::instance()->project()->mainPlotCollection()->summaryPlotCollection();

                    plotColl->summaryPlots.push_back( newSummaryPlot );

                    // Resolve references after object has been inserted into the data model
                    newSummaryPlot->resolveReferencesRecursively();
                    newSummaryPlot->initAfterReadRecursively();

                    QString nameOfCopy = QString( "Copy of " ) + newSummaryPlot->description();
                    newSummaryPlot->setDescription( nameOfCopy );

                    auto summaryCurves = newSummaryPlot->summaryCurves();
                    if ( summaryCurves.size() == sumCases.size() )
                    {
                        for ( size_t i = 0; i < summaryCurves.size(); i++ )
                        {
                            auto sumCase = sumCases[i];
                            summaryCurves[i]->setSummaryCaseY( sumCase );
                        }
                    }

                    plotColl->updateConnectedEditors();

                    newSummaryPlot->loadDataAndUpdate();
                }
            }
        }

        {
            auto wellPaths = selectedWellPaths();
            if ( wellPaths.size() == 2 )
            {
                // QString         fileName       = "d:/projects/ri-plot-templates/one_well_two_cases.rpt";
                RimSummaryPlot* newSummaryPlot = createPlotFromTemplateFile( fileName );
                if ( newSummaryPlot )
                {
                    RimSummaryPlotCollection* plotColl = RiaSummaryTools::summaryPlotCollection();

                    plotColl->summaryPlots.push_back( newSummaryPlot );

                    // Resolve references after object has been inserted into the data model
                    newSummaryPlot->resolveReferencesRecursively();
                    newSummaryPlot->initAfterReadRecursively();

                    QString nameOfCopy = QString( "Copy of " ) + newSummaryPlot->description();
                    newSummaryPlot->setDescription( nameOfCopy );

                    auto summaryCurves = newSummaryPlot->summaryCurves();
                    if ( summaryCurves.size() == wellPaths.size() )
                    {
                        for ( size_t i = 0; i < summaryCurves.size(); i++ )
                        {
                            auto wellPath = wellPaths[i];

                            summaryCurves[i]->summaryAddressY().setWellName( wellPath->name().toStdString() );
                        }
                    }

                    plotColl->updateConnectedEditors();

                    newSummaryPlot->loadDataAndUpdate();
                }
            }
        }
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
RimSummaryPlot* RicCreatePlotFromSelectionFeature::createPlotFromTemplateFile( const QString& fileName ) const
{
    QFile importFile( fileName );
    if ( !importFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        RiaLogging::error( QString( "Create Plot from Template : Could not open the file: %1" ).arg( fileName ) );
        return nullptr;
    }

    QTextStream stream( &importFile );

    QString objectAsText = stream.readAll();

    caf::PdmObjectHandle* obj =
        caf::PdmXmlObjectHandle::readUnknownObjectFromXmlString( objectAsText, caf::PdmDefaultObjectFactory::instance() );

    RimSummaryPlot* newSummaryPlot = dynamic_cast<RimSummaryPlot*>( obj );
    if ( newSummaryPlot )
    {
        return newSummaryPlot;
    }
    else
    {
        delete obj;
    }

    return nullptr;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicCreatePlotFromSelectionFeature::selectedWellPaths() const
{
    std::vector<RimWellPath*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    return objects;
}
