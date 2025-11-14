/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include <QString>
#include <QStringList>

#include <optional>
#include <string>

//==================================================================================================
///
///
//==================================================================================================
class RimOpmFlowJobSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimOpmFlowJobSettings();
    ~RimOpmFlowJobSettings() override;

    RimOpmFlowJobSettings* clone();

    void uiOrdering( caf::PdmUiGroup* uiGroup, bool expandByDefault = true );

    int mpiProcesses() const;

    QStringList commandLineOptions() const;

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<int> m_mpiProcesses;
    caf::PdmField<int> m_threadsPerProcess;

    caf::PdmField<bool>                 m_enableEsmry;
    caf::PdmField<bool>                 m_enableTuning;
    caf::PdmField<bool>                 m_enableTerminalOutput;
    caf::PdmField<std::vector<QString>> m_ignoreKeywords;
    caf::PdmField<QString>              m_parsingStrictness;

    caf::PdmField<std::pair<bool, int>>    m_newtonMaxIterations;
    caf::PdmField<std::pair<bool, double>> m_relaxedMaxPvFraction;
    caf::PdmField<std::pair<bool, double>> m_solverMaxTimeStepInDays;
    caf::PdmField<std::pair<bool, double>> m_solverMinTimeStepInDays;
    caf::PdmField<std::pair<bool, int>>    m_minStrictCnvIter;
    caf::PdmField<std::pair<bool, int>>    m_minStrictMbIter;
    caf::PdmField<std::pair<bool, double>> m_minTimeStepBasedOnNewtonIterations;
    caf::PdmField<std::pair<bool, double>> m_minTimeStepBeforeShuttingProblematicWellsInDays;
};
