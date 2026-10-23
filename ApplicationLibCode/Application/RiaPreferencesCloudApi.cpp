/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RiaPreferencesCloudApi.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( RiaPreferencesCloudApi, "RiaPreferencesCloudApi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesCloudApi::RiaPreferencesCloudApi()
{
    CAF_PDM_InitField( &m_enableDedicatedPythonEnvironment,
                       "enableDedicatedPythonEnvironment",
                       false,
                       "Enable for Dedicated ri-cloud-api Environment" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_enableDedicatedPythonEnvironment );

    CAF_PDM_InitField( &m_pythonExecutable, "pythonExecutable", QString( "python" ), "Python Executable Location" );
    m_pythonExecutable.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_pythonExecutable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesCloudApi* RiaPreferencesCloudApi::current()
{
    return RiaApplication::instance()->preferences()->cloudApiPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesCloudApi::appendItems( caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_enableDedicatedPythonEnvironment );

    // Only show the dedicated interpreter field when it is actually in use. When disabled, the
    // service falls back to Preferences -> Scripting -> Python Executable Location.
    if ( m_enableDedicatedPythonEnvironment() )
    {
        uiOrdering.add( &m_pythonExecutable );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesCloudApi::useDedicatedPythonEnvironment() const
{
    return m_enableDedicatedPythonEnvironment();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesCloudApi::pythonExecutable() const
{
    return m_pythonExecutable().trimmed();
}
