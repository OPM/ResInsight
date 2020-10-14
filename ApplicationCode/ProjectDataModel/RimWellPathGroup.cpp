/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimWellPathGroup.h"

#include "RiaTextStringTools.h"
#include "RigWellPath.h"

#include "cafPdmUiTreeOrdering.h"

#include <QStringList>

CAF_PDM_SOURCE_INIT( RimWellPathGroup, "WellPathGroup" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathGroup::RimWellPathGroup()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Well Path Group", ":/Well.png", "", "", "WellPathGroup", "A Group of Well Paths" );
    CAF_PDM_InitFieldNoDefault( &m_childWellPaths, "ChildWellPaths", "Child Well Paths", "", "", "" );
    setWellPathGeometry( new RigWellPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::addChildWellPath( RimWellPath* wellPath )
{
    if ( !this->wellPathGeometry()->wellPathPoints().empty() )
    {
        auto commonGeometry = RigWellPath::commonGeometry( {this->wellPathGeometry(), wellPath->wellPathGeometry()} );
        setWellPathGeometry( commonGeometry.p() );
        m_childWellPaths.push_back( wellPath );
        makeMoreLevelsIfNecessary();
    }
    else
    {
        cvf::ref<RigWellPath> geometryCopy( new RigWellPath( *( wellPath->wellPathGeometry() ) ) );
        setWellPathGeometry( geometryCopy.p() );
        m_childWellPaths.push_back( wellPath );
    }
    updateWellPathName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimWellPathGroup::childWellPaths() const
{
    return m_childWellPaths.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimWellPathGroup::childWellpathCount() const
{
    return m_childWellPaths.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathGroup::hasChildWellPath( RimWellPath* wellPath )
{
    return m_childWellPaths.count( wellPath ) != 0u;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::removeChildWellPath( RimWellPath* wellPath )
{
    m_childWellPaths.removeChildObject( wellPath );
    auto commonGeometry = RigWellPath::commonGeometry( wellPathGeometries() );
    setWellPathGeometry( commonGeometry.p() );
    updateWellPathName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::removeAllChildWellPaths()
{
    m_childWellPaths.clear();
    auto commonGeometry = RigWellPath::commonGeometry( wellPathGeometries() );
    setWellPathGeometry( commonGeometry.p() );
    updateWellPathName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::fixBranchNames()
{
    const auto& measuredDepths = this->wellPathGeometry()->measuredDepths();

    int index = 1;
    for ( auto wellPath : m_childWellPaths )
    {
        auto group = dynamic_cast<RimWellPathGroup*>( wellPath.p() );
        if ( group )
        {
            group->fixBranchNames();
            if ( group->name() == this->name() )
            {
                QString groupName = QString( "%1 branch #%2" ).arg( this->name() ).arg( index++ );
                if ( !measuredDepths.empty() )
                {
                    groupName += QString( " at md=%1" ).arg( measuredDepths.back() );
                }
                group->setName( groupName );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    for ( auto child : m_childWellPaths() )
    {
        if ( child )
        {
            uiTreeOrdering.add( child );
        }
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RigWellPath*> RimWellPathGroup::wellPathGeometries() const
{
    std::vector<const RigWellPath*> allGeometries;
    for ( const auto child : m_childWellPaths() )
    {
        allGeometries.push_back( child->wellPathGeometry() );
    }
    return allGeometries;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::updateWellPathName()
{
    auto autoName = createWellPathName();
    setName( autoName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathGroup::createWellPathName() const
{
    QStringList allNames;
    for ( auto wellPath : m_childWellPaths )
    {
        allNames.push_back( wellPath->name() );
    }

    QString commonName        = RiaTextStringTools::commonRoot( allNames );
    QString trimmedCommonName = RiaTextStringTools::trimNonAlphaNumericCharacters( commonName );
    return trimmedCommonName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::makeMoreLevelsIfNecessary()
{
    if ( m_childWellPaths.size() <= 1u ) return;

    auto wellPathPoints = this->wellPathGeometry()->wellPathPoints();

    auto comp = []( const cvf::Vec3d& lhs, const cvf::Vec3d& rhs ) {
        auto diff = rhs - lhs;
        if ( diff.length() < 1.0e-8 ) return false;

        if ( diff.z() == 0.0 )
        {
            if ( diff.y() == 0.0 )
            {
                return diff.x() > 0.0;
            }
            return diff.y() > 0.0;
        }
        return diff.z() > 0.0;
    };

    auto branches = std::map<cvf::Vec3d, std::vector<RimWellPath*>, decltype( comp )>( comp );

    for ( auto wellPath : m_childWellPaths )
    {
        auto childWellPathPoints = wellPath->wellPathGeometry()->wellPathPoints();
        if ( childWellPathPoints.size() > wellPathPoints.size() )
        {
            cvf::Vec3d firstDeviation = childWellPathPoints[wellPathPoints.size()];
            branches[firstDeviation].push_back( wellPath );
        }
    }

    if ( branches.size() <= 1u ) return;

    for ( const auto& [firstDeviation, wellPaths] : branches )
    {
        if ( wellPaths.size() > 1u )
        {
            RimWellPathGroup* newGroup = new RimWellPathGroup;
            for ( auto wellPath : wellPaths )
            {
                m_childWellPaths().removeChildObject( wellPath );
                newGroup->addChildWellPath( wellPath );
            }
            m_childWellPaths().push_back( newGroup );
        }
    }
}
