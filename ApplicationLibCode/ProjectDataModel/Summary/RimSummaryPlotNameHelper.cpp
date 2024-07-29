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

#include "RimSummaryPlotNameHelper.h"

#include "RiaSummaryAddressAnalyzer.h"
#include "RifEclipseSummaryAddress.h"

#include "RimObjectiveFunctionTools.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotNameHelper::RimSummaryPlotNameHelper()
{
    m_analyzer = std::make_unique<RiaSummaryAddressAnalyzer>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::clear()
{
    m_summaryCases.clear();
    m_ensembleCases.clear();
    m_analyzer->clear();

    clearTitleSubStrings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::appendAddresses( const std::vector<RiaSummaryCurveAddress>& addresses )
{
    m_analyzer->appendAddresses( addresses );

    extractPlotTitleSubStrings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::setSummaryCases( const std::vector<RimSummaryCase*>& summaryCases )
{
    m_summaryCases.clear();

    m_summaryCases.resize( summaryCases.size() );

    for ( size_t i = 0; i < summaryCases.size(); i++ )
    {
        m_summaryCases[i] = summaryCases[i];
    }

    extractPlotTitleSubStrings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::setEnsembleCases( const std::vector<RimSummaryEnsemble*>& ensembleCases )
{
    m_ensembleCases.clear();

    m_ensembleCases.resize( ensembleCases.size() );

    for ( size_t i = 0; i < ensembleCases.size(); i++ )
    {
        m_ensembleCases[i] = ensembleCases[i];
    }

    extractPlotTitleSubStrings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotNameHelper::plotTitle() const
{
    RimSummaryPlotNameHelper empty;

    return aggregatedPlotTitle( empty );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isPlotDisplayingSingleCurve() const
{
    if ( m_analyzer->quantities().size() == 2 )
    {
        if ( m_analyzer->onlyCrossPlotCurves() )
        {
            // We have cross plot curves, and two quantities. This means that we have one curve.
            return true;
        }

        std::vector<std::string> strings;
        for ( const auto& q : m_analyzer->quantities() )
            strings.push_back( q );

        auto first  = RimObjectiveFunctionTools::nativeQuantityName( strings[0] );
        auto second = RimObjectiveFunctionTools::nativeQuantityName( strings[1] );

        // We have two quantities, one summary vector and one corresponding history vector.
        if ( first == second ) return true;
    }

    return m_analyzer->quantities().size() == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isWellNameInTitle() const
{
    return !m_titleWellName.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isGroupNameInTitle() const
{
    return !m_titleGroupName.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isNetworkInTitle() const
{
    return !m_titleNetwork.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isRegionInTitle() const
{
    return !m_titleRegion.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isCaseInTitle() const
{
    return !m_titleCaseName.isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isBlockInTitle() const
{
    return !m_titleBlock.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isSegmentInTitle() const
{
    return !m_titleSegment.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isCompletionInTitle() const
{
    return !m_titleCompletion.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RimSummaryPlotNameHelper::vectorNames() const
{
    return m_analyzer->quantities();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotNameHelper::caseName() const
{
    return m_titleCaseName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryPlotNameHelper::titleVectorName() const
{
    return m_titleQuantity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryPlotNameHelper::titleWellName() const
{
    return m_titleWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryPlotNameHelper::titleGroupName() const
{
    return m_titleGroupName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryPlotNameHelper::titleNetwork() const
{
    return m_titleNetwork;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryPlotNameHelper::titleRegion() const
{
    return m_titleRegion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryPlotNameHelper::titleBlock() const
{
    return m_titleBlock;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryPlotNameHelper::titleSegment() const
{
    return m_titleSegment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryPlotNameHelper::titleCompletion() const
{
    return m_titleCompletion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryPlotNameHelper::numberOfCases() const
{
    return m_summaryCases.size() + m_ensembleCases.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::clearTitleSubStrings()
{
    m_titleQuantity.clear();
    m_titleWellName.clear();
    m_titleGroupName.clear();
    m_titleNetwork.clear();
    m_titleRegion.clear();
    m_titleBlock.clear();
    m_titleSegment.clear();
    m_titleCompletion.clear();

    m_titleCaseName.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::extractPlotTitleSubStrings()
{
    clearTitleSubStrings();

    auto wellNames  = m_analyzer->wellNames();
    auto groupNames = m_analyzer->groupNames();
    auto networks   = m_analyzer->networkNames();
    auto regions    = m_analyzer->regionNumbers();
    auto blocks     = m_analyzer->blocks();
    auto categories = m_analyzer->categories();

    if ( categories.size() == 1 )
    {
        m_titleQuantity = m_analyzer->quantityNameForTitle();

        if ( wellNames.size() == 1 )
        {
            m_titleWellName = *( wellNames.begin() );

            {
                auto segments = m_analyzer->wellSegmentNumbers( m_titleWellName );
                if ( segments.size() == 1 )
                {
                    m_titleSegment = std::to_string( *( segments.begin() ) );
                }
            }

            {
                auto completions = m_analyzer->wellCompletions( m_titleWellName );
                if ( completions.size() == 1 )
                {
                    m_titleCompletion = *( completions.begin() );
                }
            }
        }

        if ( groupNames.size() == 1 )
        {
            m_titleGroupName = *( groupNames.begin() );
        }

        if ( networks.size() == 1 )
        {
            m_titleNetwork = *( networks.begin() );
        }

        if ( regions.size() == 1 )
        {
            m_titleRegion = std::to_string( *( regions.begin() ) );
        }

        if ( blocks.size() == 1 )
        {
            m_titleBlock = *( blocks.begin() );
        }
    }

    auto summaryCases  = setOfSummaryCases();
    auto ensembleCases = setOfEnsembleCases();

    if ( summaryCases.size() == 1 && ensembleCases.empty() )
    {
        auto summaryCase = *( summaryCases.begin() );

        if ( summaryCase )
        {
            m_titleCaseName = summaryCase->displayCaseName();
        }
    }
    else if ( ensembleCases.size() == 1 && summaryCases.empty() )
    {
        auto ensembleCase = *( ensembleCases.begin() );
        if ( ensembleCase )
        {
            m_titleCaseName = ensembleCase->name();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RimSummaryPlotNameHelper::setOfSummaryCases() const
{
    std::set<RimSummaryCase*> summaryCases;

    for ( const auto& sumCase : m_summaryCases )
    {
        if ( sumCase ) summaryCases.insert( sumCase );
    }

    return summaryCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryEnsemble*> RimSummaryPlotNameHelper::setOfEnsembleCases() const
{
    std::set<RimSummaryEnsemble*> ensembleCases;

    for ( const auto& ensemble : m_ensembleCases )
    {
        if ( ensemble ) ensembleCases.insert( ensemble );
    }

    return ensembleCases;
}
