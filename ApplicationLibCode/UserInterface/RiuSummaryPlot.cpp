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

#include "RiuSummaryPlot.h"

#include "Commands/CorrelationPlotCommands/RicNewCorrelationPlotFeature.h"

#include "RimEnsembleCurveSet.h"
#include "RimPlot.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

#include "RiuPlotCurve.h"
#include "RiuPlotWidget.h"

#include "cafCmdFeatureMenuBuilder.h"

#include <QApplication>
#include <QMenu>
#include <QPointer>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryPlot::RiuSummaryPlot( RimSummaryPlot* plot )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryPlot::~RiuSummaryPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryPlot::showContextMenu( QPoint pos )
{
    if ( !plotWidget() ) return;

    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    RimSummaryPlot* plot = dynamic_cast<RimSummaryPlot*>( plotWidget()->plotDefinition() );
    if ( plot )
    {
        QVariant plotVariant( QVariant::fromValue( static_cast<void*>( plot ) ) );
        menuBuilder.addCmdFeatureWithUserData( "RicShowPlotDataCtxFeature", "Show Plot Data", plotVariant );
        menuBuilder.addCmdFeatureWithUserData( "RicEditSummaryPlotCtxFeature", "Edit Plot", plotVariant );
        menuBuilder.addCmdFeatureWithUserData( "RicSplitMultiPlotFeature", "Split into Multiple Plots", plotVariant );
        menuBuilder.addSeparator();
        menuBuilder.addCmdFeatureWithUserData( "RicDeleteSubPlotCtxFeature", "Delete Plot", plotVariant );
        menuBuilder.addSeparator();

        for ( auto curveSet : plot->curveSets() )
        {
            if ( curveSet->isFiltered() )
            {
                auto curveSetVariant = QVariant::fromValue( caf::PdmPointer<RimEnsembleCurveSet>( curveSet ) );
                auto name            = "Create Ensemble from Filtered Cases: " + curveSet->name();
                menuBuilder.addCmdFeatureWithUserData( "RicCreateEnsembleFromFilteredCasesFeature", name, curveSetVariant );
            }
        }
    }
    menuBuilder.addSeparator();

    double distanceFromClick = std::numeric_limits<double>::infinity();

    auto [plotCurve, closestCurvePoint] = plotWidget()->findClosestCurve( pos, distanceFromClick );

    if ( plotCurve && closestCurvePoint >= 0 )
    {
        auto* summaryCurve = dynamic_cast<RimSummaryCurve*>( plotCurve->ownerRimCurve() );
        if ( summaryCurve && closestCurvePoint < (int)summaryCurve->timeStepsY().size() )
        {
            std::time_t timeStep = summaryCurve->timeStepsY()[closestCurvePoint];

            RimSummaryEnsemble* ensemble = nullptr;
            QString             clickedQuantityName;
            QStringList         allQuantityNamesInPlot;

            auto clickedEnsembleCurveSet = summaryCurve->firstAncestorOrThisOfType<RimEnsembleCurveSet>();

            bool curveClicked = distanceFromClick < 50;

            if ( clickedEnsembleCurveSet )
            {
                ensemble = clickedEnsembleCurveSet->summaryEnsemble();
                if ( ensemble && ensemble->isEnsemble() )
                {
                    clickedQuantityName = QString::fromStdString( clickedEnsembleCurveSet->summaryAddressY().uiText() );

                    if ( curveClicked )
                    {
                        QVariant curveVariant( QVariant::fromValue( static_cast<void*>( summaryCurve ) ) );
                        menuBuilder.addCmdFeatureWithUserData( "RicNewSummaryPlotFromCurveFeature", "Create New Plot from Curve", curveVariant );
                    }
                }
            }

            {
                auto summaryCase = summaryCurve->summaryCaseY();
                if ( summaryCase )
                {
                    int      summaryCaseId = summaryCase->caseId();
                    QVariant summaryCaseIdVariant( summaryCaseId );
                    auto     modelName = summaryCase->nativeCaseName();

                    menuBuilder.addCmdFeatureWithUserData( "RicImportGridModelFromSummaryCurveFeature",
                                                           QString( "Open Grid Model '%1'" ).arg( modelName ),
                                                           summaryCaseIdVariant );
                }
            }

            if ( !curveClicked )
            {
                auto*                             summaryPlot        = static_cast<RimSummaryPlot*>( plotWidget()->plotDefinition() );
                std::vector<RimEnsembleCurveSet*> allCurveSetsInPlot = summaryPlot->descendantsOfType<RimEnsembleCurveSet>();
                for ( auto curveSet : allCurveSetsInPlot )
                {
                    allQuantityNamesInPlot.push_back( QString::fromStdString( curveSet->summaryAddressY().uiText() ) );
                }
            }
            else
            {
                allQuantityNamesInPlot.push_back( clickedQuantityName );
            }

            if ( !clickedQuantityName.isEmpty() || !allQuantityNamesInPlot.isEmpty() )
            {
                if ( ensemble && ensemble->isEnsemble() )
                {
                    EnsemblePlotParams params( ensemble, allQuantityNamesInPlot, clickedQuantityName, timeStep );
                    QVariant           variant = QVariant::fromValue( params );

                    menuBuilder.addCmdFeatureWithUserData( "RicNewAnalysisPlotFeature", "New Analysis Plot", variant );

                    QString subMenuName = "Create Correlation Plot";
                    if ( curveClicked )
                    {
                        subMenuName = "Create Correlation Plot From Curve Point";
                    }
                    menuBuilder.subMenuStart( subMenuName, *caf::IconProvider( ":/CorrelationPlots16x16.png" ).icon() );

                    {
                        if ( curveClicked )
                        {
                            menuBuilder.addCmdFeatureWithUserData( "RicNewCorrelationPlotFeature", "New Tornado Plot", variant );
                        }
                        menuBuilder.addCmdFeatureWithUserData( "RicNewCorrelationMatrixPlotFeature", "New Matrix Plot", variant );
                        menuBuilder.addCmdFeatureWithUserData( "RicNewCorrelationReportPlotFeature", "New Report Plot", variant );
                        if ( curveClicked )
                        {
                            menuBuilder.subMenuStart( "Cross Plots", *caf::IconProvider( ":/CorrelationCrossPlot16x16.png" ).icon() );
                            std::vector<std::pair<RigEnsembleParameter, double>> ensembleParameters =
                                ensemble->parameterCorrelations( clickedEnsembleCurveSet->summaryAddressY(), timeStep );
                            std::sort( ensembleParameters.begin(),
                                       ensembleParameters.end(),
                                       []( const std::pair<RigEnsembleParameter, double>& lhs, const std::pair<RigEnsembleParameter, double>& rhs )
                                       { return std::fabs( lhs.second ) > std::fabs( rhs.second ); } );

                            for ( const auto& param : ensembleParameters )
                            {
                                if ( std::fabs( param.second ) >= 1.0e-6 )
                                {
                                    params.ensembleParameter = param.first.name;
                                    variant                  = QVariant::fromValue( params );
                                    menuBuilder.addCmdFeatureWithUserData( "RicNewParameterResultCrossPlotFeature",
                                                                           QString( "New Cross Plot Against %1 "
                                                                                    "(Correlation: %2)" )
                                                                               .arg( param.first.name )
                                                                               .arg( param.second, 5, 'f', 2 ),
                                                                           variant );
                                }
                            }
                            menuBuilder.subMenuEnd();
                        }
                    }
                    menuBuilder.subMenuEnd();
                }
            }
        }
    }

    menuBuilder.appendToMenu( &menu );

    if ( !menu.actions().empty() )
    {
        menu.exec( plotWidget()->mapToGlobal( pos ) );

        // Parts of progress dialog GUI can be present after menu has closed related to
        // RicImportGridModelFromSummaryCurveFeature. Make sure the plot is updated, and call processEvents() to make
        // sure all GUI events are processed
        // plotWidget()->update();
        QApplication::processEvents();
    }
}
