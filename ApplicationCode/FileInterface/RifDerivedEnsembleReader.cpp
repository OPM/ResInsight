/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RifDerivedEnsembleReader.h"

#include "RimDerivedEnsembleCase.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t> RifDerivedEnsembleReader::EMPTY_TIME_STEPS_VECTOR;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifDerivedEnsembleReader::RifDerivedEnsembleReader(RimDerivedEnsembleCase* derivedCase)
{
    CVF_ASSERT(derivedCase);

    m_derivedCase = derivedCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifDerivedEnsembleReader::timeSteps(const RifEclipseSummaryAddress& resultAddress) const
{
    if (!resultAddress.isValid()) return EMPTY_TIME_STEPS_VECTOR;
    if (m_derivedCase->needsCalculation(resultAddress))
    {
        m_derivedCase->calculate(resultAddress);
    }
    return m_derivedCase->timeSteps(resultAddress);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifDerivedEnsembleReader::values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const
{
    if (!resultAddress.isValid()) return false;

    if (m_derivedCase->needsCalculation(resultAddress))
    {
        m_derivedCase->calculate(resultAddress);
    }
    auto dataValues = m_derivedCase->values(resultAddress);
    values->clear();
    values->reserve(dataValues.size());
    for (auto val : dataValues) values->push_back(val);
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifDerivedEnsembleReader::unitName(const RifEclipseSummaryAddress& resultAddress) const
{
    return "";
}
