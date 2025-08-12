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

#include "RiaDateTimeDefines.h"
#include "RiaDefines.h"

#include "RifSummaryReaderInterface.h"

#include "RimSummaryCase.h"

class RifEclipseSummaryAddress;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleStatisticsCase : public RimSummaryCase, public RifSummaryReaderInterface
{
public:
    bool hasP10Data() const;
    bool hasP50Data() const;
    bool hasP90Data() const;
    bool hasMeanData() const;

    QString                       caseName() const override;
    void                          createSummaryReaderInterface() override;
    RifSummaryReaderInterface*    summaryReader() override;
    RiaDefines::EclipseUnitSystem unitSystem() const override;

    void calculate( const std::vector<RimSummaryCase*>& summaryCases, const RifEclipseSummaryAddress& inputAddress, bool includeIncompleteCurves );

    std::vector<time_t>                  timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;
    std::pair<bool, std::vector<double>> values( const RifEclipseSummaryAddress& resultAddress ) const override;
    std::string                          unitName( const RifEclipseSummaryAddress& resultAddress ) const override;

    static std::pair<time_t, time_t>  findMinMaxTime( const std::vector<RimSummaryCase*>& sumCases,
                                                      const RifEclipseSummaryAddress&     inputAddress );
    static RiaDefines::DateTimePeriod findBestResamplingPeriod( time_t minTimeStep, time_t maxTimeStep );

private:
    void   clearData();
    size_t keywordCount() const override;

private:
    std::vector<time_t> m_timeSteps;
    std::vector<double> m_p10Data;
    std::vector<double> m_p50Data;
    std::vector<double> m_p90Data;
    std::vector<double> m_meanData;

    caf::PdmPointer<RimSummaryCase> m_firstSummaryCase;
    size_t                          m_hash = 0;
};
