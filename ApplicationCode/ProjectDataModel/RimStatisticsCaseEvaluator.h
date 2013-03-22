/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfCollection.h"
#include <vector>
#include <math.h>

#include <QPair>
#include "RimDefines.h"

class RimCase;
class RigCaseData;
class RigCaseCellResultsData;


class RimStatisticsConfig
{
public:
    RimStatisticsConfig()
        : m_min(true),
        m_max(true),
        m_mean(true),
        m_stdDev(true)
    {
    }

public:
    bool m_min;
    bool m_max;
    bool m_mean;
    bool m_stdDev;
};


class RimStatisticsCaseEvaluator
{
public:
    RimStatisticsCaseEvaluator(const std::vector<RimCase*>& sourceCases,
                               const std::vector<size_t>& timeStepIndices,
                               const RimStatisticsConfig& statisticsConfig,
                               RigCaseData* destinationCase);


    void evaluateForResults(const QList<QPair<RimDefines::ResultCatType, QString> >& resultSpecification);

    void debugOutput(RimDefines::ResultCatType resultType, const QString& resultName, size_t timeStepIdx);

private:
    void addNamedResult(RigCaseCellResultsData* cellResults, RimDefines::ResultCatType resultType, const QString& resultName, size_t activeCellCount);
    void buildSourceMetaData(RimDefines::ResultCatType resultType, const QString& resultName);

private:
    std::vector<RimCase*>  m_sourceCases;
    std::vector<size_t>         m_timeStepIndices;

    size_t                      m_globalCellCount;
    RimStatisticsConfig         m_statisticsConfig;
    RigCaseData*             m_destinationCase;
};

