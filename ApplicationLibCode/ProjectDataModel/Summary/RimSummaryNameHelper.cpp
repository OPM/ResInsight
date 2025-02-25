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

    auto titleCaseName = caseName();
    if ( !other.isCaseInTitle() && !titleCaseName.isEmpty() )
    {
        if ( !title.isEmpty() ) title += ", ";

        title += titleCaseName;
    }

    auto wellName = titleWellName();
    if ( !other.isWellNameInTitle() && !wellName.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += QString::fromStdString( wellName );
    }

    auto groupName = titleGroupName();
    if ( !other.isGroupNameInTitle() && !groupName.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += QString::fromStdString( groupName );
    }

    auto networkName = titleNetwork();
    if ( !other.isNetworkInTitle() && !networkName.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += QString::fromStdString( networkName );
    }

    auto region = titleRegion();
    if ( !other.isRegionInTitle() && !region.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Region : " + QString::fromStdString( region );
    }

    auto block = titleBlock();
    if ( !other.isBlockInTitle() && !block.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Block : " + QString::fromStdString( block );
    }

    auto segment = titleSegment();
    if ( !other.isSegmentInTitle() && !segment.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Segment : " + QString::fromStdString( segment );
    }

    auto wellCompletion = titleWellCompletion();
    if ( !other.isWellCompletionInTitle() && !wellCompletion.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Well Completion : " + QString::fromStdString( wellCompletion );
    }

    auto connection = titleConnection();
    if ( !other.isConnectionInTitle() && !connection.empty() )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += "Connection : " + QString::fromStdString( connection );
    }

    auto vectorName = titleVectorName();
    if ( ( other.titleVectorName() != titleVectorName() ) && ( !vectorName.empty() ) )
    {
        if ( !title.isEmpty() ) title += ", ";
        title += QString::fromStdString( RiuSummaryQuantityNameInfoProvider::instance()->longNameFromVectorName( vectorName, true ) );

        // https://github.com/OPM/ResInsight/issues/12157
        size_t pos = vectorName.find( '_' );
        if ( pos != std::string::npos )
        {
            // For calculated curves, the name vector name is often created using underscore. Check if the vector name already is
            // present in the title.
            auto qtVectorName = QString::fromStdString( vectorName );
            if ( title.indexOf( qtVectorName ) == -1 )
            {
                title += QString( " (%1)" ).arg( QString::fromStdString( vectorName ) );
            }
        }
    }

    return title;
}
