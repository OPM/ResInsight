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

#include "RigGroupNetwork.h"

#include <utility>

namespace
{
using Connectivity = std::map<std::string, std::pair<std::string, bool>>; // child -> (parent, is well)

// The connectivity changes of a tree type. Revisions that only change properties give the same connectivity as the previous
// revision, and are left out.
std::vector<std::pair<std::time_t, Connectivity>> connectivityChanges( const std::vector<RigGroupNetwork::TreeRevision>& revisions )
{
    std::vector<std::pair<std::time_t, Connectivity>> changes;
    for ( const auto& revision : revisions )
    {
        Connectivity connectivity;
        for ( const auto& edge : revision.edges )
        {
            connectivity[edge.child] = { edge.parent, edge.isWell() };
        }

        if ( changes.empty() || changes.back().second != connectivity ) changes.emplace_back( revision.date, std::move( connectivity ) );
    }
    return changes;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGroupNetwork::Edge::isWell() const
{
    return keyword == EdgeKeyword::WELSPECS;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGroupNetwork::Edge::isRoot() const
{
    return parent.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGroupNetwork::addTree( TreeType treeType, const TreeRevision& tree )
{
    auto& trees = m_trees[treeType];
    if ( !trees.empty() && trees.back().date == tree.date )
    {
        trees.back() = tree;
        return;
    }
    trees.push_back( tree );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGroupNetwork::hasTreeType( TreeType treeType ) const
{
    auto it = m_trees.find( treeType );
    return it != m_trees.end() && !it->second.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigGroupNetwork::TreeRevision>& RigGroupNetwork::trees( TreeType treeType ) const
{
    static const std::vector<TreeRevision> empty;

    auto it = m_trees.find( treeType );
    return it != m_trees.end() ? it->second : empty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::time_t> RigGroupNetwork::dates( TreeType treeType ) const
{
    std::vector<std::time_t> result;
    for ( const auto& tree : trees( treeType ) )
    {
        result.push_back( tree.date );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGroupNetwork::TreeRevision* RigGroupNetwork::treeAtDate( TreeType treeType, std::time_t date ) const
{
    const auto& treeRevisions = trees( treeType );
    if ( treeRevisions.empty() ) return nullptr;

    const TreeRevision* result = &treeRevisions.front();
    for ( const auto& tree : treeRevisions )
    {
        if ( tree.date > date ) break;
        result = &tree;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGroupNetwork::isEmpty() const
{
    return !hasTreeType( TreeType::GRUPTREE ) && !hasTreeType( TreeType::BRANPROP );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGroupNetwork::hasSameTopology( const RigGroupNetwork& other ) const
{
    for ( auto treeType : { TreeType::GRUPTREE, TreeType::BRANPROP } )
    {
        if ( connectivityChanges( trees( treeType ) ) != connectivityChanges( other.trees( treeType ) ) ) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RigGroupNetwork::keywordText( EdgeKeyword keyword )
{
    switch ( keyword )
    {
        case EdgeKeyword::GRUPTREE:
            return "GRUPTREE";
        case EdgeKeyword::BRANPROP:
            return "BRANPROP";
        case EdgeKeyword::WELSPECS:
            return "WELSPECS";
    }
    return {};
}
