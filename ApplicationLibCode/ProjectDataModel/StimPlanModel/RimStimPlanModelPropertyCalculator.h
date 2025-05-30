/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RiaStimPlanModelDefines.h"

#include <vector>

class RimStimPlanModelCalculator;
class RimStimPlanModel;

//==================================================================================================
///
//==================================================================================================
class RimStimPlanModelPropertyCalculator
{
public:
    virtual ~RimStimPlanModelPropertyCalculator() {};

    virtual bool isMatching( RiaDefines::CurveProperty curveProperty ) const = 0;
    virtual bool calculate( RiaDefines::CurveProperty curveProperty,
                            const RimStimPlanModel*   stimPlanModel,
                            int                       timeStep,
                            std::vector<double>&      values,
                            std::vector<double>&      measuredDepthValues,
                            std::vector<double>&      tvDepthValues,
                            double&                   rkbDiff ) const                          = 0;
};
