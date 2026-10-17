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

#include "RimWorkflowDefinition.h"

#include "RimWorkflowPortCompatibility.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowDefinitionEdge::isWholeInput() const
{
    return input.isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowDefinitionEdge::isWholeOutput() const
{
    return output.isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowDefinitionEdge::isCollected() const
{
    return collect != Collect::None;
}

//--------------------------------------------------------------------------------------------------
/// Same tasks and ports, ignoring how the value is collected
//--------------------------------------------------------------------------------------------------
bool RimWorkflowDefinitionEdge::sameConnection( const RimWorkflowDefinitionEdge& other ) const
{
    return from == other.from && output == other.output && to == other.to && input == other.input;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWorkflowDefinitionNode* RimWorkflowDefinition::findNode( const QString& nodeName ) const
{
    for ( const auto& node : nodes )
    {
        if ( node.name == nodeName ) return &node;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionNode* RimWorkflowDefinition::findNode( const QString& nodeName )
{
    for ( auto& node : nodes )
    {
        if ( node.name == nodeName ) return &node;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWorkflowDefinition::nodeIndex( const QString& nodeName ) const
{
    for ( size_t i = 0; i < nodes.size(); ++i )
    {
        if ( nodes[i].name == nodeName ) return static_cast<int>( i );
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowDefinition::taskType( const QString& taskId ) const
{
    return taskTypes.value( taskId ).toObject();
}

//--------------------------------------------------------------------------------------------------
/// The task type as seen by the workflow: a mapped task has `dict[str, Output]` as output, and its
/// key and value fields are not inputs
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowDefinition::taskTypeForNode( const QString& nodeName ) const
{
    const auto* node = findNode( nodeName );
    if ( !node ) return {};
    const QJsonObject type = taskType( node->taskId );
    if ( type.isEmpty() || !node->map ) return type;
    return RimWorkflowPortCompatibility::mappedTaskType( type, *node->map );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWorkflowDefinitionEdge> RimWorkflowDefinition::incomingEdges( const QString& nodeName ) const
{
    std::vector<RimWorkflowDefinitionEdge> result;
    for ( const auto& edge : edges )
    {
        if ( edge.to == nodeName ) result.push_back( edge );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWorkflowDefinitionEdge> RimWorkflowDefinition::outgoingEdges( const QString& nodeName ) const
{
    std::vector<RimWorkflowDefinitionEdge> result;
    for ( const auto& edge : edges )
    {
        if ( edge.from == nodeName ) result.push_back( edge );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowDefinition::nodeNames() const
{
    QStringList names;
    for ( const auto& node : nodes )
        names.append( node.name );
    return names;
}
