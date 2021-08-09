/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include "RimParameterGroups.h"

#include "RimGenericParameter.h"
#include "RimParameterGroup.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterGroups::RimParameterGroups()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterGroups::~RimParameterGroups()
{
    clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroups::clear()
{
    for ( const auto& [key, value] : m_groups )
    {
        delete value;
    }
    m_groups.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterGroups::mergeGroup( RimParameterGroup* group, bool addCommentAsParameter /* = false */ )
{
    const QString grpName = group->name();

    if ( m_groups.count( grpName ) == 0 )
    {
        RimParameterGroup* newGroup = new RimParameterGroup();
        newGroup->setName( grpName );
        newGroup->setLabel( group->label() );
        newGroup->setComment( group->comment() );
        if ( addCommentAsParameter && !newGroup->comment().isEmpty() )
        {
            newGroup->addParameter( "comments", newGroup->comment() );
        }

        m_groups[grpName] = newGroup;
    }

    RimParameterGroup* dstGroup = m_groups[grpName];
    for ( auto& parameter : group->parameters() )
    {
        dstGroup->addParameter( parameter->duplicate() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimParameterGroup*> RimParameterGroups::groups()
{
    std::vector<RimParameterGroup*> retGroups;

    for ( const auto& [key, value] : m_groups )
    {
        retGroups.push_back( value );
    }

    return retGroups;
}
