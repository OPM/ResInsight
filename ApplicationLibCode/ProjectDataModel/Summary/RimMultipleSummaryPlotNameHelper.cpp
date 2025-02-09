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

#include "RimMultipleSummaryPlotNameHelper.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiSummaryPlotNameHelper::RimMultiSummaryPlotNameHelper( std::vector<const RimSummaryNameHelper*> nameHelpers )
    : m_nameHelpers( nameHelpers )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMultiSummaryPlotNameHelper::plotTitle() const
{
    if ( m_nameHelpers.size() == 1 ) return m_nameHelpers.front()->plotTitle();

    if ( m_nameHelpers.size() == 2 )
    {
        auto first  = m_nameHelpers[0];
        auto second = m_nameHelpers[1];

        return first->aggregatedPlotTitle( *second );
    }

    return "Plot Title";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isPlotDisplayingSingleCurve() const
{
    int plotCountWithSingleQuantity = 0;
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isPlotDisplayingSingleCurve() ) plotCountWithSingleQuantity++;
    }

    return plotCountWithSingleQuantity == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isWellNameInTitle() const
{
    return std::any_of( m_nameHelpers.begin(), m_nameHelpers.end(), []( auto nameHelper ) { return nameHelper->isWellNameInTitle(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isGroupNameInTitle() const
{
    return std::any_of( m_nameHelpers.begin(), m_nameHelpers.end(), []( auto nameHelper ) { return nameHelper->isGroupNameInTitle(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isNetworkInTitle() const
{
    return std::any_of( m_nameHelpers.begin(), m_nameHelpers.end(), []( auto nameHelper ) { return nameHelper->isNetworkInTitle(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isRegionInTitle() const
{
    return std::any_of( m_nameHelpers.begin(), m_nameHelpers.end(), []( auto nameHelper ) { return nameHelper->isRegionInTitle(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isCaseInTitle() const
{
    return std::any_of( m_nameHelpers.begin(), m_nameHelpers.end(), []( auto nameHelper ) { return nameHelper->isCaseInTitle(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isBlockInTitle() const
{
    return std::any_of( m_nameHelpers.begin(), m_nameHelpers.end(), []( auto nameHelper ) { return nameHelper->isBlockInTitle(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isSegmentInTitle() const
{
    return std::any_of( m_nameHelpers.begin(), m_nameHelpers.end(), []( auto nameHelper ) { return nameHelper->isSegmentInTitle(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isConnectionInTitle() const
{
    return std::any_of( m_nameHelpers.begin(), m_nameHelpers.end(), []( auto nameHelper ) { return nameHelper->isConnectionInTitle(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RimMultiSummaryPlotNameHelper::vectorNames() const
{
    std::vector<std::string> allNames;

    for ( auto nameHelper : m_nameHelpers )
    {
        auto nameHelperVectorNames = nameHelper->vectorNames();
        allNames.insert( allNames.end(), nameHelperVectorNames.begin(), nameHelperVectorNames.end() );
    }

    return allNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMultiSummaryPlotNameHelper::caseName() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isCaseInTitle() ) return nameHelper->caseName();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimMultiSummaryPlotNameHelper::titleVectorName() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isPlotDisplayingSingleCurve() ) return nameHelper->titleVectorName();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimMultiSummaryPlotNameHelper::titleWellName() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isWellNameInTitle() ) return nameHelper->titleWellName();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimMultiSummaryPlotNameHelper::titleGroupName() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isGroupNameInTitle() ) return nameHelper->titleGroupName();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimMultiSummaryPlotNameHelper::titleNetwork() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isNetworkInTitle() ) return nameHelper->titleNetwork();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimMultiSummaryPlotNameHelper::titleRegion() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isRegionInTitle() ) return nameHelper->titleRegion();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimMultiSummaryPlotNameHelper::titleBlock() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isBlockInTitle() ) return nameHelper->titleBlock();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimMultiSummaryPlotNameHelper::titleSegment() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isSegmentInTitle() ) return nameHelper->titleSegment();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimMultiSummaryPlotNameHelper::titleConnection() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isConnectionInTitle() ) return nameHelper->titleConnection();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimMultiSummaryPlotNameHelper::numberOfCases() const
{
    size_t caseCount = 0;
    for ( auto nameHelper : m_nameHelpers )
    {
        caseCount += nameHelper->numberOfCases();
    }

    return caseCount;
}
