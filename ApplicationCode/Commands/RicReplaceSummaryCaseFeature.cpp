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

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaSummaryTools.h"

#include "RicImportGeneralDataFeature.h"

#include "RimCalculatedSummaryCase.h"
#include "RimFileSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryCurve.h"
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

    const QStringList fileNames =
        RicImportGeneralDataFeature::getEclipseFileNamesWithDialog( RiaDefines::ECLIPSE_SUMMARY_FILE );
    if ( fileNames.isEmpty() ) return;

    QString oldSummaryHeaderFilename = summaryCase->summaryHeaderFilename();
    summaryCase->setSummaryHeaderFileName( fileNames[0] );
    summaryCase->resetAutoShortName();
    summaryCase->createSummaryReaderInterface();
    summaryCase->createRftReaderInterface();
    RiaLogging::info( QString( "Replaced summary data for %1" ).arg( oldSummaryHeaderFilename ) );

    RimSummaryCalculationCollection* calcColl = RiaApplication::instance()->project()->calculationCollection();

    // Find and update all changed calculations
    std::set<int> ids;
    for ( RimSummaryCalculation* summaryCalculation : calcColl->calculations() )
    {
        bool needsUpdate = checkIfCalculationNeedsUpdate( summaryCalculation, summaryCase );
        if ( needsUpdate )
        {
            ids.insert( summaryCalculation->id() );
            summaryCalculation->parseExpression();
            summaryCalculation->calculate();
            summaryCalculation->updateDependentCurvesAndPlots();
        }
    }

    RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();
    for ( RimSummaryPlot* summaryPlot : summaryPlotColl->summaryPlots )
    {
        // Update summary curves on calculated data
        std::vector<RimSummaryCurve*> summaryCurves = summaryPlot->summaryCurves();
        for ( RimSummaryCurve* summaryCurve : summaryCurves )
        {
            RifEclipseSummaryAddress summaryAddressY = summaryCurve->summaryAddressY();
            if ( summaryAddressY.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED &&
                 ids.find( summaryAddressY.id() ) != ids.end() )
            {
                if ( calcColl )
                {
                    RimSummaryCalculation* calculation = calcColl->findCalculationById( summaryAddressY.id() );
                    QString                description = calculation->description();

                    RifEclipseSummaryAddress updatedAdr =
                        RifEclipseSummaryAddress::calculatedAddress( description.toStdString(), calculation->id() );
                    summaryCurve->setSummaryAddressYAndApplyInterpolation( updatedAdr );
                    summaryCurve->loadDataAndUpdate( true );
                }
            }
        }

        summaryPlot->loadDataAndUpdate();
    }

    RimSummaryCrossPlotCollection* summaryCrossPlotColl = RiaSummaryTools::summaryCrossPlotCollection();
    for ( RimSummaryPlot* summaryPlot : summaryCrossPlotColl->summaryPlots() )
    {
        // Update summary curves on calculated data
        std::vector<RimSummaryCurve*> summaryCurves = summaryPlot->summaryCurves();
        for ( RimSummaryCurve* summaryCurve : summaryCurves )
        {
            RifEclipseSummaryAddress summaryAddressX = summaryCurve->summaryAddressX();
            if ( summaryAddressX.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED &&
                 ids.find( summaryAddressX.id() ) != ids.end() )
            {
                if ( calcColl )
                {
                    RimSummaryCalculation* calculation = calcColl->findCalculationById( summaryAddressX.id() );
                    QString                description = calculation->description();

                    RifEclipseSummaryAddress updatedAdr =
                        RifEclipseSummaryAddress::calculatedAddress( description.toStdString(), calculation->id() );
                    summaryCurve->setSummaryAddressX( updatedAdr );
                    summaryCurve->loadDataAndUpdate( true );
                }
            }

            RifEclipseSummaryAddress summaryAddressY = summaryCurve->summaryAddressY();
            if ( summaryAddressY.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED &&
                 ids.find( summaryAddressY.id() ) != ids.end() )
            {
                if ( calcColl )
                {
                    RimSummaryCalculation* calculation = calcColl->findCalculationById( summaryAddressX.id() );
                    QString                description = calculation->description();

                    RifEclipseSummaryAddress updatedAdr =
                        RifEclipseSummaryAddress::calculatedAddress( description.toStdString(), calculation->id() );
                    summaryCurve->setSummaryAddressYAndApplyInterpolation( updatedAdr );
                    summaryCurve->loadDataAndUpdate( true );
                }
            }
        }

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReplaceSummaryCaseFeature::checkIfCalculationNeedsUpdate( const RimSummaryCalculation* summaryCalculation,
                                                                  const RimFileSummaryCase*    summaryCase )
{
    std::vector<RimSummaryCalculationVariable*> variables = summaryCalculation->allVariables();
    for ( RimSummaryCalculationVariable* variable : variables )
    {
        if ( variable->summaryCase() == summaryCase )
        {
            return true;
        }
    }

    return false;
}
