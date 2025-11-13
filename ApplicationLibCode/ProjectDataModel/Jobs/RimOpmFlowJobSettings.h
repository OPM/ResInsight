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

    void uiOrdering( caf::PdmUiGroup* uiGroup );

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    // QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmField<int> m_mpiProcesses;

    caf::PdmField<bool> m_enableEsmry;
    caf::PdmField<bool> m_enableTuning;
    caf::PdmField<bool> m_enableTerminalOutput;

    caf::PdmField<std::vector<QString>>    m_ignoreKeywords;
    caf::PdmField<std::pair<bool, int>>    m_newtonMaxIterations;
    caf::PdmField<QString>                 m_parsingStrictness;
    caf::PdmField<std::pair<bool, double>> m_relaxedMaxPvFraction;
    caf::PdmField<std::pair<bool, double>> m_solverMaxTimeStepInDays;
    caf::PdmField<std::pair<bool, double>> m_solverMinTimeStepInDays;
    caf::PdmField<std::pair<bool, int>>    m_minStrictCnvIter;
    caf::PdmField<std::pair<bool, int>>    m_minStrictMbIter;
    caf::PdmField<std::pair<bool, double>> m_minTimeStepBasedOnNewtonIterations;
    caf::PdmField<std::pair<bool, double>> m_minTimeStepBeforeShuttingProblematicWellsInDays;
    caf::PdmField<int>                     m_threadsPerProcess;

    // Future TODO:
    //    --tolerance-cnv=SCALAR                        Local convergence tolerance (Maximum of local saturation errors). Default: 0.01
    //    --tolerance-cnv-energy=SCALAR                 Local energy convergence tolerance (Maximum of local energy errors). Default: 0.01
    //    --tolerance-cnv-energy-relaxed=SCALAR         Relaxed local energy convergence tolerance that applies for iterations after the
    //    iterations with the strict tolerance. Default: 1
    //    --tolerance-cnv-relaxed=SCALAR                Relaxed local convergence tolerance that applies for iterations after the iterations
    //    with the strict tolerance. Default: 1
    //    --tolerance-energy-balance=SCALAR             Tolerated energy balance error relative to (scaled) total energy present. Default:
    //    1e-07
    //    --tolerance-energy-balance-relaxed=SCALAR     Relaxed tolerated energy balance error that applies for iterations after the
    //    iterations with the strict tolerance. Default: 1e-06
    //    --tolerance-mb=SCALAR                         Tolerated mass balance error relative to total mass present. Default: 1e-07
    //    --tolerance-mb-relaxed=SCALAR                 Relaxed tolerated mass balance error that applies for iterations after the
    //    iterations with the strict tolerance. Default: 1e-06
    //    --tolerance-pressure-ms-wells=SCALAR          Tolerance for the pressure equations for multi-segment wells. Default: 1000
    //    --tolerance-well-control=SCALAR               Tolerance for the well control equations. Default: 1e-07
    //    --tolerance-wells=SCALAR                      Well convergence tolerance. Default: 0.0001
    //    --well-group-constraints-max-iterations=INTEGER  Maximum number of iterations in the well/group switching algorithm. Default: 1
    //
};
