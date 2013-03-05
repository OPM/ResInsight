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

#include "RigEclipseCase.h"

#include <vector>
#include <math.h>

class RigStatisticsEvaluator
{
public:
    RigStatisticsEvaluator(const std::vector<double>& values)
        : m_values(values),
        m_min(HUGE_VAL),
        m_max(-HUGE_VAL),
        m_mean(cvf::UNDEFINED_DOUBLE),
        m_dev(0.0)
    {
    }


    void getStatistics(double& min, double& max, double& mean, double& dev)
    {
        evaluate();

        min = m_min;
        max = m_max;
        mean = m_mean;
        dev = m_dev;
    }

private:
    void evaluate()
    {
        double sum = 0.0;
        double sumSquared = 0.0;

        size_t validValueCount = 0;

        for (size_t i = 0; i < m_values.size(); i++)
        {
            double val = m_values[i];
            if (val == HUGE_VAL) continue;

            validValueCount++;

            if (val < m_min) m_min = val;
            if (val > m_max) m_max = val;

            sum += val;
            sumSquared += (val * val);
        }

        if (validValueCount > 0)
        {
            m_mean = sum / validValueCount;


            // http://en.wikipedia.org/wiki/Standard_deviation#Rapid_calculation_methods
            // Running standard deviation

            double s0 = validValueCount;
            double s1 = sum;
            double s2 = sumSquared;

            m_dev = cvf::Math::sqrt( (s0 * s2) - (s1 * s1) ) / s0;
        }
    }


private:
    const std::vector<double>& m_values;

    double m_min;
    double m_max;
    double m_mean;
    double m_dev;
};



class RigStatisticsConfig
{
public:
    RigStatisticsConfig()
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


class RigStatistics
{
public:
    RigStatistics(cvf::Collection<RigEclipseCase>& sourceCases,
                  const std::vector<size_t>& timeStepIndices,
                  const RigStatisticsConfig& statisticsConfig,
                  RigEclipseCase* destinationCase);


    void evaluateStatistics(RimDefines::ResultCatType resultType, const QString& resultName);

    void debugOutput(RimDefines::ResultCatType resultType, const QString& resultName, size_t timeStepIdx);

private:
    void computeActiveCellUnion();
    void addNamedResult(RigReservoirCellResults* cellResults, RimDefines::ResultCatType resultType, const QString& resultName, size_t activeCellCount);
    void buildSourceMetaData( RimDefines::ResultCatType resultType, const QString& resultName );
private:
    cvf::Collection<RigEclipseCase> m_sourceCases;
    std::vector<size_t>             m_timeStepIndices;

    size_t                          m_globalCellCount;

    RigStatisticsConfig             m_statisticsConfig;

    cvf::ref<RigEclipseCase>        m_destinationCase;
};

