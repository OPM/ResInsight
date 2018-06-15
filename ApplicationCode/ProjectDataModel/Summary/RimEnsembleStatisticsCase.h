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


#pragma once

#include "RifEclipseSummaryAddress.h"

#include "RimSummaryCase.h"

class RifEnsembleStatisticsReader;
class RimEnsembleCurveSet;


//==================================================================================================
///  
//==================================================================================================
class RimEnsembleStatisticsCase : public RimSummaryCase
{
public:
    RimEnsembleStatisticsCase(RimEnsembleCurveSet* curveSet);

    const std::vector<time_t>&          timeSteps() const;
    const std::vector<double>&          p10() const;
    const std::vector<double>&          p50() const;
    const std::vector<double>&          p90() const;
    const std::vector<double>&          mean() const;

    bool                                hasP10Data() const {return !m_p10Data.empty(); }
    bool                                hasP50Data() const { return !m_p50Data.empty(); }
    bool                                hasP90Data() const { return !m_p90Data.empty(); }

    virtual QString                     caseName() override;
    virtual void                        createSummaryReaderInterface() override;
    virtual RifSummaryReaderInterface*  summaryReader() override;

    virtual void        updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath) override {}

    const RimEnsembleCurveSet*          curveSet() const;

    void                                calculate();

private:
    void                                calculate(const std::vector<RimSummaryCase*> sumCases, const RifEclipseSummaryAddress& inputAddress);
    void clearData();
    std::vector<RimSummaryCase*>        validSummaryCases(const std::vector<RimSummaryCase*> allSumCases, const RifEclipseSummaryAddress& inputAddress);
    void                                calculateStatistics(const std::vector<double>& values, double* p10, double* p50, double* p90, double* mean);

private:
    std::unique_ptr<RifEnsembleStatisticsReader> m_statisticsReader;
    RimEnsembleCurveSet* m_curveSet;

    std::vector<time_t>         m_timeSteps;
    std::vector<double>         m_p10Data;
    std::vector<double>         m_p50Data;
    std::vector<double>         m_p90Data;
    std::vector<double>         m_meanData;

    RifEclipseSummaryAddress    m_addressUsedInLastCalculation;
};
