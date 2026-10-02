/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiuWorkflowGraphLayout.h"

#include <QSet>

#include <algorithm>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowGraphLayout::Layout RiuWorkflowGraphLayout::computeLayers( const QStringList&                              nodes,
                                                                      const std::vector<std::pair<QString, QString>>& edges )
{
    QStringList uniqueNodes = nodes;
    uniqueNodes.removeDuplicates();
    uniqueNodes.removeAll( QString() );
    const QSet<QString> nodeSet( uniqueNodes.begin(), uniqueNodes.end() );

    QMap<QString, QStringList>        predecessors;
    QMap<QString, QStringList>        successors;
    QSet<std::pair<QString, QString>> seen;
    for ( const auto& [from, to] : edges )
    {
        if ( from == to || !nodeSet.contains( from ) || !nodeSet.contains( to ) || seen.contains( { from, to } ) ) continue;
        seen.insert( { from, to } );
        predecessors[to].append( from );
        successors[from].append( to );
    }

    Layout layout;

    QMap<QString, int> remainingIncoming;
    for ( const QString& node : uniqueNodes )
        remainingIncoming[node] = predecessors.value( node ).size();

    QSet<QString> placed;
    QStringList   ready;
    for ( const QString& node : uniqueNodes )
    {
        if ( remainingIncoming[node] == 0 ) ready.append( node );
    }

    while ( placed.size() < uniqueNodes.size() )
    {
        if ( ready.isEmpty() )
        {
            // A cycle: release the first unplaced node (in input order) to break it
            for ( const QString& node : uniqueNodes )
            {
                if ( !placed.contains( node ) )
                {
                    ready.append( node );
                    break;
                }
            }
        }

        const QString node = ready.takeFirst();
        if ( placed.contains( node ) ) continue;
        placed.insert( node );

        int rank = 0;
        for ( const QString& predecessor : predecessors.value( node ) )
        {
            if ( placed.contains( predecessor ) ) rank = std::max( rank, layout.ranks.value( predecessor ) + 1 );
        }
        layout.ranks[node] = rank;

        for ( const QString& successor : successors.value( node ) )
        {
            if ( placed.contains( successor ) ) continue;
            if ( --remainingIncoming[successor] == 0 ) ready.append( successor );
        }
    }

    int layerCount = 0;
    for ( int rank : layout.ranks )
        layerCount = std::max( layerCount, rank + 1 );
    layout.layers.resize( layerCount );
    for ( auto it = layout.ranks.cbegin(); it != layout.ranks.cend(); ++it )
        layout.layers[it.value()].append( it.key() );

    // One barycenter pass from left to right, ties broken by name
    QMap<QString, int> positions;
    for ( QStringList& layer : layout.layers )
    {
        std::vector<std::pair<double, QString>> keyed;
        for ( const QString& node : layer )
        {
            double sum   = 0.0;
            int    count = 0;
            for ( const QString& predecessor : predecessors.value( node ) )
            {
                if ( positions.contains( predecessor ) && layout.ranks.value( predecessor ) < layout.ranks.value( node ) )
                {
                    sum += positions.value( predecessor );
                    ++count;
                }
            }
            keyed.emplace_back( count > 0 ? sum / count : std::numeric_limits<double>::max(), node );
        }
        std::sort( keyed.begin(), keyed.end() );

        layer.clear();
        for ( size_t i = 0; i < keyed.size(); ++i )
        {
            layer.append( keyed[i].second );
            positions[keyed[i].second] = static_cast<int>( i );
        }
    }

    return layout;
}
