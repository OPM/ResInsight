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
///
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowDefinition::taskTypeForNode( const QString& nodeName ) const
{
    const auto* node = findNode( nodeName );
    return node ? taskType( node->taskId ) : QJsonObject();
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
