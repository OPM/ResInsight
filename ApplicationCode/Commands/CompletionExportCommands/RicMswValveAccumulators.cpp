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
RicMswICDAccumulator::RicMswICDAccumulator( RiaEclipseUnitTools::UnitSystem unitSystem )
    : RicMswValveAccumulator( unitSystem )
    , m_areaSum( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMswICDAccumulator::accumulateValveParameters( const RimWellPathValve* wellPathValve,
                                                      size_t                  subValve,
                                                      double                  contributionFraction )
{
    CVF_ASSERT( wellPathValve );
    if ( wellPathValve->componentType() == RiaDefines::ICV || wellPathValve->componentType() == RiaDefines::ICD )
    {
        double icdOrificeRadius = wellPathValve->orificeDiameter( m_unitSystem ) / 2;
        double icdArea          = icdOrificeRadius * icdOrificeRadius * cvf::PI_D;

        m_areaSum += icdArea * contributionFraction;
        m_coefficientCalculator.addValueAndWeight( wellPathValve->flowCoefficient(), icdArea * contributionFraction );
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswICDAccumulator::applyToSuperValve( std::shared_ptr<RicMswValve> valve )
{
    std::shared_ptr<RicMswWsegValve> icd = std::dynamic_pointer_cast<RicMswWsegValve>( valve );
    CVF_ASSERT( icd );
    icd->setArea( m_areaSum );
    if ( m_coefficientCalculator.validAggregatedWeight() )
    {
        icd->setFlowCoefficient( m_coefficientCalculator.weightedMean() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswAICDAccumulator::RicMswAICDAccumulator( RiaEclipseUnitTools::UnitSystem unitSystem )
    : RicMswValveAccumulator( unitSystem )
    , m_valid( false )
    , m_deviceOpen( false )
    , m_accumulatedLength( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMswAICDAccumulator::accumulateValveParameters( const RimWellPathValve* wellPathValve,
                                                       size_t                  subValve,
                                                       double                  contributionFraction )
{
    CVF_ASSERT( wellPathValve );
    if ( wellPathValve->componentType() == RiaDefines::AICD )
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
                        m_meanCalculators[i].addValueAndWeight( values[i], contributionFraction );
                    }
                }
                std::pair<double, double> valveSegment       = wellPathValve->valveSegments()[subValve];
                double                    valveSegmentLength = std::fabs( valveSegment.second - valveSegment.first );
                const RimPerforationInterval* perfInterval   = nullptr;
                wellPathValve->firstAncestorOrThisOfTypeAsserted( perfInterval );
                double perfIntervalLength = std::fabs( perfInterval->endMD() - perfInterval->startMD() );
                double lengthFraction     = 1.0;
                if ( perfIntervalLength > 1.0e-8 )
                {
                    lengthFraction = valveSegmentLength / perfIntervalLength;
                }
                m_accumulatedLength += lengthFraction * contributionFraction;
            }
        }
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswAICDAccumulator::applyToSuperValve( std::shared_ptr<RicMswValve> valve )
{
    std::shared_ptr<RicMswPerforationAICD> aicd = std::dynamic_pointer_cast<RicMswPerforationAICD>( valve );

    if ( aicd )
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

        aicd->values() = values;
    }
}
