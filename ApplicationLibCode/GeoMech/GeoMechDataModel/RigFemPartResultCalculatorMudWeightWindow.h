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

#include "RigFemPartResultCalculator.h"

#include "RimMudWeightWindowParameters.h"

#include "cvfVector3.h"

#include <map>

class RigFemPartResultsCollection;
class RigFemScalarResultFrames;
class RigFemResultAddress;

//==================================================================================================
///
//==================================================================================================
class RigFemPartResultCalculatorMudWeightWindow : public RigFemPartResultCalculator
{
public:
    explicit RigFemPartResultCalculatorMudWeightWindow( RigFemPartResultsCollection& collection );
    ~RigFemPartResultCalculatorMudWeightWindow() override;
    bool                      isMatching( const RigFemResultAddress& resVarAddr ) const override;
    RigFemScalarResultFrames* calculate( int partIndex, const RigFemResultAddress& resVarAddr ) override;

    cvf::Vec3d calculateWellPathTangent( double azimuth, double inclination );

private:
    void loadParameterFramesOrValue( RimMudWeightWindowParameters::ParameterType parameterType,
                                     int                                         partIndex,
                                     std::map<RimMudWeightWindowParameters::ParameterType, RigFemScalarResultFrames*>& parameterFrames,
                                     std::map<RimMudWeightWindowParameters::ParameterType, float>& parameterValues );

    static std::vector<float>
        loadDataForFrame( RimMudWeightWindowParameters::ParameterType parameterType,
                          std::map<RimMudWeightWindowParameters::ParameterType, RigFemScalarResultFrames*>& parameterFrames,
                          int frameIndex );

    static float
        getValueForElement( RimMudWeightWindowParameters::ParameterType parameterType,
                            const std::map<RimMudWeightWindowParameters::ParameterType, std::vector<float>>& parameterFrameData,
                            const std::map<RimMudWeightWindowParameters::ParameterType, float> parameterValues,
                            int                                                                elmIdx );
};
