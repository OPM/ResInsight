/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RifReaderEclipseSummary.h"

#include "ert/ecl/ecl_sum.h"

#include <string>
#include <assert.h>

#include <QDateTime>
#include "ert/ecl/smspec_node.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary::RifReaderEclipseSummary()
    : ecl_sum(NULL)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary::~RifReaderEclipseSummary()
{
    close();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseSummary::open(const std::string& headerFileName, const std::vector<std::string>& dataFileNames)
{
    assert(ecl_sum == NULL); 
    
    if (headerFileName.empty() || dataFileNames.size() == 0) return false;

    assert(!headerFileName.empty());
    assert(dataFileNames.size() > 0);

    stringlist_type* dataFiles = stringlist_alloc_new();
    for (size_t i = 0; i < dataFileNames.size(); i++)
    {
        stringlist_append_copy(dataFiles, dataFileNames[i].data());
    }

    std::string itemSeparatorInVariableNames = ":";
    ecl_sum = ecl_sum_fread_alloc(headerFileName.data(), dataFiles, itemSeparatorInVariableNames.data());

    stringlist_free(dataFiles);

    if (ecl_sum)
    {
        eclSmSpec = ecl_sum_get_smspec(ecl_sum);
        assert(eclSmSpec != NULL);

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::close()
{
    if (ecl_sum)
    {
        ecl_sum_free(ecl_sum);
        ecl_sum = NULL;
    }
}

RifEclipseSummaryAddress addressFromErtSmSpecNode(const smspec_node_type * ertSumVarNode)
{
    if (smspec_node_get_var_type(ertSumVarNode) == ECL_SMSPEC_INVALID_VAR) return RifEclipseSummaryAddress();

    RifEclipseSummaryAddress::SummaryVarCategory sumCategory(RifEclipseSummaryAddress::SUMMARY_INVALID);
    std::string        quantityName;
    int                regionNumber(-1);
    int                regionNumber2(-1);
    std::string        wellGroupName;
    std::string        wellName;
    int                wellSegmentNumber(-1);
    std::string        lgrName;
    int                cellI(-1);
    int                cellJ(-1);
    int                cellK(-1);

    quantityName = smspec_node_get_keyword(ertSumVarNode);

    switch (smspec_node_get_var_type(ertSumVarNode))
    {
        case ECL_SMSPEC_AQUIFER_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_AQUIFER;
        }
        break;
        case ECL_SMSPEC_WELL_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL;
            wellName = smspec_node_get_wgname(ertSumVarNode);
        }
        break;
        case ECL_SMSPEC_REGION_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_REGION;
            regionNumber = smspec_node_get_num(ertSumVarNode);
        }
        break;
        case ECL_SMSPEC_FIELD_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_FIELD;
        }
        break;
        case ECL_SMSPEC_GROUP_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL_GROUP;
            wellGroupName = smspec_node_get_wgname(ertSumVarNode);
        }
        break;
        case ECL_SMSPEC_BLOCK_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_BLOCK;
            // Todo: Get IJK values
        }
        break;
        case ECL_SMSPEC_COMPLETION_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION;
            wellName = smspec_node_get_wgname(ertSumVarNode);
            // Todo: Get IJK values
        }
        break;
        case ECL_SMSPEC_LOCAL_BLOCK_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR;
            // Todo: GetLGR name
            // Todo: Get IJK values
        }
        break;
        case ECL_SMSPEC_LOCAL_COMPLETION_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR;
            wellName = smspec_node_get_wgname(ertSumVarNode);
            // Todo: GetLGR name
            // Todo: Get IJK values
        }
        break;
        case ECL_SMSPEC_LOCAL_WELL_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL_LGR;
            wellName = smspec_node_get_wgname(ertSumVarNode);
            // Todo: GetLGR name
        }
        break;
        case ECL_SMSPEC_NETWORK_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_NETWORK;
        }
        break;
        case ECL_SMSPEC_REGION_2_REGION_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION;
            // Todo: Get R1 R2 values
        }
        break;
        case ECL_SMSPEC_SEGMENT_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT;
            wellSegmentNumber = smspec_node_get_num(ertSumVarNode);
        }
        break;
        case ECL_SMSPEC_MISC_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_MISC;
        }
        break;
        default:
            CVF_ASSERT(false);
        break;
    }

    return RifEclipseSummaryAddress(sumCategory, 
                                    quantityName, 
                                    regionNumber, 
                                    regionNumber2, 
                                    wellGroupName, 
                                    wellName, 
                                    wellSegmentNumber, 
                                    lgrName, 
                                    cellI, cellJ, cellK);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<RifEclipseSummaryAddress>& RifReaderEclipseSummary::allResultAddresses() 
{
    if (m_allResultAddresses.size() == 0)
    {
        int varCount = ecl_smspec_num_nodes( eclSmSpec);
        for(int i = 0; i < varCount; i++)
        {
            const smspec_node_type * ertSumVarNode = ecl_smspec_iget_node(eclSmSpec, i);

            m_allResultAddresses.push_back(addressFromErtSmSpecNode(ertSumVarNode));
        }
    }

    return m_allResultAddresses;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseSummary::values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values)
{
    assert(ecl_sum != NULL);
    values->clear();
    int tsCount = timeStepCount();
    values->reserve(timeStepCount());

    int variableIndex = indexFromAddress(resultAddress);

    if(variableIndex < 0) return false;

    const smspec_node_type * ertSumVarNode = ecl_smspec_iget_node(eclSmSpec, variableIndex);
    int paramsIndex = smspec_node_get_params_index(ertSumVarNode);

    for(int time_index = 0; time_index < tsCount; time_index++)
    {
        double value = ecl_sum_iget(ecl_sum, time_index, paramsIndex);
        values->push_back(value);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifReaderEclipseSummary::timeStepCount() const
{
    assert(ecl_sum != NULL);

    return ecl_sum_get_data_length(ecl_sum);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifReaderEclipseSummary::timeSteps() const
{
    assert(ecl_sum != NULL);

    std::vector<time_t> steps;
    for (int time_index = 0; time_index < timeStepCount(); time_index++)
    {
        time_t sim_time = ecl_sum_iget_sim_time(ecl_sum , time_index);
        steps.push_back(sim_time);
    }

    return steps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RifReaderEclipseSummary::fromTimeT(const std::vector<time_t>& timeSteps)
{
    std::vector<QDateTime> a;

    for (size_t i = 0; i < timeSteps.size(); i++)
    {
        QDateTime dt = QDateTime::fromTime_t(timeSteps[i]);
        a.push_back(dt);
    }

    return a;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifReaderEclipseSummary::indexFromAddress(const RifEclipseSummaryAddress& resultAddress)
{
    // For now, a slow linear search
    int resIndex = -1;
    int addrCount = static_cast<int>(this->allResultAddresses().size());
    for (int idx = 0; idx < addrCount;  ++idx)
    {
        if (m_allResultAddresses[idx] == resultAddress) return idx;
    }

    return -1;
}

#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::populateVectorFromStringList(stringlist_type* stringList, std::vector<std::string>* strings)
{
    assert(stringList && strings);

    for (int i = 0; i < stringlist_get_size(stringList); i++)
    {
        strings->push_back(stringlist_iget(stringList, i));
    }
}

#endif