/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RiaPorosityModel.h"

#include "RigEclipseToStimPlanCellTransmissibilityCalculator.h"

#include "cvfBase.h"
#include "cvfMatrix4.h"

#include <map>
#include <vector>

class QString;

class RimEclipseCase;
class RigFractureGrid;
class RigTransmissibilityCondenser;
class RimFracture;

//==================================================================================================
///
//==================================================================================================
class RigEclipseToStimPlanCalculator
{
public:
    explicit RigEclipseToStimPlanCalculator(const RimEclipseCase*  caseToApply,
                                            cvf::Mat4d             fractureTransform,
                                            double                 skinFactor,
                                            double                 cDarcy,
                                            const RigFractureGrid& fractureGrid);

    void appendDataToTransmissibilityCondenser(const RimFracture*            fracture,
                                               bool                          useFiniteConductivityInFracture,
                                               RigTransmissibilityCondenser* condenser) const;

    // Returns the area of each stimplan cell intersecting eclipse cells
    std::map<size_t, double> eclipseCellAreas() const;

private:
    void computeValues();

private:
    const RimEclipseCase*  m_case;
    double                 m_cDarcy;
    double                 m_fractureSkinFactor;
    cvf::Mat4d             m_fractureTransform;
    const RigFractureGrid& m_fractureGrid;

    std::vector<RigEclipseToStimPlanCellTransmissibilityCalculator> m_singleFractureCellCalculators;
};
