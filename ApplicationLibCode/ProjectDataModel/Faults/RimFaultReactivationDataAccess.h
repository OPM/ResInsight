/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include "cvfObject.h"
#include "cvfVector3.h"

#include <map>

class RimEclipseCase;
class RigEclipseCaseData;
class RigResultAccessor;
class RigMainGrid;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccess
{
public:
    RimFaultReactivationDataAccess( RimEclipseCase* thecase, size_t timeStepIndex );
    ~RimFaultReactivationDataAccess();

    void useCellIndexAdjustment( std::map<size_t, size_t> adjustments );

    double porePressureAtPosition( cvf::Vec3d position, double defaultPorePressureGradient );

    size_t timeStepIndex() const;

protected:
    double calculatePorePressure( double depth, double gradient );

private:
    RimEclipseCase*             m_case;
    RigEclipseCaseData*         m_caseData;
    const RigMainGrid*          m_mainGrid;
    size_t                      m_timeStepIndex;
    cvf::ref<RigResultAccessor> m_resultAccessor;
    std::map<size_t, size_t>    m_cellIndexAdjustment;
};
