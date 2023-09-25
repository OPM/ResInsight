/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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
class RimEnsembleCrossPlotStatisticsCase : public RimSummaryCase, public RifSummaryReaderInterface
{
public:
    RimEnsembleCrossPlotStatisticsCase();

    void calculate( const std::vector<RimSummaryCase*>& sumCases,
                    const RifEclipseSummaryAddress&     inputAddressX,
                    const RifEclipseSummaryAddress&     inputAddressY,
                    bool                                includeIncompleteCurves,
                    int                                 binCount,
                    int                                 sampleCountThreshold );

    bool hasP10Data() const { return !m_p10Data.empty(); }
    bool hasP50Data() const { return !m_p50Data.empty(); }
    bool hasP90Data() const { return !m_p90Data.empty(); }
    bool hasMeanData() const { return !m_meanData.empty(); }

    QString                       caseName() const override;
    void                          createSummaryReaderInterface() override;
    RifSummaryReaderInterface*    summaryReader() override;
    RiaDefines::EclipseUnitSystem unitSystem() const override;

    std::vector<time_t> timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;
    bool                values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const override;
    std::string         unitName( const RifEclipseSummaryAddress& resultAddress ) const override;

private:
    void clearData();

private:
    std::vector<double> m_p10Data;
    std::vector<double> m_p50Data;
    std::vector<double> m_p90Data;
    std::vector<double> m_meanData;

    RifEclipseSummaryAddress m_adrX;
    RifEclipseSummaryAddress m_adrY;

    std::vector<double> m_binnedXValues;

    caf::PdmPointer<RimSummaryCase> m_firstSummaryCase;
};
