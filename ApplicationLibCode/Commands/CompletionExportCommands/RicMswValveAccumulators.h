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

#include "RiaDefines.h"
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
    RicMswValveAccumulator( std::shared_ptr<RicMswValve> valve, RiaDefines::EclipseUnitSystem unitSystem )
        : m_valve( valve )
        , m_unitSystem( unitSystem )
        , m_valid( false )
    {
    }
    virtual bool accumulateValveParameters( const RimWellPathValve* wellPathValve,
                                            double                  overlapLength,
                                            double                  perforationCompsegsLength ) = 0;
    virtual void applyToSuperValve()                                           = 0;

    std::shared_ptr<RicMswValve> superValve() const { return m_valve; }

protected:
    std::shared_ptr<RicMswValve>  m_valve;
    RiaDefines::EclipseUnitSystem m_unitSystem;
    bool                          m_valid;
};

//==================================================================================================
///
//==================================================================================================
class RicMswICDAccumulator : public RicMswValveAccumulator
{
public:
    RicMswICDAccumulator( std::shared_ptr<RicMswValve> valve, RiaDefines::EclipseUnitSystem unitSystem );
    bool accumulateValveParameters( const RimWellPathValve* wellPathValve,
                                    double                  overlapLength,
                                    double                  perforationCompsegsLength ) override;
    void applyToSuperValve() override;

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
    RicMswAICDAccumulator( std::shared_ptr<RicMswValve> valve, RiaDefines::EclipseUnitSystem unitSystem );
    bool   accumulateValveParameters( const RimWellPathValve* wellPathValve,
                                      double                  overlapLength,
                                      double                  perforationCompsegsLength ) override;
    void   applyToSuperValve() override;
    double accumulatedLength() const;

private:
    bool                                                           m_deviceOpen;
    std::array<RiaWeightedMeanCalculator<double>, AICD_NUM_PARAMS> m_meanCalculators;
    double                                                         m_accumulatedLength;
    double                                                         m_accumulatedFlowScalingFactorDivisor;
};
