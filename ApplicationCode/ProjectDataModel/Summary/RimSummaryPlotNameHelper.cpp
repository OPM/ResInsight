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

#include "RifEclipseSummaryAddress.h"

#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "RiuSummaryQuantityNameInfoProvider.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotNameHelper::RimSummaryPlotNameHelper()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::clear()
{
    m_summaryCases.clear();
    m_ensembleCases.clear();
    m_analyzer.clear();

    clearTitleSubStrings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::appendAddresses( const std::vector<RifEclipseSummaryAddress>& addresses )
{
    m_analyzer.appendAddresses( addresses );

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
void RimSummaryPlotNameHelper::setEnsembleCases( const std::vector<RimSummaryCaseCollection*>& ensembleCases )
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
    QString title;

    if ( !m_titleCaseName.isEmpty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += m_titleCaseName;
    }

    if ( !m_titleWellName.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += QString::fromStdString( m_titleWellName );
    }

    if ( !m_titleWellGroupName.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += QString::fromStdString( m_titleWellGroupName );
    }

    if ( !m_titleRegion.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Region : " + QString::fromStdString( m_titleRegion );
    }

    if ( !m_titleBlock.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Block : " + QString::fromStdString( m_titleBlock );
    }

    if ( !m_titleSegment.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Segment : " + QString::fromStdString( m_titleSegment );
    }

    if ( !m_titleCompletion.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Completion : " + QString::fromStdString( m_titleCompletion );
    }

    if ( !m_titleQuantity.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += QString::fromStdString(
            RiuSummaryQuantityNameInfoProvider::instance()->longNameFromQuantityName( m_titleQuantity, true ) );
    }

    if ( title.isEmpty() )
    {
        title = "Composed Plot";
    }

    return title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isPlotDisplayingSingleQuantity() const
{
    return m_analyzer.quantities().size() == 1;
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
bool RimSummaryPlotNameHelper::isWellGroupNameInTitle() const
{
    return !m_titleWellGroupName.empty();
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
void RimSummaryPlotNameHelper::clearTitleSubStrings()
{
    m_titleQuantity.clear();
    m_titleRegion.clear();
    m_titleWellName.clear();
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

    auto wellNames      = m_analyzer.wellNames();
    auto wellGroupNames = m_analyzer.wellGroupNames();
    auto regions        = m_analyzer.regionNumbers();
    auto blocks         = m_analyzer.blocks();
    auto categories     = m_analyzer.categories();

    if ( categories.size() == 1 )
    {
        m_titleQuantity = m_analyzer.quantityNameForTitle();

        if ( wellNames.size() == 1 )
        {
            m_titleWellName = *( wellNames.begin() );

            {
                auto segments = m_analyzer.wellSegmentNumbers( m_titleWellName );
                if ( segments.size() == 1 )
                {
                    m_titleSegment = std::to_string( *( segments.begin() ) );
                }
            }

            {
                auto completions = m_analyzer.wellCompletions( m_titleWellName );
                if ( completions.size() == 1 )
                {
                    m_titleCompletion = *( completions.begin() );
                }
            }
        }

        if ( wellGroupNames.size() == 1 )
        {
            m_titleWellGroupName = *( wellGroupNames.begin() );
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
std::set<RimSummaryCaseCollection*> RimSummaryPlotNameHelper::setOfEnsembleCases() const
{
    std::set<RimSummaryCaseCollection*> ensembleCases;

    for ( const auto& ensemble : m_ensembleCases )
    {
        if ( ensemble ) ensembleCases.insert( ensemble );
    }

    return ensembleCases;
}
