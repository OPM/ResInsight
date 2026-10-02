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

#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

//--------------------------------------------------------------------------------------------------
/// Preferences for the ri-cloud-api service. By default the service runs with the same Python
/// interpreter as the scripting engine (Preferences -> Scripting -> Python Executable Location). A
/// dedicated environment can be enabled here to avoid package/version conflicts between the two.
//--------------------------------------------------------------------------------------------------
class RiaPreferencesCloudApi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RiaPreferencesCloudApi();

    static RiaPreferencesCloudApi* current();

    void appendItems( caf::PdmUiOrdering& uiOrdering );

    bool    useDedicatedPythonEnvironment() const;
    QString pythonExecutable() const;

private:
    caf::PdmField<bool>    m_enableDedicatedPythonEnvironment;
    caf::PdmField<QString> m_pythonExecutable;
};
