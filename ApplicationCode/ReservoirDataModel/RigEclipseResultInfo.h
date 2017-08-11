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

#pragma once

#include "RiaDefines.h"

#include <QDateTime>
#include <QString>

#include <vector>

//==================================================================================================
/// 
//==================================================================================================
class RigEclipseTimeStepInfo
{
public:
    RigEclipseTimeStepInfo(const QDateTime& date, int reportNumber, double daysSinceSimulationStart);

    static std::vector<RigEclipseTimeStepInfo> createTimeStepInfos(std::vector<QDateTime> dates,
                                                            std::vector<int> reportNumbers,
                                                            std::vector<double> daysSinceSimulationStarts);
public:
    QDateTime   m_date;
    int         m_reportNumber;
    double      m_daysSinceSimulationStart;
};


//==================================================================================================
/// 
//==================================================================================================
class RigEclipseResultInfo
{
public:
    RigEclipseResultInfo(RiaDefines::ResultCatType resultType, bool needsToBeStored, bool mustBeCalculated,
               QString resultName, size_t gridScalarResultIndex);

    std::vector<QDateTime>  dates() const;
    std::vector<double>     daysSinceSimulationStarts() const;
    std::vector<int>        reportNumbers() const;

public:
    RiaDefines::ResultCatType   m_resultType;
    bool                        m_needsToBeStored;
    bool                        m_mustBeCalculated;
    QString                     m_resultName;
    size_t                      m_gridScalarResultIndex;
    
    std::vector<RigEclipseTimeStepInfo> m_timeStepInfos;
};
