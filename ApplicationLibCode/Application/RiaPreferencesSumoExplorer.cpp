/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RiaPreferencesSumoExplorer.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "Cloud/RiaSumoExplorerDefines.h"

#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( RiaPreferencesSumoExplorer, "RiaPreferencesSumoExplorer" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSumoExplorer::RiaPreferencesSumoExplorer()
{
    CAF_PDM_InitField( &m_serverPort, "serverPort", RiaSumoExplorerDefines::defaultPort(), "Server Port" );

    CAF_PDM_InitFieldNoDefault( &m_pythonPath, "pythonPath", "Python Executable Path" );

    CAF_PDM_InitField( &m_autoStartServer, "autoStartServer", true, "Auto-Start Server" );

    CAF_PDM_InitField( &m_sumoEnvironment, "sumoEnvironment", QString( "prod" ), "Sumo Environment" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSumoExplorer* RiaPreferencesSumoExplorer::current()
{
    return RiaApplication::instance()->preferences()->sumoExplorerPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
unsigned int RiaPreferencesSumoExplorer::serverPort() const
{
    return m_serverPort;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesSumoExplorer::pythonPath() const
{
    return m_pythonPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSumoExplorer::autoStartServer() const
{
    return m_autoStartServer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesSumoExplorer::sumoEnvironment() const
{
    return m_sumoEnvironment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSumoExplorer::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_autoStartServer );
    uiOrdering.add( &m_serverPort );
    uiOrdering.add( &m_pythonPath );
    uiOrdering.add( &m_sumoEnvironment );

    uiOrdering.skipRemainingFields( true );
}
