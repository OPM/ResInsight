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
#include <QStringList>

#include <utility>
#include <vector>

//==================================================================================================
/// Column assignment and ordering of workflow tasks for the graph view. Pure; positions in pixels
/// are left to the view, which knows the node sizes.
//==================================================================================================
namespace RiuWorkflowGraphLayout
{
struct Layout
{
    QMap<QString, int>       ranks;
    std::vector<QStringList> layers;
};

// Longest-path ranks from a topological sort (Kahn), so the input does not need to be in
// topological order. Nodes on cycles are placed after their resolved predecessors. Within a
// column the nodes are ordered by the mean position of their predecessors, then by name.
Layout computeLayers( const QStringList& nodes, const std::vector<std::pair<QString, QString>>& edges );
} // namespace RiuWorkflowGraphLayout
