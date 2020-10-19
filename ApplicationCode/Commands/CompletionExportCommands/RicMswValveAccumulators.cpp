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
#include "RicMswValveAccumulators.h"

#include "RiaStatisticsTools.h"

#include "RicMswCompletions.h"

#include "RimPerforationInterval.h"
#include "RimWellPathValve.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswICDAccumulator::RicMswICDAccumulator( std::shared_ptr<RicMswValve> valve, RiaEclipseUnitTools::UnitSystem unitSystem )
    : RicMswValveAccumulator( valve, unitSystem )
    , m_areaSum( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMswICDAccumulator::accumulateValveParameters( const RimWellPathValve* wellPathValve,
                                                      double                  overlapLength,
                                                      double                  perforationCompsegsLength )
{
    const double eps = 1.0e-8;

    CVF_ASSERT( wellPathValve );
    if ( wellPathValve->componentType() == RiaDefines::WellPathComponentType::ICV ||
         wellPathValve->componentType() == RiaDefines::WellPathComponentType::ICD )
    {
        size_t nICDs            = wellPathValve->valveLocations().size();
        double icdOrificeRadius = wellPathValve->orificeDiameter( m_unitSystem ) / 2;
        double icdArea          = icdOrificeRadius * icdOrificeRadius * cvf::PI_D;
        double totalIcdArea     = static_cast<double>( nICDs ) * icdArea;

        double icdAreaFactor = totalIcdArea * overlapLength / perforationCompsegsLength;

        if ( icdAreaFactor > eps )
        {
            m_valid = true;
            m_areaSum += icdAreaFactor;

            m_coefficientCalculator.addValueAndWeight( wellPathValve->flowCoefficient(), icdAreaFactor );
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswICDAccumulator::applyToSuperValve()
{
    std::shared_ptr<RicMswWsegValve> icd = std::dynamic_pointer_cast<RicMswWsegValve>( m_valve );
    CVF_ASSERT( icd );

    if ( m_coefficientCalculator.validAggregatedWeight() && m_valid )
    {
        icd->setIsValid( m_valid );
        icd->setArea( m_areaSum );
        icd->setFlowCoefficient( m_coefficientCalculator.weightedMean() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswAICDAccumulator::RicMswAICDAccumulator( std::shared_ptr<RicMswValve> valve, RiaEclipseUnitTools::UnitSystem unitSystem )
    : RicMswValveAccumulator( valve, unitSystem )
    , m_deviceOpen( false )
    , m_accumulatedLength( 0.0 )
    , m_accumulatedFlowScalingFactorDivisor( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMswAICDAccumulator::accumulateValveParameters( const RimWellPathValve* wellPathValve,
                                                       double                  overlapLength,
                                                       double                  perforationCompsegsLength )
{
    const double eps = 1.0e-8;

    CVF_ASSERT( wellPathValve );
    if ( wellPathValve->componentType() == RiaDefines::WellPathComponentType::AICD && overlapLength > eps )
    {
        const RimWellPathAicdParameters* params = wellPathValve->aicdParameters();
        if ( params->isValid() )
        {
            m_valid      = true;
            m_deviceOpen = m_deviceOpen || params->isOpen();
            if ( params->isOpen() )
            {
                std::array<double, AICD_NUM_PARAMS> values = params->doubleValues();
                for ( size_t i = 0; i < (size_t)AICD_NUM_PARAMS; ++i )
                {
                    if ( RiaStatisticsTools::isValidNumber( values[i] ) )
                    {
                        m_meanCalculators[i].addValueAndWeight( values[i], overlapLength );
                    }
                }

                m_accumulatedLength += overlapLength / perforationCompsegsLength;

                // https://github.com/OPM/ResInsight/issues/6126
                //
                // flowScalingFactor =  1 / (lengthFraction * aicdCount)
                // where:
                // lengthFraction = length_COMPSEGS / Sum_length_COMPSEGS_for_valve
                // N_AICDs = number of AICDs in perforation interval
                size_t aicdCount      = wellPathValve->valveLocations().size();
                double lengthFraction = overlapLength / perforationCompsegsLength;
                double divisor        = lengthFraction * aicdCount;
                m_accumulatedFlowScalingFactorDivisor += divisor;

                return true;
            }
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswAICDAccumulator::applyToSuperValve()
{
    const double                           eps  = 1.0e-8;
    std::shared_ptr<RicMswPerforationAICD> aicd = std::dynamic_pointer_cast<RicMswPerforationAICD>( m_valve );

    if ( aicd && m_valid && m_accumulatedLength > eps )
    {
        std::array<double, AICD_NUM_PARAMS> values;

        for ( size_t i = 0; i < (size_t)AICD_NUM_PARAMS; ++i )
        {
            if ( m_meanCalculators[i].validAggregatedWeight() )
            {
                values[i] = m_meanCalculators[i].weightedMean();
            }
            else
            {
                values[i] = std::numeric_limits<double>::infinity();
            }
        }
        aicd->setIsValid( m_valid );
        aicd->setIsOpen( m_deviceOpen );
        aicd->setLength( m_accumulatedLength );

        // See https://github.com/OPM/ResInsight/issues/6126
        double flowScalingFactor = 0.0;
        if ( m_accumulatedFlowScalingFactorDivisor > eps )
        {
            flowScalingFactor = 1.0 / m_accumulatedFlowScalingFactorDivisor;
        }

        aicd->setflowScalingFactor( flowScalingFactor );

        aicd->values() = values;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswAICDAccumulator::accumulatedLength() const
{
    return m_accumulatedLength;
}
