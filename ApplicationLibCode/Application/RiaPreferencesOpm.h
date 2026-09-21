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

#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimOpmFlowJobSettings;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaPreferencesOpm : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RiaPreferencesOpm();

    static RiaPreferencesOpm* current();

    void appendItems( caf::PdmUiOrdering& uiOrdering );

    bool validateFlowSettings() const;

    QString     opmFlowCommand() const;
    QStringList wslOptions() const;
    bool        useWsl() const;
    bool        useMpi() const;
    QString     mpirunCommand() const;

    RimOpmFlowJobSettings* createDefaultJobSettings() const;

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmField<QString>                     m_opmFlowCommand;
    caf::PdmField<bool>                        m_useWsl;
    caf::PdmField<QString>                     m_wslDistribution;
    caf::PdmField<bool>                        m_useMpi;
    caf::PdmField<QString>                     m_mpirunCommand;
    caf::PdmChildField<RimOpmFlowJobSettings*> m_jobSettings;

    QStringList m_availableWslDists;
};
