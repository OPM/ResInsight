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

#include "RimDefines.h"
#include "RimStatisticsCase.h"

#include <vector>
#include <math.h>

#include <QPair>

class RimCase;
class RigCaseData;
class RigCaseCellResultsData;


class RimStatisticsConfig
{
public:
    RimStatisticsConfig()
        : m_calculatePercentiles(true),
        m_pMinPos(10.0),
        m_pMidPos(50.0),
        m_pMaxPos(90.0),
        m_pValMethod(RimStatisticsCase::INTERPOLATED_OBSERVATION)
    {
    }

public:
    bool    m_calculatePercentiles;
    double  m_pMinPos;
    double  m_pMidPos;
    double  m_pMaxPos;
    RimStatisticsCase::PercentileCalcType m_pValMethod;
};


class RimStatisticsCaseEvaluator
{
public:
    RimStatisticsCaseEvaluator(const std::vector<RimCase*>& sourceCases,
                               const std::vector<size_t>& timeStepIndices,
                               const RimStatisticsConfig& statisticsConfig,
                               RigCaseData* destinationCase);

    struct ResSpec 
    {
        ResSpec() : m_resType(RimDefines::DYNAMIC_NATIVE), m_poroModel(RifReaderInterface::MATRIX_RESULTS) {}
        ResSpec( RifReaderInterface::PorosityModelResultType poroModel,
                 RimDefines::ResultCatType                   resType,
                 QString                                     resVarName) : m_poroModel(poroModel), m_resType(resType), m_resVarName(resVarName) {}

        RifReaderInterface::PorosityModelResultType m_poroModel;
        RimDefines::ResultCatType                   m_resType;
        QString                                     m_resVarName;
    };

    void evaluateForResults(const QList<ResSpec >& resultSpecification);

    void debugOutput(RimDefines::ResultCatType resultType, const QString& resultName, size_t timeStepIdx);

private:
    void addNamedResult(RigCaseCellResultsData* cellResults, RimDefines::ResultCatType resultType, const QString& resultName, size_t activeCellCount);
    void buildSourceMetaData(RifReaderInterface::PorosityModelResultType poroModel, RimDefines::ResultCatType resultType, const QString& resultName);

    enum StatisticsParamType { MIN, MAX, RANGE, MEAN, STDEV, PMIN, PMID, PMAX, STAT_PARAM_COUNT };

private:
    std::vector<RimCase*>  m_sourceCases;
    std::vector<size_t>    m_timeStepIndices;

    size_t                 m_globalCellCount;
    RimStatisticsConfig    m_statisticsConfig;
    RigCaseData*           m_destinationCase;
};

