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
