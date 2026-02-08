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

#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaPreferencesSumoExplorer : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RiaPreferencesSumoExplorer();

    static RiaPreferencesSumoExplorer* current();

    unsigned int serverPort() const;
    QString      pythonPath() const;
    bool         autoStartServer() const;
    QString      sumoEnvironment() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<unsigned int> m_serverPort;
    caf::PdmField<QString>      m_pythonPath;
    caf::PdmField<bool>         m_autoStartServer;
    caf::PdmField<QString>      m_sumoEnvironment;
};
