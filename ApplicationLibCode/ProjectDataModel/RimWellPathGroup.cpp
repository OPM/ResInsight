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
#include "RimWellPathGroup.h"

#include "RiaTextStringTools.h"
#include "RigWellPath.h"
#include "RimModeledWellPathLateral.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathCompletions.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"

#include <QStringList>

CAF_PDM_SOURCE_INIT( RimWellPathGroup, "WellPathGroup" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathGroup::RimWellPathGroup()
    : wellPathAddedOrRemoved( this )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Well Path Group",
                                                    ":/WellPathGroup.svg",
                                                    "",
                                                    "",
                                                    "WellPathGroup",
                                                    "A Group of Well Paths" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_childWellPaths, "ChildWellPaths", "Child Well Paths", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_groupName, "GroupName", "Group Name", "", "", "" );
    m_groupName.registerGetMethod( this, &RimWellPathGroup::createGroupName );
    setWellPathGeometry( new RigWellPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::addChildWellPath( RimWellPath* wellPath )
{
    if ( m_childWellPaths.empty() && isTopLevelWellPath() && wellPath->completions()->hasCompletions() )
    {
        RimWellPath::copyCompletionSettings( wellPath, this );
    }

    if ( !this->wellPathGeometry()->wellPathPoints().empty() )
    {
        m_childWellPaths.push_back( wellPath );
        createWellPathGeometry();

        makeMoreLevelsIfNecessary();
    }
    else
    {
        cvf::ref<RigWellPath> geometryCopy( new RigWellPath( *( wellPath->wellPathGeometry() ) ) );
        setWellPathGeometry( geometryCopy.p() );
        m_childWellPaths.push_back( wellPath );
    }

    wellPath->nameChanged.connect( this, &RimWellPathGroup::onChildNameChanged );

    updateAllRequiredEditors();
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
    RimWellPath::copyCompletionSettings( this, wellPath );

    if ( auto geometry = wellPath->wellPathGeometry(); geometry )
    {
        geometry->setUniqueStartIndex( 0u );
    }
    createWellPathGeometry();
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::removeAllChildWellPaths()
{
    auto childWellPaths = m_childWellPaths.childObjects();
    for ( auto wellPath : childWellPaths )
    {
        removeChildWellPath( wellPath );
    }
    setWellPathGeometry( cvf::ref<RigWellPath>( new RigWellPath ).p() );
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::createWellPathGeometry()
{
    for ( auto wellPath : m_childWellPaths )
    {
        if ( auto group = dynamic_cast<RimWellPathGroup*>( wellPath.p() ); group )
        {
            group->createWellPathGeometry();
        }
    }
    if ( wellPathGeometries().empty() ) return;

    auto commonGeometry = RigWellPath::commonGeometry( wellPathGeometries() );
    for ( auto wellPath : m_childWellPaths )
    {
        size_t startIndex = 0u;
        size_t commonSize = commonGeometry->wellPathPoints().size();
        if ( commonSize > 0u ) startIndex = commonSize - 1u;
        wellPath->wellPathGeometry()->setUniqueStartIndex( startIndex );
    }
    setWellPathGeometry( commonGeometry.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    RimWellPath::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );

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
caf::PdmFieldHandle* RimWellPathGroup::userDescriptionField()
{
    return &m_groupName;
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
QString RimWellPathGroup::createGroupName() const
{
    QStringList               allNames;
    std::vector<RimWellPath*> descendantWellPaths;
    this->descendantsOfType( descendantWellPaths );
    for ( auto wellPath : descendantWellPaths )
    {
        if ( !dynamic_cast<RimWellPathGroup*>( wellPath ) && !dynamic_cast<RimModeledWellPathLateral*>( wellPath ) )
        {
            allNames.push_back( wellPath->name() );
        }
    }

    QString     commonName        = RiaTextStringTools::commonRoot( allNames );
    QString     trimmedCommonName = RiaTextStringTools::trimNonAlphaNumericCharacters( commonName );
    QStringList branchNames;
    for ( auto& name : allNames )
    {
        name.remove( commonName );
        name = RiaTextStringTools::trimNonAlphaNumericCharacters( name );
        name = name.simplified();
        if ( !name.isEmpty() ) branchNames.push_back( name );
    }
    QString fullName = trimmedCommonName;
    if ( !branchNames.isEmpty() )
    {
        fullName += QString( "(%1)" ).arg( branchNames.join( ", " ) );
    }
    return fullName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGroup::onChildNameChanged( const caf::SignalEmitter* emitter )
{
    updateConnectedEditors();
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

    bool anyNonTrivialBranches = false;
    for ( const auto& firstDeviationAndWellPaths : branches )
    {
        const auto& wellPaths = firstDeviationAndWellPaths.second;
        if ( wellPaths.size() > 1u )
        {
            anyNonTrivialBranches = true;
            break;
        }
    }
    if ( anyNonTrivialBranches )
    {
        size_t startIndex = 0u;
        size_t commonSize = wellPathGeometry()->wellPathPoints().size();
        if ( commonSize > 0u ) startIndex = commonSize - 1u;

        for ( const auto& firstDeviationAndWellPaths : branches )
        {
            const auto& wellPaths = firstDeviationAndWellPaths.second;
            if ( wellPaths.size() > 1u )
            {
                RimWellPathGroup* newGroup = new RimWellPathGroup;
                for ( auto wellPath : wellPaths )
                {
                    m_childWellPaths().removeChildObject( wellPath );
                    newGroup->addChildWellPath( wellPath );
                    newGroup->wellPathGeometry()->setUniqueStartIndex( startIndex );
                }
                m_childWellPaths().push_back( newGroup );
            }
        }
    }
}
