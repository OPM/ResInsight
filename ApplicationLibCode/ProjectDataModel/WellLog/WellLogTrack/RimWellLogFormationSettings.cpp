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

#include "RimWellLogFormationSettings.h"

#include "RiaSimWellBranchTools.h"

#include "RiaWellLogTrackDefines.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimTools.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"

#include "RigEclipseCaseData.h"

#include "cafPdmUiGroup.h"

CAF_PDM_SOURCE_INIT( RimWellLogFormationSettings, "RimWellLogFormationSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFormationSettings::RimWellLogFormationSettings()
{
    CAF_PDM_InitObject( "Formation Settings" );

    CAF_PDM_InitFieldNoDefault( &m_formationSource, "FormationSource", "Source" );

    CAF_PDM_InitFieldNoDefault( &m_formationTrajectoryType, "FormationTrajectoryType", "Trajectory" );

    CAF_PDM_InitFieldNoDefault( &m_formationWellPathForSourceCase, "FormationWellPath", "Well Path" );
    m_formationWellPathForSourceCase.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_formationWellPathForSourceWellPath, "FormationWellPathForSourceWellPath", "Well Path" );
    m_formationWellPathForSourceWellPath.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_formationSimWellName, "FormationSimulationWellName", QString( "None" ), "Simulation Well" );
    CAF_PDM_InitField( &m_formationBranchIndex, "FormationBranchIndex", 0, " " );
    CAF_PDM_InitField( &m_formationBranchDetection,
                       "FormationBranchDetection",
                       true,
                       "Branch Detection",
                       "",
                       "Compute branches based on how simulation well cells are organized",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_formationCase, "FormationCase", "Formation Case" );
    m_formationCase.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_formationLevel, "FormationLevel", "Well Pick Filter" );

    CAF_PDM_InitField( &m_showFormationFluids, "ShowFormationFluids", false, "Show Fluids" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellLogTrackFormationSource RimWellLogFormationSettings::formationSource() const
{
    return m_formationSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::setFormationSource( RiaDefines::WellLogTrackFormationSource source )
{
    m_formationSource = source;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimWellLogFormationSettings::formationCase() const
{
    return m_formationCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::setFormationCase( RimCase* rimCase )
{
    m_formationCase = rimCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellLogTrackTrajectoryType RimWellLogFormationSettings::trajectoryType() const
{
    return m_formationTrajectoryType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::setTrajectoryType( RiaDefines::WellLogTrackTrajectoryType trajectoryType )
{
    m_formationTrajectoryType = trajectoryType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogFormationSettings::wellPathForSourceCase() const
{
    return m_formationWellPathForSourceCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::setWellPathForSourceCase( RimWellPath* wellPath )
{
    m_formationWellPathForSourceCase = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogFormationSettings::wellPathForSourceWellPath() const
{
    return m_formationWellPathForSourceWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::setWellPathForSourceWellPath( RimWellPath* wellPath )
{
    m_formationWellPathForSourceWellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogFormationSettings::simWellName() const
{
    return m_formationSimWellName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::setSimWellName( const QString& simWellName )
{
    m_formationSimWellName = simWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogFormationSettings::branchIndex() const
{
    return m_formationBranchIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::setBranchIndex( int branchIndex )
{
    m_formationBranchIndex = branchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogFormationSettings::branchDetection() const
{
    return m_formationBranchDetection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::setBranchDetection( bool branchDetection )
{
    m_formationBranchDetection = branchDetection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellLogTrackFormationLevel RimWellLogFormationSettings::formationLevel() const
{
    return m_formationLevel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::setFormationLevel( RiaDefines::WellLogTrackFormationLevel level )
{
    m_formationLevel = level;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogFormationSettings::showFormationFluids() const
{
    return m_showFormationFluids();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::setShowFormationFluids( bool show )
{
    m_showFormationFluids = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::uiOrdering( const QString& uiConfigName, caf::PdmUiOrdering& uiOrdering, bool formationsForCaseWithSimWellOnly )
{
    if ( !formationsForCaseWithSimWellOnly )
    {
        uiOrdering.add( &m_formationSource );
    }

    if ( m_formationSource() == RiaDefines::WellLogTrackFormationSource::CASE )
    {
        uiOrdering.add( &m_formationCase );

        if ( !formationsForCaseWithSimWellOnly )
        {
            uiOrdering.add( &m_formationTrajectoryType );

            if ( m_formationTrajectoryType() == RiaDefines::WellLogTrackTrajectoryType::WELL_PATH )
            {
                uiOrdering.add( &m_formationWellPathForSourceCase );
            }
        }

        if ( formationsForCaseWithSimWellOnly || m_formationTrajectoryType() == RiaDefines::WellLogTrackTrajectoryType::SIMULATION_WELL )
        {
            uiOrdering.add( &m_formationSimWellName );

            RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromSimWellName( &uiOrdering,
                                                                                       m_formationSimWellName,
                                                                                       m_formationBranchDetection,
                                                                                       m_formationBranchIndex );
        }
    }
    else if ( m_formationSource() == RiaDefines::WellLogTrackFormationSource::WELL_PICK_FILTER )
    {
        uiOrdering.add( &m_formationWellPathForSourceWellPath );
        if ( m_formationWellPathForSourceWellPath() )
        {
            uiOrdering.add( &m_formationLevel );
            uiOrdering.add( &m_showFormationFluids );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogFormationSettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_formationSource && m_formationSource() == RiaDefines::WellLogTrackFormationSource::WELL_PICK_FILTER )
    {
        std::vector<RimWellPath*> wellPaths;
        RimTools::wellPathWithFormations( &wellPaths );
        for ( RimWellPath* wellPath : wellPaths )
        {
            if ( wellPath == m_formationWellPathForSourceCase )
            {
                m_formationWellPathForSourceWellPath = m_formationWellPathForSourceCase();
                break;
            }
        }
    }

    auto track = firstAncestorOrThisOfType<RimWellLogTrack>();
    if ( track )
    {
        track->loadDataAndUpdate();
        track->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogFormationSettings::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_formationWellPathForSourceCase )
    {
        RimTools::wellPathOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_formationWellPathForSourceWellPath )
    {
        RimTools::wellPathWithFormationsOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_formationCase )
    {
        RimTools::caseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_formationSimWellName )
    {
        std::set<QString> sortedWellNames;

        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_formationCase.value() );
        if ( eclipseCase && eclipseCase->eclipseCaseData() )
        {
            sortedWellNames = eclipseCase->eclipseCaseData()->findSortedWellNames();
        }

        caf::IconProvider simWellIcon( ":/Well.svg" );
        for ( const QString& wname : sortedWellNames )
        {
            options.push_back( caf::PdmOptionItemInfo( wname, wname, false, simWellIcon ) );
        }
    }
    else if ( fieldNeedingOptions == &m_formationBranchIndex )
    {
        auto simulationWellBranches = RiaSimWellBranchTools::simulationWellBranches( m_formationSimWellName(), m_formationBranchDetection );
        options                     = RiaSimWellBranchTools::valueOptionsForBranchIndexField( simulationWellBranches );
    }
    else if ( fieldNeedingOptions == &m_formationLevel )
    {
        if ( m_formationWellPathForSourceWellPath )
        {
            const RigWellPathFormations* formations = m_formationWellPathForSourceWellPath->formationsGeometry();
            if ( formations )
            {
                using FormationLevelEnum = caf::AppEnum<RiaDefines::WellLogTrackFormationLevel>;

                options.push_back( caf::PdmOptionItemInfo( FormationLevelEnum::uiText( RiaDefines::WellLogTrackFormationLevel::NONE ),
                                                           RiaDefines::WellLogTrackFormationLevel::NONE ) );

                options.push_back( caf::PdmOptionItemInfo( FormationLevelEnum::uiText( RiaDefines::WellLogTrackFormationLevel::ALL ),
                                                           RiaDefines::WellLogTrackFormationLevel::ALL ) );

                for ( const auto& level : formations->formationsLevelsPresent() )
                {
                    options.push_back( caf::PdmOptionItemInfo( FormationLevelEnum::uiText( level ), level ) );
                }
            }
        }
    }

    return options;
}
