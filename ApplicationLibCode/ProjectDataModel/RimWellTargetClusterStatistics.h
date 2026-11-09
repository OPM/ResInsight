/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-  Equinor ASA
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

#include "Well/RigWellTargetMapping.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

//==================================================================================================
///
/// Read-only statistics for one cluster (well target) found by a well target mapping
///
//==================================================================================================
class RimWellTargetClusterStatistics : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellTargetClusterStatistics();

    void setStatistics( const RigWellTargetMapping::ClusterStatistics& statistics );

private:
    caf::PdmField<int>    m_clusterId;
    caf::PdmField<int>    m_numCells;
    caf::PdmField<double> m_totalPorvSoil;
    caf::PdmField<double> m_totalPorvSgas;
    caf::PdmField<double> m_totalPorvSoilAndSgas;
    caf::PdmField<double> m_totalFipOil;
    caf::PdmField<double> m_totalFipGas;
    caf::PdmField<double> m_totalRfipOil;
    caf::PdmField<double> m_totalRfipGas;
    caf::PdmField<double> m_totalSfipOil;
    caf::PdmField<double> m_totalSfipGas;
    caf::PdmField<double> m_permeability;
    caf::PdmField<double> m_pressure;
};
