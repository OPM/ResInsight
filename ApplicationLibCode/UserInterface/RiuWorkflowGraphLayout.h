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
#include <QPointF>
#include <QSizeF>
#include <QString>
#include <QStringList>

#include <utility>
#include <vector>

//==================================================================================================
/// Column assignment and ordering of workflow tasks for the graph view. Pure; positions in pixels
/// are left to the view, which knows the node sizes.
//==================================================================================================
namespace RiuWorkflowGraphLayout
{
enum class Orientation
{
    Horizontal, // Ranks are columns from left to right
    Vertical // Ranks are rows from top to bottom
};

struct Layout
{
    QMap<QString, int>       ranks;
    std::vector<QStringList> layers;
};

// Longest-path ranks from a topological sort (Kahn), so the input does not need to be in
// topological order. Nodes on cycles are placed after their resolved predecessors. Within a
// column the nodes are ordered by the mean position of their predecessors, then by name.
Layout computeLayers( const QStringList& nodes, const std::vector<std::pair<QString, QString>>& edges );

struct Spacing
{
    qreal columnStep; // Horizontal: distance between the left edges of neighbouring columns
    qreal rowGap; // Horizontal: gap between the nodes of a column
    qreal rankGap; // Vertical: gap between neighbouring rows
    qreal siblingGap; // Vertical: gap between the nodes of a row
};

// Top-left corner of each node in the layout, given the size of each node. The ranks are
// centered on the largest rank, and in vertical layouts each node is centered within its row.
QMap<QString, QPointF>
    computePositions( const Layout& layout, const QMap<QString, QSizeF>& sizes, Orientation orientation, const Spacing& spacing );
} // namespace RiuWorkflowGraphLayout
