/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "Summary/RiaSummaryTools.h"

#include "RiaFilePathTools.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RifEclipseSummaryAddress.h"
#include "RifSummaryReaderInterface.h"

#include "RimDepthTrackPlot.h"
#include "RimMainPlotCollection.h"
#include "RimObservedDataCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTable.h"
#include "RimSummaryTableCollection.h"
#include "RimWellPlotTools.h"
#include "RimWellRftPlot.h"

#include "cafPdmObject.h"
#include "cafPdmObjectHandleTools.h"

#include <QRegularExpression>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlotCollection* RiaSummaryTools::summaryMultiPlotCollection()
{
    return RimMainPlotCollection::current()->summaryMultiPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RiaSummaryTools::singleTopLevelSummaryCases()
{
    if ( summaryCaseMainCollection() ) return summaryCaseMainCollection()->topLevelSummaryCases();

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection* RiaSummaryTools::summaryCaseMainCollection()
{
    RimProject* project = RimProject::current();
    return project->activeOilField()->summaryCaseMainCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedDataCollection* RiaSummaryTools::observedDataCollection()
{
    RimProject* project = RimProject::current();
    return project->activeOilField()->observedDataCollection();
}

//--------------------------------------------------------------------------------------------------
/// Update the summary curves referencing this curve, as the curve is identified by the name
//--------------------------------------------------------------------------------------------------
void RiaSummaryTools::notifyCalculatedCurveNameHasChanged( int calculationId, const QString& currentCurveName )
{
    RimSummaryMultiPlotCollection* summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection();

    for ( RimSummaryMultiPlot* multiPlot : summaryPlotColl->multiPlots() )
    {
        for ( RimSummaryPlot* plot : multiPlot->summaryPlots() )
        {
            for ( RimSummaryCurve* curve : plot->summaryCurves() )
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressY();
                if ( adr.isCalculated() && adr.id() == calculationId )
                {
                    adr.setVectorName( currentCurveName.toStdString() );
                    curve->setSummaryAddressY( adr );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RiaSummaryTools::parentSummaryPlot( caf::PdmObject* object )
{
    if ( object )
    {
        return object->firstAncestorOrThisOfType<RimSummaryPlot>();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlotCollection* RiaSummaryTools::parentSummaryPlotCollection( caf::PdmObject* object )
{
    if ( object )
    {
        return object->firstAncestorOrThisOfType<RimSummaryMultiPlotCollection>();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RiaSummaryTools::parentSummaryMultiPlot( caf::PdmObject* object )
{
    if ( object )
    {
        return object->firstAncestorOrThisOfType<RimSummaryMultiPlot>();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTable* RiaSummaryTools::parentSummaryTable( caf::PdmObject* object )
{
    if ( object )
    {
        return object->firstAncestorOrThisOfType<RimSummaryTable>();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTableCollection* RiaSummaryTools::parentSummaryTableCollection( caf::PdmObject* object )
{
    if ( object )
    {
        return object->firstAncestorOrThisOfType<RimSummaryTableCollection>();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddressDefines::CurveType RiaSummaryTools::identifyCurveType( const RifEclipseSummaryAddress& address )
{
    if ( address.isCalculated() )
    {
        std::vector<RimSummaryCase*>          cases;
        std::vector<RifEclipseSummaryAddress> addresses;

        getSummaryCasesAndAddressesForCalculation( address.id(), cases, addresses );
        for ( const RifEclipseSummaryAddress& variableAddress : addresses )
        {
            if ( !variableAddress.hasAccumulatedData() )
            {
                return RifEclipseSummaryAddressDefines::CurveType::RATE;
            }
        }

        // All the variables are accumulated
        return RifEclipseSummaryAddressDefines::CurveType::ACCUMULATED;
    }

    return address.hasAccumulatedData() ? RifEclipseSummaryAddressDefines::CurveType::ACCUMULATED
                                        : RifEclipseSummaryAddressDefines::CurveType::RATE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryTools::getSummaryCasesAndAddressesForCalculation( int                                    id,
                                                                 std::vector<RimSummaryCase*>&          cases,
                                                                 std::vector<RifEclipseSummaryAddress>& addresses )
{
    RimProject* proj = RimProject::current();

    RimSummaryCalculationCollection* calculationColl = proj->calculationCollection();
    if ( !calculationColl ) return;

    RimUserDefinedCalculation* calculation = calculationColl->findCalculationById( id );
    if ( !calculation ) return;

    for ( RimUserDefinedCalculationVariable* v : calculation->allVariables() )
    {
        auto* scv = dynamic_cast<RimSummaryCalculationVariable*>( v );
        if ( scv )
        {
            cases.push_back( scv->summaryCase() );
            addresses.push_back( scv->summaryAddress()->address() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<time_t>, std::vector<double>> RiaSummaryTools::resampledValuesForPeriod( const RifEclipseSummaryAddress& address,
                                                                                               const std::vector<time_t>&      timeSteps,
                                                                                               const std::vector<double>&      values,
                                                                                               RiaDefines::DateTimePeriod      period )
{
    // NB! The curve type can be overridden by the user, so there might be a discrepancy between the curve type and the curve type derived
    // from the address
    // See RimSummaryCurve::curveType()

    return resampledValuesForPeriod( RiaSummaryTools::identifyCurveType( address ), timeSteps, values, period );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<time_t>, std::vector<double>>
    RiaSummaryTools::resampledValuesForPeriod( RifEclipseSummaryAddressDefines::CurveType accumulatedOrRate,
                                               const std::vector<time_t>&                 timeSteps,
                                               const std::vector<double>&                 values,
                                               RiaDefines::DateTimePeriod                 period )
{
    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData( values, timeSteps );

    if ( accumulatedOrRate == CurveType::ACCUMULATED )
    {
        resampler.resampleAndComputePeriodEndValues( period );
    }
    else
    {
        resampler.resampleAndComputeWeightedMeanValues( period );
    }

    return { resampler.resampledTimeSteps(), resampler.resampledValues() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RiaSummaryTools::summaryCaseById( int caseId )
{
    auto summaryCases = RimProject::current()->allSummaryCases();

    for ( auto summaryCase : summaryCases )
    {
        if ( summaryCase->caseId() == caseId )
        {
            return summaryCase;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RiaSummaryTools::ensembleById( int ensembleId )
{
    auto ensembles = RimProject::current()->summaryEnsembles();

    for ( auto ensemble : ensembles )
    {
        if ( ensemble->ensembleId() == ensembleId )
        {
            return ensemble;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaSummaryTools::optionsForAllSummaryCases()
{
    bool includeEnsembleName = true;
    return optionsForSummaryCases( RimProject::current()->allSummaryCases(), includeEnsembleName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaSummaryTools::optionsForSummaryCases( const std::vector<RimSummaryCase*>& cases, bool includeEnsembleName )
{
    QList<caf::PdmOptionItemInfo> options;

    for ( RimSummaryCase* c : cases )
    {
        QString displayName = c->displayCaseName();
        if ( includeEnsembleName && c->ensemble() )
        {
            QString ensembleName = c->ensemble()->name();
            if ( !ensembleName.isEmpty() )
            {
                displayName = ensembleName + " : " + displayName;
            }
        }

        options.push_back( caf::PdmOptionItemInfo( displayName, c, false, c->uiIconProvider() ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryTools::copyCurveDataSources( RimSummaryCurve& curve, const RimSummaryCurve& otherCurve )
{
    curve.setSummaryAddressX( otherCurve.summaryAddressX() );
    curve.setSummaryCaseX( otherCurve.summaryCaseX() );

    curve.setSummaryAddressY( otherCurve.summaryAddressY() );
    curve.setSummaryCaseY( otherCurve.summaryCaseY() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryTools::copyCurveAxisData( RimSummaryCurve& curve, const RimSummaryCurve& otherCurve )
{
    curve.setAxisTypeX( otherCurve.axisTypeX() );
    curve.setTopOrBottomAxisX( otherCurve.axisX() );

    curve.setLeftOrRightAxisY( otherCurve.axisY() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryTools::updateRequiredCalculatedCurves( RimSummaryCase* sourceSummaryCase )
{
    RimSummaryCalculationCollection* calcColl = RimProject::current()->calculationCollection();

    for ( RimUserDefinedCalculation* summaryCalculation : calcColl->calculations() )
    {
        bool needsUpdate = RiaSummaryTools::isCalculationRequired( summaryCalculation, sourceSummaryCase );
        if ( needsUpdate )
        {
            summaryCalculation->parseExpression();
            summaryCalculation->calculate();
            summaryCalculation->updateDependentObjects();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSummaryTools::isCalculationRequired( const RimUserDefinedCalculation* summaryCalculation, const RimSummaryCase* summaryCase )
{
    std::vector<RimUserDefinedCalculationVariable*> variables = summaryCalculation->allVariables();
    for ( RimUserDefinedCalculationVariable* variable : variables )
    {
        if ( auto* summaryVariable = dynamic_cast<RimSummaryCalculationVariable*>( variable ) )
        {
            return summaryVariable->summaryCase() == summaryCase;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryTools::reloadSummaryCaseAndUpdateConnectedPlots( RimSummaryCase* summaryCase, bool createAddressObjects )
{
    if ( !summaryCase ) return;

    summaryCase->updateAutoShortName();

    // Recreate the reader interface, but do not create address objects yet. For ensembles, we do not want to create addresses for all cases
    summaryCase->createSummaryReaderInterface();

    if ( createAddressObjects )
    {
        if ( auto reader = summaryCase->summaryReader() )
        {
            reader->createAddressesIfRequired();
        }
    }
    summaryCase->createRftReaderInterface();
    summaryCase->refreshMetaData();

    RiaSummaryTools::updateRequiredCalculatedCurves( summaryCase );

    RimSummaryMultiPlotCollection* summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection();
    for ( RimSummaryMultiPlot* multiPlot : summaryPlotColl->multiPlots() )
    {
        for ( RimSummaryPlot* summaryPlot : multiPlot->summaryPlots() )
        {
            summaryPlot->loadDataAndUpdate();

            // Consider to make the zoom optional
            summaryPlot->zoomAll();
        }

        multiPlot->updatePlotTitles();
    }

    auto depthTrackPlots = caf::PdmObjectHandleTools::referringAncestorOfType<RimDepthTrackPlot, RimSummaryCase>( { summaryCase } );
    RimWellPlotTools::loadDataAndUpdateDepthTrackPlots( depthTrackPlots );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryTools::updateConnectedPlots( RimSummaryEnsemble* ensemble )
{
    RimSummaryMultiPlotCollection* summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection();
    for ( RimSummaryMultiPlot* multiPlot : summaryPlotColl->multiPlots() )
    {
        for ( RimSummaryPlot* summaryPlot : multiPlot->summaryPlots() )
        {
            summaryPlot->loadDataAndUpdate();

            // Consider to make the zoom optional
            summaryPlot->zoomAll();
        }

        multiPlot->updatePlotTitles();
    }

    auto depthTrackPlots = caf::PdmObjectHandleTools::referringAncestorOfType<RimDepthTrackPlot, RimSummaryEnsemble>( { ensemble } );
    RimWellPlotTools::loadDataAndUpdateDepthTrackPlots( depthTrackPlots );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
time_t RiaSummaryTools::calculateTimeThreshold( const time_t& minimum, const time_t& maximum )
{
    // The cutoff time is the time where the time step is less than 1% of the total time range
    const auto epsilon       = 0.01;
    const auto timeThreshold = maximum - ( maximum - minimum ) * epsilon;

    return timeThreshold;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryTools::updateSummaryEnsembleNames()
{
    if ( auto sumCaseMainColl = RiaSummaryTools::summaryCaseMainCollection() )
    {
        sumCaseMainColl->updateEnsembleNames();
    }
}
