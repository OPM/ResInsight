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

#include "RiuGroupNetworkLayout.h"

#include <QSet>

#include <functional>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMap<QString, RiuGroupNetworkLayout::Position>
    RiuGroupNetworkLayout::computePositions( const std::vector<std::pair<QString, QString>>& nodeParents )
{
    QSet<QString> knownNodes;
    for ( const auto& [node, parent] : nodeParents )
    {
        knownNodes.insert( node );
    }

    QStringList                roots;
    QMap<QString, QStringList> children;
    QSet<QString>              added;
    for ( const auto& [node, parent] : nodeParents )
    {
        if ( added.contains( node ) ) continue;
        added.insert( node );

        if ( parent.isEmpty() || !knownNodes.contains( parent ) || parent == node )
        {
            roots.push_back( node );
        }
        else
        {
            children[parent].push_back( node );
        }
    }

    QMap<QString, Position> positions;
    double                  nextLeafRow = 0.0;

    std::function<void( const QString&, int )> place = [&]( const QString& node, int depth )
    {
        positions[node].column = depth;

        QStringList placedChildren;
        for ( const auto& child : children.value( node ) )
        {
            if ( positions.contains( child ) ) continue;
            place( child, depth + 1 );
            placedChildren.push_back( child );
        }

        if ( placedChildren.isEmpty() )
        {
            positions[node].row = nextLeafRow;
            nextLeafRow += 1.0;
        }
        else
        {
            positions[node].row = 0.5 * ( positions[placedChildren.front()].row + positions[placedChildren.back()].row );
        }
    };

    for ( const auto& root : roots )
    {
        place( root, 0 );
    }

    return positions;
}
