/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include <vector>

#include "cvfBase.h"
#include "cvfMatrix4.h"

class RimEclipseCase;
class RigFractureCell;

//==================================================================================================
///
//==================================================================================================

class RigEclipseToStimPlanCellTransmissibilityCalculator
{
public:
    explicit RigEclipseToStimPlanCellTransmissibilityCalculator(const RimEclipseCase* caseToApply,
                                                                cvf::Mat4f fractureTransform,
                                                                double skinFactor,
                                                                double cDarcy,
                                                                const RigFractureCell& stimPlanCell);

    const std::vector<size_t>&  globalIndeciesToContributingEclipseCells();
    const std::vector<double>&  contributingEclipseCellTransmissibilities();

private:
    void                        calculateStimPlanCellsMatrixTransmissibility();
    std::vector<size_t>         getPotentiallyFracturedCellsForPolygon(std::vector<cvf::Vec3d> polygon);


private:
    const RimEclipseCase*               m_case;
    double                              m_cDarcy;
    double                              m_fractureSkinFactor;
    cvf::Mat4f                          m_fractureTransform;
    const RigFractureCell&              m_stimPlanCell;

    std::vector<size_t>                 m_globalIndeciesToContributingEclipseCells;
    std::vector<double>                 m_contributingEclipseCellTransmissibilities;
};

