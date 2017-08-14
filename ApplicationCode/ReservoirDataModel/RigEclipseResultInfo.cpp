/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RigEclipseResultInfo.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigEclipseTimeStepInfo::RigEclipseTimeStepInfo(const QDateTime& date, int reportNumber, double daysSinceSimulationStart)
    : m_date(date),
    m_reportNumber(reportNumber),
    m_daysSinceSimulationStart(daysSinceSimulationStart)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseTimeStepInfo> RigEclipseTimeStepInfo::createTimeStepInfos(std::vector<QDateTime> dates,
                                                                  std::vector<int> reportNumbers,
                                                                  std::vector<double> daysSinceSimulationStarts)
{
    CVF_ASSERT(dates.size() == reportNumbers.size());
    CVF_ASSERT(dates.size() == daysSinceSimulationStarts.size());

    std::vector<RigEclipseTimeStepInfo> timeStepInfos;

    for (size_t i = 0; i < dates.size(); i++)
    {
        timeStepInfos.push_back(RigEclipseTimeStepInfo(dates[i], reportNumbers[i], daysSinceSimulationStarts[i]));
    }

    return timeStepInfos;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigEclipseResultInfo::RigEclipseResultInfo(RiaDefines::ResultCatType resultType, bool needsToBeStored, bool mustBeCalculated,
                       QString resultName, size_t gridScalarResultIndex)
    : m_resultType(resultType),
    m_needsToBeStored(needsToBeStored),
    m_mustBeCalculated(mustBeCalculated),
    m_resultName(resultName),
    m_gridScalarResultIndex(gridScalarResultIndex)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RigEclipseResultInfo::dates() const
{
    std::vector<QDateTime> values;

    for (auto v : m_timeStepInfos)
    {
        values.push_back(v.m_date);
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigEclipseResultInfo::daysSinceSimulationStarts() const
{
    std::vector<double> values;

    for (auto v : m_timeStepInfos)
    {
        values.push_back(v.m_daysSinceSimulationStart);
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> RigEclipseResultInfo::reportNumbers() const
{
    std::vector<int> values;

    for (auto v : m_timeStepInfos)
    {
        values.push_back(v.m_reportNumber);
    }

    return values;
}

