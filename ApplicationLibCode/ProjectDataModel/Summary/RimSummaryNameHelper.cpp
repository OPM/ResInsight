/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimSummaryNameHelper.h"

#include "RiuSummaryQuantityNameInfoProvider.h"
#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryNameHelper::aggregatedPlotTitle( const RimSummaryNameHelper& other ) const
{
    QString title;

    auto titleCaseName = this->caseName();
    if ( !other.isCaseInTitle() && !titleCaseName.isEmpty() )
    {
        if ( !title.isEmpty() ) title += ", ";

        title += titleCaseName;
    }

    auto wellName = this->titleWellName();
    if ( !other.isWellNameInTitle() && !wellName.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += QString::fromStdString( wellName );
    }

    auto groupName = this->titleGroupName();
    if ( !other.isGroupNameInTitle() && !groupName.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += QString::fromStdString( groupName );
    }

    auto region = this->titleRegion();
    if ( !other.isRegionInTitle() && !region.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Region : " + QString::fromStdString( region );
    }

    auto block = this->titleBlock();
    if ( !other.isBlockInTitle() && !block.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Block : " + QString::fromStdString( block );
    }

    auto segment = this->titleSegment();
    if ( !other.isSegmentInTitle() && !segment.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Segment : " + QString::fromStdString( segment );
    }

    auto completion = this->titleCompletion();
    if ( !other.isCompletionInTitle() && !completion.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Completion : " + QString::fromStdString( completion );
    }

    auto quantity = this->titleQuantity();
    if ( ( other.titleQuantity() != this->titleQuantity() ) && ( !quantity.empty() ) )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += QString::fromStdString(
            RiuSummaryQuantityNameInfoProvider::instance()->longNameFromQuantityName( quantity, true ) );
    }

    return title;
}
