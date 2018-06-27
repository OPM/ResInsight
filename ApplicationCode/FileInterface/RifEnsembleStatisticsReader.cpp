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

#include "RifEnsembleStatisticsReader.h"

#include "RimEnsembleStatisticsCase.h"


static const std::vector<time_t> EMPTY_TIME_STEPS_VECTOR;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEnsembleStatisticsReader::RifEnsembleStatisticsReader(RimEnsembleStatisticsCase* ensStatCase)
{
    CVF_ASSERT(ensStatCase);

    m_ensembleStatCase = ensStatCase;

    m_allResultAddresses = std::set<RifEclipseSummaryAddress>(
        {
            RifEclipseSummaryAddress::ensembleStatisticsAddress(ENSEMBLE_STAT_P10_QUANTITY_NAME, ""),
            RifEclipseSummaryAddress::ensembleStatisticsAddress(ENSEMBLE_STAT_P50_QUANTITY_NAME, ""),
            RifEclipseSummaryAddress::ensembleStatisticsAddress(ENSEMBLE_STAT_P90_QUANTITY_NAME, ""),
            RifEclipseSummaryAddress::ensembleStatisticsAddress(ENSEMBLE_STAT_MEAN_QUANTITY_NAME, "")
        });
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifEnsembleStatisticsReader::timeSteps(const RifEclipseSummaryAddress& resultAddress) const
{
    if (!validateAddress(resultAddress)) return EMPTY_TIME_STEPS_VECTOR;
    return m_ensembleStatCase->timeSteps();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEnsembleStatisticsReader::values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const
{
    if (!validateAddress(resultAddress)) return false;

    const std::vector<double>* sourceData = nullptr;
    auto quantityName = resultAddress.ensembleStatisticsQuantityName();

    if (quantityName == ENSEMBLE_STAT_P10_QUANTITY_NAME)        sourceData = &m_ensembleStatCase->p10();
    else if (quantityName == ENSEMBLE_STAT_P50_QUANTITY_NAME)   sourceData = &m_ensembleStatCase->p50();
    else if (quantityName == ENSEMBLE_STAT_P90_QUANTITY_NAME)   sourceData = &m_ensembleStatCase->p90();
    else if (quantityName == ENSEMBLE_STAT_MEAN_QUANTITY_NAME)  sourceData = &m_ensembleStatCase->mean();

    if (!sourceData) return false;

    values->clear();
    values->reserve(sourceData->size());
    for (auto val : *sourceData) values->push_back(val);
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifEnsembleStatisticsReader::unitName(const RifEclipseSummaryAddress& resultAddress) const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEnsembleStatisticsReader::validateAddress(const RifEclipseSummaryAddress& address) const
{
    return address.category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS &&
        !address.quantityName().empty();
}
