/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RiaPreferencesOpm.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaWslTools.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT( RiaPreferencesOpm, "RiaPreferencesOpm" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesOpm::RiaPreferencesOpm()
{
    CAF_PDM_InitFieldNoDefault( &m_opmFlowCommand, "opmFlowCommand", "Opm Flow Command to run" );
    m_opmFlowCommand.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_opmFlowCommand.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitField( &m_useWsl, "useWsl", false, "Use WSL to run Opm Flow" );
    CAF_PDM_InitField( &m_useMpi, "useMpi", false, "Use MPI to run Opm Flow" );
    CAF_PDM_InitField( &m_mpiProcesses, "mpiProcesses", 4, "Number of MPI processes to use" );

    CAF_PDM_InitField( &m_mpirunCommand, "mpirunCommand", QString( "/usr/bin/mpirun" ), "Path to mpirun command" );
    m_mpirunCommand.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_mpirunCommand.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_wslDistribution, "wslDistribution", "WSL Distribution to use:" );
    m_wslDistribution.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_availableWslDists = RiaWslTools::wslDistributionList();
    if ( !m_availableWslDists.isEmpty() ) m_wslDistribution = m_availableWslDists.at( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesOpm* RiaPreferencesOpm::current()
{
    return RiaApplication::instance()->preferences()->opmPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesOpm::appendItems( caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* opmGrp = uiOrdering.addNewGroup( "Opm Flow Settings" );
    opmGrp->add( &m_opmFlowCommand );
    auto wslCmd = RiaWslTools::wslCommand();
    if ( !wslCmd.isEmpty() )
    {
        opmGrp->add( &m_useWsl );
        if ( m_useWsl() )
        {
            opmGrp->add( &m_wslDistribution );
        }
    }

    opmGrp->add( &m_useMpi );
    if ( m_useMpi() )
    {
        opmGrp->add( &m_mpiProcesses );
        opmGrp->add( &m_mpirunCommand );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiaPreferencesOpm::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_wslDistribution )
    {
        for ( auto dist : m_availableWslDists )
        {
            options.push_back( caf::PdmOptionItemInfo( dist, QVariant::fromValue( dist ) ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesOpm::opmFlowCommand() const
{
    return m_opmFlowCommand;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesOpm::mpirunCommand() const
{
    return m_mpirunCommand;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaPreferencesOpm::wslOptions() const
{
    QStringList options;
    if ( m_useWsl() )
    {
        options.append( "-d" );
        options.append( m_wslDistribution() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesOpm::validateFlowSettings() const
{
    return m_opmFlowCommand().size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesOpm::useWsl() const
{
    return m_useWsl();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesOpm::useMpi() const
{
    return m_useMpi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaPreferencesOpm::mpiProcesses() const
{
    return std::max( 1, m_mpiProcesses() );
}
