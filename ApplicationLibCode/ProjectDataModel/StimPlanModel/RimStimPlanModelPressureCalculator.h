/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelPropertyCalculator.h"
#include "RimStimPlanModelWellLogCalculator.h"

#include "RiaStimPlanModelDefines.h"

#include "cvfObject.h"

#include <vector>

class RigEclipseCaseData;
class RimEclipseInputPropertyCollection;
class RimEclipseResultDefinition;
class RigResultAccessor;

class RimStimPlanModelPressureCalculator : public RimStimPlanModelWellLogCalculator
{
public:
    RimStimPlanModelPressureCalculator( RimStimPlanModelCalculator* stimPlanModelCalculator );

    bool isMatching( RiaDefines::CurveProperty curveProperty ) const override;

protected:
    bool extractValuesForProperty( RiaDefines::CurveProperty curveProperty,
                                   const RimStimPlanModel*   stimPlanModel,
                                   int                       timeStep,
                                   std::vector<double>&      values,
                                   std::vector<double>&      measuredDepthValues,
                                   std::vector<double>&      tvDepthValues,
                                   double&                   rkbDiff ) const override;

    bool extractPressureDataFromTable( RiaDefines::CurveProperty curveProperty,
                                       const RimStimPlanModel*   stimPlanModel,
                                       std::vector<double>&      values,
                                       std::vector<double>&      measuredDepthValues,
                                       std::vector<double>&      tvDepthValues ) const;
};
