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
RimMultiSummaryPlotNameHelper::RimMultiSummaryPlotNameHelper( std::vector<const RimSummaryPlotNameHelperInterface*> nameHelpers )
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
bool RimMultiSummaryPlotNameHelper::isPlotDisplayingSingleQuantity() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isPlotDisplayingSingleQuantity() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isWellNameInTitle() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isWellNameInTitle() ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isWellGroupNameInTitle() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isWellGroupNameInTitle() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isRegionInTitle() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isRegionInTitle() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isCaseInTitle() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isCaseInTitle() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isBlockInTitle() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isBlockInTitle() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isSegmentInTitle() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isSegmentInTitle() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMultiSummaryPlotNameHelper::isCompletionInTitle() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isCompletionInTitle() ) return true;
    }

    return false;
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
std::string RimMultiSummaryPlotNameHelper::titleQuantity() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isPlotDisplayingSingleQuantity() ) return nameHelper->titleQuantity();
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
std::string RimMultiSummaryPlotNameHelper::titleWellGroupName() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isWellGroupNameInTitle() ) return nameHelper->titleWellGroupName();
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
        if ( nameHelper->isBlockInTitle() ) return nameHelper->titleBlock();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimMultiSummaryPlotNameHelper::titleCompletion() const
{
    for ( auto nameHelper : m_nameHelpers )
    {
        if ( nameHelper->isCompletionInTitle() ) return nameHelper->titleCompletion();
    }

    return "";
}
