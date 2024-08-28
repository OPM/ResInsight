/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor
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
#include "RifEclipseSummaryAddress.h"

#include <QString>

class RimSummaryCurve;
class RimGridTimeHistoryCurve;
class RimAsciiDataCurve;

struct CurveData
{
    QString                                    name;
    RifEclipseSummaryAddressDefines::CurveType curveType;
    std::vector<double>                        values;
};

enum class SummaryCurveType
{
    CURVE_TYPE_SUMMARY  = 0x1,
    CURVE_TYPE_OBSERVED = 0x2
};

enum class ResampleAlgorithm
{
    NONE,
    DATA_DECIDES,
    PERIOD_END
};

class RimSummaryCurvesData
{
public:
    RimSummaryCurvesData();
    void clear();

    void addCurveData( const QString& caseName, const QString& ensembleName, const std::vector<time_t>& curvetimeSteps, const CurveData& curveData );

    void addCurveDataNoSearch( const QString&                caseName,
                               const QString&                ensembleName,
                               const std::vector<time_t>&    curvetimeSteps,
                               const std::vector<CurveData>& curveDataVector );

    static QString createTextForExport( const std::vector<RimSummaryCurve*>&         curves,
                                        const std::vector<RimAsciiDataCurve*>&       asciiCurves,
                                        const std::vector<RimGridTimeHistoryCurve*>& gridCurves,
                                        RiaDefines::DateTimePeriod                   resamplingPeriod,
                                        bool                                         showTimeAsLongString );

    static QString createTextForCrossPlotCurves( const std::vector<RimSummaryCurve*>& curves );

private:
    static void populateSummaryCurvesData( std::vector<RimSummaryCurve*> curves, SummaryCurveType curveType, RimSummaryCurvesData* curvesData );
    static void populateTimeHistoryCurvesData( std::vector<RimGridTimeHistoryCurve*> curves, RimSummaryCurvesData* curvesData );
    static void populateAsciiDataCurvesData( std::vector<RimAsciiDataCurve*> curves, RimSummaryCurvesData* curvesData );

    static void prepareCaseCurvesForExport( RiaDefines::DateTimePeriod  period,
                                            ResampleAlgorithm           algorithm,
                                            const RimSummaryCurvesData& inputCurvesData,
                                            RimSummaryCurvesData*       resultCurvesData );

    static void appendToExportDataForCase( QString& out, const std::vector<time_t>& timeSteps, const std::vector<CurveData>& curveData );
    static void appendToExportData( QString& out, const std::vector<RimSummaryCurvesData>& curvesData, bool showTimeAsLongString );
    RimSummaryCurvesData static concatCurvesData( const std::vector<RimSummaryCurvesData>& curvesData );

private:
    size_t findCaseIndexForCaseId( const QString& caseId, size_t timeStepCount );

    QString createCaseId( const QString& caseName, const QString& ensembleName );

public:
    RiaDefines::DateTimePeriod          resamplePeriod;
    std::vector<QString>                caseIds;
    std::vector<std::vector<time_t>>    timeSteps;
    std::vector<std::vector<CurveData>> allCurveData;
};
