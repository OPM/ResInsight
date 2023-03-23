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

#include "RiaSummaryTools.h"

#include "RiaFilePathTools.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RifEclipseSummaryAddress.h"

#include "RimMainPlotCollection.h"
#include "RimObservedDataCollection.h"
#include "RimOilField.h"
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
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTable.h"
#include "RimSummaryTableCollection.h"

#include "cafPdmObject.h"

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
RimSummaryCrossPlotCollection* RiaSummaryTools::summaryCrossPlotCollection()
{
    return RimMainPlotCollection::current()->summaryCrossPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection* RiaSummaryTools::summaryCaseMainCollection()
{
    RimProject*                   project                   = RimProject::current();
    RimSummaryCaseMainCollection* summaryCaseMainCollection = project->activeOilField()->summaryCaseMainCollection();
    CVF_ASSERT( summaryCaseMainCollection );
    return summaryCaseMainCollection;
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
                    curve->setSummaryAddressYAndApplyInterpolation( adr );
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
    if ( parentCrossPlot( object ) )
    {
        return nullptr;
    }

    RimSummaryPlot* summaryPlot = nullptr;

    if ( object )
    {
        object->firstAncestorOrThisOfType( summaryPlot );
    }

    return summaryPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlotCollection* RiaSummaryTools::parentSummaryPlotCollection( caf::PdmObject* object )
{
    RimSummaryMultiPlotCollection* summaryPlotColl = nullptr;

    if ( object )
    {
        object->firstAncestorOrThisOfType( summaryPlotColl );
    }

    return summaryPlotColl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RiaSummaryTools::parentSummaryMultiPlot( caf::PdmObject* object )
{
    RimSummaryMultiPlot* multiPlot = nullptr;

    if ( object )
    {
        object->firstAncestorOrThisOfType( multiPlot );
    }

    return multiPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlot* RiaSummaryTools::parentCrossPlot( caf::PdmObject* object )
{
    RimSummaryCrossPlot* crossPlot = nullptr;

    if ( object )
    {
        object->firstAncestorOrThisOfType( crossPlot );
    }

    return crossPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlotCollection* RiaSummaryTools::parentCrossPlotCollection( caf::PdmObject* object )
{
    RimSummaryCrossPlotCollection* crossPlotColl = nullptr;

    if ( object )
    {
        object->firstAncestorOrThisOfType( crossPlotColl );
    }

    return crossPlotColl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSummaryTools::isSummaryCrossPlot( const RimSummaryPlot* plot )
{
    return dynamic_cast<const RimSummaryCrossPlot*>( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTable* RiaSummaryTools::parentSummaryTable( caf::PdmObject* object )
{
    RimSummaryTable* summaryTable = nullptr;

    if ( object )
    {
        object->firstAncestorOrThisOfType( summaryTable );
    }

    return summaryTable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTableCollection* RiaSummaryTools::parentSummaryTableCollection( caf::PdmObject* object )
{
    RimSummaryTableCollection* summaryTableColl = nullptr;

    if ( object )
    {
        object->firstAncestorOrThisOfType( summaryTableColl );
    }

    return summaryTableColl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSummaryTools::hasAccumulatedData( const RifEclipseSummaryAddress& address )
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
                return false;
            }
        }

        // All the variables are accumulated
        return true;
    }

    return address.hasAccumulatedData();
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
        RimSummaryCalculationVariable* scv = dynamic_cast<RimSummaryCalculationVariable*>( v );
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
    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData( values, timeSteps );

    if ( RiaSummaryTools::hasAccumulatedData( address ) )
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
RimSummaryCaseCollection* RiaSummaryTools::ensembleById( int ensembleId )
{
    auto ensembles = RimProject::current()->summaryGroups();

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
    return optionsForSummaryCases( RimProject::current()->allSummaryCases() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaSummaryTools::optionsForSummaryCases( const std::vector<RimSummaryCase*>& cases )
{
    QList<caf::PdmOptionItemInfo> options;

    for ( RimSummaryCase* c : cases )
    {
        options.push_back( caf::PdmOptionItemInfo( c->displayCaseName(), c, false, c->uiIconProvider() ) );
    }

    return options;
}
