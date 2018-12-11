/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RiaEclipseUnitTools.h"
#include "RiaWeightedMeanCalculator.h"
#include "RimWellPathAicdParameters.h"

#include <array>
#include <memory>

class RimWellPathValve;
class RicMswValve;

//==================================================================================================
///
//==================================================================================================
class RicMswValveAccumulator
{
public:
    RicMswValveAccumulator(RiaEclipseUnitTools::UnitSystem unitSystem) : m_unitSystem(unitSystem) {}
    virtual bool accumulateValveParameters(const RimWellPathValve* wellPathValve, size_t subValve, double contributionFraction) = 0;
    virtual void applyToSuperValve(std::shared_ptr<RicMswValve> valve) = 0;

protected:
    RiaEclipseUnitTools::UnitSystem m_unitSystem;
};

//==================================================================================================
///
//==================================================================================================
class RicMswICDAccumulator : public RicMswValveAccumulator
{
public:
    RicMswICDAccumulator(RiaEclipseUnitTools::UnitSystem unitSystem);
    bool accumulateValveParameters(const RimWellPathValve* wellPathValve, size_t subValve, double contributionFraction) override;
    void applyToSuperValve(std::shared_ptr<RicMswValve> valve) override;

private:
    RiaWeightedMeanCalculator<double> m_coefficientCalculator;
    double                            m_areaSum;
};

//==================================================================================================
///
//==================================================================================================
class RicMswAICDAccumulator : public RicMswValveAccumulator
{
public:
    RicMswAICDAccumulator(RiaEclipseUnitTools::UnitSystem unitSystem);
    bool accumulateValveParameters(const RimWellPathValve* wellPathValve, size_t subValve, double contributionFraction) override;
    void applyToSuperValve(std::shared_ptr<RicMswValve> valve) override;

private:
    bool m_valid;
    bool m_deviceOpen;
    std::array<RiaWeightedMeanCalculator<double>, AICD_NUM_PARAMS> m_meanCalculators;
    RiaWeightedMeanCalculator<double>                              m_lengthCalculator;
};