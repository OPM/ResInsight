#include "RicMswValveAccumulators.h"

#include "RicMswCompletions.h"

#include "RimWellPathValve.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswICDAccumulator::RicMswICDAccumulator(RiaEclipseUnitTools::UnitSystem unitSystem)
    : RicMswValveAccumulator(unitSystem)
    , m_areaSum(0.0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswICDAccumulator::accumulateValveParameters(const RimWellPathValve* wellPathValve, double contributionFraction)
{
    double icdOrificeRadius = wellPathValve->orificeDiameter(m_unitSystem) / 2;
    double icdArea          = icdOrificeRadius * icdOrificeRadius * cvf::PI_D;

    m_areaSum += icdArea * contributionFraction;
    m_coefficientCalculator.addValueAndWeight(wellPathValve->flowCoefficient(), icdArea * contributionFraction);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswICDAccumulator::applyToSuperValve(std::shared_ptr<RicMswValve> valve)
{
    std::shared_ptr<RicMswICD> icd = std::dynamic_pointer_cast<RicMswICD>(valve);
    CVF_ASSERT(icd);
    icd->setArea(m_areaSum);
    if (m_coefficientCalculator.validAggregatedWeight())
    {
        icd->setFlowCoefficient(m_coefficientCalculator.weightedMean());
    }
}


