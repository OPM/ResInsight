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
    RigEclipseResultInfo(RiaDefines::ResultCatType resultType,
                         QString resultName,
                         bool needsToBeStored = false,
                         bool mustBeCalculated = false,
                         size_t gridScalarResultIndex = 0u);

    RiaDefines::ResultCatType   resultType() const;
    void                        setResultType(RiaDefines::ResultCatType newType);
    const QString&              resultName() const;
    void                        setResultName(const QString& name);
    bool                        needsToBeStored() const;
    void                        setNeedsToBeStored(bool store);    
    bool                        mustBeCalculated() const;
    void                        setMustBeCalculated(bool mustCalculate);
    size_t                      gridScalarResultIndex() const;
    void                        setGridScalarResultIndex(size_t index);
    
    const std::vector<RigEclipseTimeStepInfo>& timeStepInfos() const;
    void                                       setTimeStepInfos(const std::vector<RigEclipseTimeStepInfo>& timeSteps);

    std::vector<QDateTime>      dates() const;
    std::vector<double>         daysSinceSimulationStarts() const;
    std::vector<int>            reportNumbers() const;
    
private:
    RiaDefines::ResultCatType   m_resultType;
    bool                        m_needsToBeStored;
    bool                        m_mustBeCalculated;
    QString                     m_resultName;
    size_t                      m_gridScalarResultIndex;
    
    std::vector<RigEclipseTimeStepInfo> m_timeStepInfos;
};
