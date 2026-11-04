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

#include <QColor>
#include <QString>

#include <optional>
#include <set>
#include <vector>

//==================================================================================================
/// Plain description of the nodes of a network graph, shared by the project data model and the graph
/// view. The view only draws these nodes; filtering and collapsing are done here, so the logic is
/// independent of the GUI.
//==================================================================================================
namespace RimGroupNetworkGraph
{
struct Node
{
    QString name;
    QString parent; // Empty for a root node
    bool    isWell     = false;
    bool    isProducer = false;
    bool    isInjector = false;

    QString               valueText; // Node metric shown in the node, such as the pressure
    QString               valueNumber; // The number and unit of valueText, shown separately in the node
    QString               valueUnit;
    std::optional<double> edgeValue; // Flow from the node to its parent
    QString               edgeText;
    QColor                edgeColor; // Colour of the edge, given by the selected edge value
    QString               toolTip;

    // Set by visibleNodes()
    int    childCount        = 0; // Children that pass the type filter
    bool   collapsed         = false;
    int    hiddenNodeCount   = 0; // Descendants hidden by collapsing the node
    double edgeWidthFraction = 0.0; // Edge value relative to the largest visible edge value

    bool isOther() const;
};

struct Filter
{
    bool              showProducers = true;
    bool              showInjectors = true;
    bool              showOther     = true;
    std::set<QString> collapsedNodes;
};

// The result lists parents before children, depth first from the roots. Nodes on a cycle are not
// reachable from a root and are left out. A node is kept if its type is shown or any of its
// descendants is kept, so the path to a shown node is never broken. Roots are always kept. The
// descendants of a collapsed node are hidden.
std::vector<Node> visibleNodes( const std::vector<Node>& nodes, const Filter& filter );

QString formatValue( double value );
} // namespace RimGroupNetworkGraph
