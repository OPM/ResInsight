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

#include <vector>

class QString;

class RigResultInfo
{
public:
    RigResultInfo(RiaDefines::ResultCatType resultType, bool needsToBeStored, bool mustBeCalculated,
               QString resultName, size_t gridScalarResultIndex);

public:
    RiaDefines::ResultCatType   m_resultType;
    bool                        m_needsToBeStored;
    bool                        m_mustBeCalculated;
    QString                     m_resultName;
    size_t                      m_gridScalarResultIndex;
    std::vector<QDateTime>      m_timeStepDates;
    std::vector<int>            m_timeStepReportNumbers;
    std::vector<double>         m_daysSinceSimulationStart;
};
