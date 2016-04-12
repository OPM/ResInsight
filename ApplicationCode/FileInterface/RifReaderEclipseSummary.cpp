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

#include "ecl_sum.h"

#include <string>
#include <assert.h>

#include <QDateTime>

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
        assert(ecl_sum_get_smspec(ecl_sum) != NULL);

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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifReaderEclipseSummary::variableNames() const
{
    assert(ecl_sum != NULL);

    // Get all possible variable names from file
    stringlist_type* keys = ecl_sum_alloc_matching_general_var_list(ecl_sum, NULL);
    stringlist_sort(keys, NULL);
    
    std::vector<std::string> names;
    RifReaderEclipseSummary::populateVectorFromStringList(keys, &names);

    stringlist_free(keys);

    return names;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseSummary::values(const std::string& variableName, std::vector<double>* values)
{
    assert(ecl_sum != NULL);

    int variableIndex = variableIndexFromVariableName(variableName);
    if (variableIndex < 0) return false;

    for (int time_index = 0; time_index < timeStepCount(); time_index++)
    {
        double value = ecl_sum_iget(ecl_sum, time_index, variableIndex);

        values->push_back(value);
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifReaderEclipseSummary::variableIndexFromVariableName(const std::string& keyword) const
{
    assert(ecl_sum != NULL);

    return ecl_sum_get_general_var_params_index(ecl_sum, keyword.data());
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
std::vector<std::string> RifReaderEclipseSummary::wellGroupNames() const
{
    assert(ecl_sum != NULL);

    std::vector<std::string> names;
    stringlist_type* stringList = ecl_sum_alloc_group_list(ecl_sum, NULL);
    RifReaderEclipseSummary::populateVectorFromStringList(stringList, &names);

    stringlist_free(stringList);

    return names;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifReaderEclipseSummary::wellNames() const
{
    assert(ecl_sum != NULL);

    std::vector<std::string> names;
    stringlist_type* stringList = ecl_sum_alloc_well_list(ecl_sum, NULL);
    RifReaderEclipseSummary::populateVectorFromStringList(stringList, &names);

    stringlist_free(stringList);

    return names;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifReaderEclipseSummary::wellVariableNames() const
{
    assert(ecl_sum != NULL);

    std::vector<std::string> names;
    stringlist_type* stringList = ecl_sum_alloc_well_var_list(ecl_sum);
    RifReaderEclipseSummary::populateVectorFromStringList(stringList, &names);
    stringlist_free(stringList);

    return names;
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
void RifReaderEclipseSummary::populateVectorFromStringList(stringlist_type* stringList, std::vector<std::string>* strings)
{
    assert(stringList && strings);

    for (int i = 0; i < stringlist_get_size(stringList); i++)
    {
        strings->push_back(stringlist_iget(stringList, i));
    }
}

