/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RimSummaryEnsembleTools.h"

#include "Summary/RiaSummaryTools.h"

#include "RifReaderRftInterface.h"
#include "RifSummaryReaderInterface.h"

#include "RigEnsembleParameter.h"

#include "RimCorrelationPlotCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimMainPlotCollection.h"
#include "RimParameterResultCrossPlot.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiTreeView.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimSummaryEnsembleTools::wellsWithRftData( const std::vector<RimSummaryCase*>& summaryCases )
{
    std::set<QString> allWellNames;
    for ( auto summaryCase : summaryCases )
    {
        if ( auto reader = summaryCase->rftReader() )
        {
            std::set<QString> wellNames = reader->wellNames();
            allWellNames.insert( wellNames.begin(), wellNames.end() );
        }
    }
    return allWellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEnsembleParameter RimSummaryEnsembleTools::createEnsembleParameter( const std::vector<RimSummaryCase*>& summaryCases,
                                                                       const QString&                      paramName )
{
    RigEnsembleParameter eParam;
    eParam.name = paramName;

    size_t numericValuesCount = 0;
    size_t textValuesCount    = 0;

    // Make sure the values list exactly matches the case count
    // And use an invalid value (infinity) for invalid cases.
    eParam.values.resize( summaryCases.size(), std::numeric_limits<double>::infinity() );

    // Prepare case realization params, and check types
    for ( size_t caseIdx = 0; caseIdx < summaryCases.size(); ++caseIdx )
    {
        auto rimCase = summaryCases[caseIdx];

        auto crp = rimCase->caseRealizationParameters();
        if ( !crp ) continue;

        auto value = crp->parameterValue( paramName );
        if ( !value.isValid() ) continue;

        if ( value.isNumeric() )
        {
            double numVal          = value.numericValue();
            eParam.values[caseIdx] = QVariant( numVal );
            if ( numVal < eParam.minValue ) eParam.minValue = numVal;
            if ( numVal > eParam.maxValue ) eParam.maxValue = numVal;
            numericValuesCount++;
        }
        else if ( value.isText() )
        {
            eParam.values[caseIdx] = QVariant( value.textValue() );
            textValuesCount++;
        }
    }

    if ( numericValuesCount && !textValuesCount )
    {
        eParam.type = RigEnsembleParameter::TYPE_NUMERIC;
    }
    else if ( textValuesCount && !numericValuesCount )
    {
        eParam.type = RigEnsembleParameter::TYPE_TEXT;
    }
    if ( numericValuesCount && textValuesCount )
    {
        // A mix of types have been added to parameter values
        if ( numericValuesCount > textValuesCount )
        {
            // Use numeric type
            for ( auto& val : eParam.values )
            {
                if ( val.type() == QVariant::String )
                {
                    val.setValue( std::numeric_limits<double>::infinity() );
                }
            }
            eParam.type = RigEnsembleParameter::TYPE_NUMERIC;
        }
        else
        {
            // Use text type
            for ( auto& val : eParam.values )
            {
                if ( val.type() == QVariant::Double )
                {
                    val.setValue( QString::number( val.value<double>() ) );
                }
            }
            eParam.type     = RigEnsembleParameter::TYPE_TEXT;
            eParam.minValue = std::numeric_limits<double>::infinity();
            eParam.maxValue = -std::numeric_limits<double>::infinity();
        }
    }

    if ( eParam.isText() )
    {
        // Remove duplicate texts
        std::set<QString> valueSet;
        for ( const auto& val : eParam.values )
        {
            valueSet.insert( val.toString() );
        }
        eParam.values.clear();
        for ( const auto& val : valueSet )
        {
            eParam.values.push_back( QVariant( val ) );
        }
    }

    return eParam;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleTools::sortByBinnedVariation( std::vector<RigEnsembleParameter>& parameterVector )
{
    double minStdDev = std::numeric_limits<double>::infinity();
    double maxStdDev = 0.0;
    for ( const auto& paramPair : parameterVector )
    {
        double stdDev = paramPair.normalizedStdDeviation();
        if ( stdDev != 0.0 )
        {
            minStdDev = std::min( minStdDev, stdDev );
            maxStdDev = std::max( maxStdDev, stdDev );
        }
    }
    if ( ( maxStdDev - minStdDev ) <= 0.0 )
    {
        return;
    }

    double delta = ( maxStdDev - minStdDev ) / (float)( RigEnsembleParameter::NR_OF_VARIATION_BINS );

    std::vector<double> bins;
    bins.push_back( 0.0 );
    for ( int i = 0; i < RigEnsembleParameter::NR_OF_VARIATION_BINS - 1; ++i )
    {
        bins.push_back( minStdDev + ( i + 1 ) * delta );
    }

    for ( RigEnsembleParameter& nameParamPair : parameterVector )
    {
        int binNumber = -1;
        for ( double bin : bins )
        {
            if ( nameParamPair.normalizedStdDeviation() > bin )
            {
                binNumber++;
            }
        }
        nameParamPair.variationBin = binNumber;
    }

    // Sort by variation bin (highest first) but keep name as sorting parameter when parameters have the same variation
    // index
    std::stable_sort( parameterVector.begin(),
                      parameterVector.end(),
                      []( const RigEnsembleParameter& lhs, const RigEnsembleParameter& rhs ) { return lhs.variationBin > rhs.variationBin; } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEnsembleParameter> RimSummaryEnsembleTools::alphabeticEnsembleParameters( const std::vector<RimSummaryCase*>& summaryCases )
{
    std::set<QString> paramSet;
    for ( RimSummaryCase* rimCase : summaryCases )
    {
        if ( rimCase->caseRealizationParameters() != nullptr )
        {
            auto ps = rimCase->caseRealizationParameters()->parameters();
            for ( auto p : ps )
            {
                paramSet.insert( p.first );
            }
        }
    }

    std::vector<RigEnsembleParameter> sortedEnsembleParameters;
    sortedEnsembleParameters.reserve( paramSet.size() );
    for ( const QString& parameterName : paramSet )
    {
        sortedEnsembleParameters.push_back( RimSummaryEnsembleTools::createEnsembleParameter( summaryCases, parameterName ) );
    }
    return sortedEnsembleParameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEnsembleParameter>
    RimSummaryEnsembleTools::createVariationSortedEnsembleParameters( const std::vector<RimSummaryCase*>& summaryCases )
{
    std::set<QString> paramSet;
    for ( RimSummaryCase* rimCase : summaryCases )
    {
        if ( rimCase->caseRealizationParameters() != nullptr )
        {
            auto ps = rimCase->caseRealizationParameters()->parameters();
            for ( const auto& p : ps )
            {
                paramSet.insert( p.first );
            }
        }
    }
    std::vector<RigEnsembleParameter> parameters;
    parameters.reserve( paramSet.size() );
    for ( const QString& parameterName : paramSet )
    {
        auto ensembleParameter = RimSummaryEnsembleTools::createEnsembleParameter( summaryCases, parameterName );
        parameters.push_back( ensembleParameter );
    }
    RimSummaryEnsembleTools::sortByBinnedVariation( parameters );

    return parameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void clearEnsembleParametersHashes( const std::vector<RimSummaryCase*>& summaryCases )
{
    for ( auto sumCase : summaryCases )
    {
        auto crp = sumCase->caseRealizationParameters();
        if ( crp ) crp->clearParametersHash();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryEnsembleTools::calculateEnsembleParametersIntersectionHash( const std::vector<RimSummaryCase*>& summaryCases )
{
    clearEnsembleParametersHashes( summaryCases );

    // Find ensemble parameters intersection
    std::set<QString> paramNames;

    for ( size_t i = 0; i < summaryCases.size(); i++ )
    {
        auto crp = summaryCases[i]->caseRealizationParameters();
        if ( !crp ) continue;

        auto caseParamNames = crp->parameterNames();

        if ( i == 0 )
            paramNames = caseParamNames;
        else
        {
            std::set<QString> newIntersection;
            std::set_intersection( paramNames.begin(),
                                   paramNames.end(),
                                   caseParamNames.begin(),
                                   caseParamNames.end(),
                                   std::inserter( newIntersection, newIntersection.end() ) );

            if ( paramNames.size() != newIntersection.size() ) paramNames = newIntersection;
        }
    }

    for ( auto sumCase : summaryCases )
    {
        auto crp = sumCase->caseRealizationParameters();
        if ( crp ) crp->calculateParametersHash( paramNames );
    }

    size_t commonAddressCount = 0;

    // Find common address count
    for ( const auto sumCase : summaryCases )
    {
        const auto reader = sumCase->summaryReader();
        if ( !reader ) continue;
        auto currAddrCount = reader->allResultAddresses().size();

        if ( commonAddressCount == 0 )
        {
            commonAddressCount = currAddrCount;
        }
        else
        {
            if ( currAddrCount != commonAddressCount )
            {
                commonAddressCount = 0;
                break;
            }
        }
    }

    return commonAddressCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryEnsembleTools::isEnsembleCurve( RimPlotCurve* sourceCurve )
{
    auto summaryCurve = dynamic_cast<RimSummaryCurve*>( sourceCurve );
    if ( !summaryCurve ) return false;

    return summaryCurve->isEnsembleCurve();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryEnsembleTools::summaryCasesFromCurves( const std::vector<RimPlotCurve*>& sourceCurves )
{
    std::vector<RimSummaryCase*> sourceCases;

    for ( auto sourceCurve : sourceCurves )
    {
        auto sourceSummaryCurve = dynamic_cast<RimSummaryCurve*>( sourceCurve );
        if ( !sourceSummaryCurve ) continue;

        auto ensembleCurveSet = sourceSummaryCurve->firstAncestorOfType<RimEnsembleCurveSet>();
        if ( !ensembleCurveSet ) continue;

        auto sourceCase = sourceSummaryCurve->summaryCaseY();
        if ( !sourceCase ) continue;

        sourceCases.push_back( sourceCase );
    }

    return sourceCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleTools::selectSummaryCasesInProjectTree( const std::vector<RimSummaryCase*>& sourceCases )
{
    if ( sourceCases.empty() ) return;

    if ( auto mainWindow = RiuPlotMainWindow::instance() )
    {
        if ( auto treeView = mainWindow->getTreeViewWithItem( sourceCases.front() ) )
        {
            std::vector<const caf::PdmUiItem*> sourceItems;
            for ( auto sourceCase : sourceCases )
            {
                sourceItems.push_back( sourceCase );
            }

            treeView->selectItems( sourceItems );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleTools::highlightCurvesForSummaryCases( const std::vector<RimSummaryCase*>& sourceCases )
{
    auto summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection();

    for ( auto multiPlot : summaryPlotColl->multiPlots() )
    {
        for ( auto plot : multiPlot->summaryPlots() )
        {
            auto plotWidget = dynamic_cast<RiuQwtPlotWidget*>( plot->plotWidget() );
            if ( !plotWidget ) continue;

            // Do nothing if no curve sets are present, as this will cause the single curve highlight to be removed
            if ( plot->curveSets().empty() ) continue;

            std::vector<RimPlotCurve*> curvesForSameRealization;
            for ( const auto& curveSet : plot->curveSets() )
            {
                for ( auto ensembleCurve : curveSet->curves() )
                {
                    auto summaryCaseY = ensembleCurve->summaryCaseY();
                    if ( std::find( sourceCases.begin(), sourceCases.end(), summaryCaseY ) != sourceCases.end() )
                    {
                        curvesForSameRealization.push_back( ensembleCurve );
                    }
                }
            }

            bool updateCurveOrder = false;
            plotWidget->resetPlotItemHighlighting( updateCurveOrder );

            if ( !curvesForSameRealization.empty() )
            {
                std::sort( curvesForSameRealization.begin(), curvesForSameRealization.end() );
                curvesForSameRealization.erase( std::unique( curvesForSameRealization.begin(), curvesForSameRealization.end() ),
                                                curvesForSameRealization.end() );

                plotWidget->highlightCurvesUpdateOrder( curvesForSameRealization );
            }

            plotWidget->replot();
        }
    }

    auto plotColl = RimMainPlotCollection::current();
    if ( auto correlationPlotColl = plotColl->correlationPlotCollection() )
    {
        auto crossPlots = correlationPlotColl->descendantsIncludingThisOfType<RimParameterResultCrossPlot>();
        for ( auto c : crossPlots )
        {
            c->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleTools::resetHighlightAllPlots()
{
    auto summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection();

    for ( auto multiPlot : summaryPlotColl->multiPlots() )
    {
        for ( auto plot : multiPlot->summaryPlots() )
        {
            if ( auto plotWidget = dynamic_cast<RiuQwtPlotWidget*>( plot->plotWidget() ) )
            {
                plotWidget->resetPlotItemHighlighting();
                plotWidget->replot();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryEnsembleTools::caseWithMostKeywords( const std::vector<RimSummaryCase*>& sourceCases )
{
    int maxAddrCount = 0;
    int maxAddrIndex = -1;

    for ( int i = 0; i < (int)sourceCases.size(); i++ )
    {
        RimSummaryCase* currCase = sourceCases[i];
        if ( !currCase ) continue;

        RifSummaryReaderInterface* reader = currCase->summaryReader();
        if ( !reader ) continue;

        auto addrCount = reader->keywordCount();
        if ( addrCount > maxAddrCount )
        {
            maxAddrCount = addrCount;
            maxAddrIndex = i;
        }
    }

    if ( maxAddrIndex < 0 )
    {
        return nullptr;
    }

    return sourceCases[maxAddrIndex];
}
