/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicPlotProductionRateFeature.h"

#include "RiaGuiApplication.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"

#include "RifEclipseSummaryAddress.h"
#include "RifSummaryReaderInterface.h"

#include "RigSimWellData.h"

#include "RiaSummaryTools.h"
#include "Rim3dView.h"
#include "RimEclipseResultCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewTools.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicPlotProductionRateFeature, "RicPlotProductionRateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPlotProductionRateFeature::isCommandEnabled() const
{
    std::vector<RimSimWellInView*> collection;
    caf::SelectionManager::instance()->objectsByType( &collection );

    for ( RimSimWellInView* well : collection )
    {
        auto summaryCase = RimSimWellInViewTools::summaryCaseForWell( well );
        if ( summaryCase )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPlotProductionRateFeature::onActionTriggered( bool isChecked )
{
    RimProject* project = RimProject::current();
    CAF_ASSERT( project );

    RimSummaryCaseMainCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection() : nullptr;
    if ( !sumCaseColl ) return;

    std::vector<RimSimWellInView*> collection;
    caf::SelectionManager::instance()->objectsByType( &collection );

    RimSummaryPlot* summaryPlotToSelect = nullptr;

    for ( RimSimWellInView* well : collection )
    {
        auto summaryCase = RimSimWellInViewTools::summaryCaseForWell( well );
        if ( !summaryCase ) continue;

        QString description = "Well Production Rates : ";

        if ( RimSimWellInViewTools::isInjector( well ) )
        {
            description = "Well Injection Rates : ";
        }

        description += well->name();
        RimSummaryPlot* plot = new RimSummaryPlot();
        plot->setUiName( description );
        RimSummaryMultiPlot* multiPlot = RicSummaryPlotBuilder::createAndAppendSingleSummaryMultiPlot( plot );

        if ( RimSimWellInViewTools::isInjector( well ) )
        {
            // Left Axis

            RiaDefines::PlotAxis plotAxis = RiaDefines::PlotAxis::PLOT_AXIS_LEFT;

            {
                // Note : The parameter "WOIR" is probably never-existing, but we check for existence before creating
                // curve Oil
                QString parameterName = "WOIR";
                RicPlotProductionRateFeature::addSummaryCurve( plot,
                                                               well,
                                                               summaryCase,
                                                               parameterName,
                                                               plotAxis,
                                                               RimSummaryCurveAppearanceCalculator::cycledGreenColor( 0 ) );
            }

            {
                // Water
                QString parameterName = "WWIR";
                RicPlotProductionRateFeature::addSummaryCurve( plot,
                                                               well,
                                                               summaryCase,
                                                               parameterName,
                                                               plotAxis,
                                                               RimSummaryCurveAppearanceCalculator::cycledBlueColor( 0 ) );
            }

            {
                // Gas
                QString parameterName = "WGIR";
                RicPlotProductionRateFeature::addSummaryCurve( plot,
                                                               well,
                                                               summaryCase,
                                                               parameterName,
                                                               plotAxis,
                                                               RimSummaryCurveAppearanceCalculator::cycledRedColor( 0 ) );
            }
        }
        else
        {
            // Left Axis

            RiaDefines::PlotAxis plotAxis = RiaDefines::PlotAxis::PLOT_AXIS_LEFT;

            {
                // Oil
                QString parameterName = "WOPR";
                RicPlotProductionRateFeature::addSummaryCurve( plot,
                                                               well,
                                                               summaryCase,
                                                               parameterName,
                                                               plotAxis,
                                                               RimSummaryCurveAppearanceCalculator::cycledGreenColor( 0 ) );
            }

            {
                // Water
                QString parameterName = "WWPR";
                RicPlotProductionRateFeature::addSummaryCurve( plot,
                                                               well,
                                                               summaryCase,
                                                               parameterName,
                                                               plotAxis,
                                                               RimSummaryCurveAppearanceCalculator::cycledBlueColor( 0 ) );
            }

            {
                // Gas
                QString parameterName = "WGPR";
                RicPlotProductionRateFeature::addSummaryCurve( plot,
                                                               well,
                                                               summaryCase,
                                                               parameterName,
                                                               plotAxis,
                                                               RimSummaryCurveAppearanceCalculator::cycledRedColor( 0 ) );
            }
        }

        // Right Axis

        {
            RiaDefines::PlotAxis plotAxis = RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;

            {
                QString parameterName = "WTHP";
                RicPlotProductionRateFeature::addSummaryCurve( plot,
                                                               well,
                                                               summaryCase,
                                                               parameterName,
                                                               plotAxis,
                                                               RimSummaryCurveAppearanceCalculator::cycledNoneRGBBrColor( 0 ) );
            }

            {
                QString parameterName = "WBHP";
                RicPlotProductionRateFeature::addSummaryCurve( plot,
                                                               well,
                                                               summaryCase,
                                                               parameterName,
                                                               plotAxis,
                                                               RimSummaryCurveAppearanceCalculator::cycledNoneRGBBrColor( 1 ) );
            }
        }

        multiPlot->updateConnectedEditors();
        plot->loadDataAndUpdate();

        summaryPlotToSelect = plot;
    }

    if ( summaryPlotToSelect )
    {
        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();
        if ( mainPlotWindow )
        {
            mainPlotWindow->selectAsCurrentItem( summaryPlotToSelect );
            mainPlotWindow->setExpanded( summaryPlotToSelect );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPlotProductionRateFeature::setupActionLook( QAction* actionToSetup )
{
    // actionToSetup->setIcon(QIcon(":/WellAllocPlot16x16.png"));
    actionToSetup->setText( "Plot Production Rates" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RicPlotProductionRateFeature::addSummaryCurve( RimSummaryPlot*         plot,
                                                                const RimSimWellInView* well,
                                                                RimSummaryCase*         summaryCase,
                                                                const QString&          vectorName,
                                                                RiaDefines::PlotAxis    plotAxis,
                                                                const cvf::Color3f&     color )
{
    CVF_ASSERT( plot );
    CVF_ASSERT( summaryCase );
    CVF_ASSERT( well );

    RifEclipseSummaryAddress addr( RifEclipseSummaryAddress::SUMMARY_WELL,
                                   vectorName.toStdString(),
                                   -1,
                                   -1,
                                   "",
                                   well->name().toStdString(),
                                   -1,
                                   "",
                                   -1,
                                   -1,
                                   -1,
                                   -1,
                                   false,
                                   -1 );

    if ( !summaryCase->summaryReader()->hasAddress( addr ) )
    {
        return nullptr;
    }

    RimSummaryCurve* newCurve = new RimSummaryCurve();
    plot->addCurveAndUpdate( newCurve );

    newCurve->setSummaryCaseY( summaryCase );
    newCurve->setSummaryAddressYAndApplyInterpolation( addr );
    newCurve->setColor( color );
    newCurve->setLeftOrRightAxisY( RiuPlotAxis( plotAxis ) );
    newCurve->loadDataAndUpdate( true );

    return newCurve;
}
