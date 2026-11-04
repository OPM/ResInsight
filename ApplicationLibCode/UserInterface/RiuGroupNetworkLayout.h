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

#pragma once

#include <QMap>
#include <QString>

#include <utility>
#include <vector>

//==================================================================================================
/// Column and row assignment of the nodes of a network tree for the graph view. Columns are the depth
/// in the tree (root to the left). Leaves get consecutive rows, and a parent is centred on its
/// children, so the children of a node are placed next to each other. Pure; positions in pixels are
/// left to the view, which knows the node sizes.
//==================================================================================================
namespace RiuGroupNetworkLayout
{
struct Position
{
    int    column = 0;
    double row    = 0.0;
};

// Nodes as (node, parent) pairs, in the order the children are to be shown. An empty or unknown
// parent makes the node a root. Several roots are stacked below each other. Nodes on cycles are
// left out.
QMap<QString, Position> computePositions( const std::vector<std::pair<QString, QString>>& nodeParents );
} // namespace RiuGroupNetworkLayout
