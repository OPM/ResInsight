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

#include "RimSummaryCurvesData.h"

#include "RiaGuiApplication.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaSummaryTools.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RimAsciiDataCurve.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

#include "cvfAssert.h"
#include "cvfMath.h"

#include <QMessageBox>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::populateTimeHistoryCurvesData( std::vector<RimGridTimeHistoryCurve*> curves, RimSummaryCurvesData* curvesData )
{
    CVF_ASSERT( curvesData );

    curvesData->clear();

    for ( RimGridTimeHistoryCurve* curve : curves )
    {
        if ( !curve->isChecked() ) continue;
        QString curveCaseName = curve->caseName();

        CurveData curveData = { curve->curveExportDescription( {} ), RifEclipseSummaryAddressDefines::CurveType::ACCUMULATED, curve->yValues() };

        curvesData->addCurveData( curveCaseName, "", curve->timeStepValues(), curveData );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::populateAsciiDataCurvesData( std::vector<RimAsciiDataCurve*> curves, RimSummaryCurvesData* curvesData )
{
    CVF_ASSERT( curvesData );

    curvesData->clear();

    for ( RimAsciiDataCurve* curve : curves )
    {
        if ( !curve->isChecked() ) continue;

        CurveData curveData = { curve->curveExportDescription( {} ), RifEclipseSummaryAddressDefines::CurveType::ACCUMULATED, curve->yValues() };

        curvesData->addCurveDataNoSearch( "", "", curve->timeSteps(), { curveData } );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurvesData::RimSummaryCurvesData()
    : resamplePeriod( RiaDefines::DateTimePeriod::NONE )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::clear()
{
    caseIds.clear();
    timeSteps.clear();
    allCurveData.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::addCurveData( const QString&             caseName,
                                         const QString&             ensembleName,
                                         const std::vector<time_t>& curvetimeSteps,
                                         const CurveData&           curveData )
{
    QString caseId            = createCaseId( caseName, ensembleName );
    size_t  existingCaseIndex = findCaseIndexForCaseId( caseId, curvetimeSteps.size() );

    if ( existingCaseIndex == cvf::UNDEFINED_SIZE_T )
    {
        caseIds.push_back( caseId );
        timeSteps.push_back( curvetimeSteps );
        allCurveData.push_back( { curveData } );
    }
    else
    {
        CVF_ASSERT( timeSteps[existingCaseIndex].size() == curveData.values.size() );

        allCurveData[existingCaseIndex].push_back( curveData );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::addCurveDataNoSearch( const QString&                caseName,
                                                 const QString&                ensembleName,
                                                 const std::vector<time_t>&    curvetimeSteps,
                                                 const std::vector<CurveData>& curveDataVector )
{
    QString caseId = createCaseId( caseName, ensembleName );

    caseIds.push_back( caseId );
    timeSteps.push_back( curvetimeSteps );
    allCurveData.push_back( curveDataVector );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurvesData::createTextForExport( const std::vector<RimSummaryCurve*>&         curves,
                                                   const std::vector<RimAsciiDataCurve*>&       asciiCurves,
                                                   const std::vector<RimGridTimeHistoryCurve*>& gridCurves,
                                                   RiaDefines::DateTimePeriod                   resamplingPeriod,
                                                   bool                                         showTimeAsLongString )
{
    if ( curves.empty() && asciiCurves.empty() && gridCurves.empty() ) return {};

    QString out;

    RimSummaryCurvesData summaryCurvesData;
    RimSummaryCurvesData summaryCurvesObsData;
    RimSummaryCurvesData gridCurvesData;
    RimSummaryCurvesData::populateSummaryCurvesData( curves, SummaryCurveType::CURVE_TYPE_SUMMARY, &summaryCurvesData );
    RimSummaryCurvesData::populateSummaryCurvesData( curves, SummaryCurveType::CURVE_TYPE_OBSERVED, &summaryCurvesObsData );
    RimSummaryCurvesData::populateTimeHistoryCurvesData( gridCurves, &gridCurvesData );

    RimSummaryCurvesData::appendToExportData( out, { summaryCurvesObsData }, showTimeAsLongString );

    std::vector<RimSummaryCurvesData> exportData( 2 );

    RimSummaryCurvesData::prepareCaseCurvesForExport( resamplingPeriod, ResampleAlgorithm::DATA_DECIDES, summaryCurvesData, &exportData[0] );
    RimSummaryCurvesData::prepareCaseCurvesForExport( resamplingPeriod, ResampleAlgorithm::PERIOD_END, gridCurvesData, &exportData[1] );

    RimSummaryCurvesData::appendToExportData( out, exportData, showTimeAsLongString );

    {
        // Handle observed data pasted into plot from clipboard

        RimSummaryCurvesData asciiCurvesData;
        RimSummaryCurvesData::populateAsciiDataCurvesData( asciiCurves, &asciiCurvesData );

        RimSummaryCurvesData::appendToExportData( out, { asciiCurvesData }, showTimeAsLongString );
    }

    return out;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurvesData::createTextForCrossPlotCurves( const std::vector<RimSummaryCurve*>& curves )
{
    QString text;

    for ( const auto& curve : curves )
    {
        const auto curveAddress = curve->curveAddress();
        const auto xAddress     = curveAddress.summaryAddressX();
        const auto yAddress     = curveAddress.summaryAddressY();

        const auto xValues = curve->valuesX();
        const auto yValues = curve->valuesY();

        if ( xValues.size() == yValues.size() )
        {
            text += curve->curveExportDescription( {} ) + "\n";

            text +=
                QString( "%1\t%2\n" ).arg( QString::fromStdString( xAddress.vectorName() ) ).arg( QString::fromStdString( yAddress.vectorName() ) );

            for ( size_t i = 0; i < xValues.size(); i++ )
            {
                QString line;
                line += QString::number( xValues[i], 'g', RimSummaryPlot::precision() );
                line += "\t";
                line += QString::number( yValues[i], 'g', RimSummaryPlot::precision() );
                line += "\n";

                text += line;
            }
        }

        text += "\n";
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::populateSummaryCurvesData( std::vector<RimSummaryCurve*> curves,
                                                      SummaryCurveType              curveType,
                                                      RimSummaryCurvesData*         curvesData )
{
    CVF_ASSERT( curvesData );

    curvesData->clear();

    for ( RimSummaryCurve* curve : curves )
    {
        bool isObservedCurve = curve->summaryCaseY() ? curve->summaryCaseY()->isObservedData() : false;

        // Make sure a regression curve can be resampled https://github.com/OPM/ResInsight/issues/11372
        if ( curve->isRegressionCurve() ) isObservedCurve = false;

        if ( !curve->isChecked() ) continue;
        if ( isObservedCurve && ( curveType != SummaryCurveType::CURVE_TYPE_OBSERVED ) ) continue;
        if ( !isObservedCurve && ( curveType != SummaryCurveType::CURVE_TYPE_SUMMARY ) ) continue;
        if ( !curve->summaryCaseY() ) continue;

        QString curveCaseName = curve->summaryCaseY()->displayCaseName();
        QString ensembleName;
        if ( curve->curveDefinition().ensemble() )
        {
            ensembleName = curve->curveDefinition().ensemble()->name();
        }

        CurveData curveData = { curve->curveExportDescription( {} ), curve->curveType(), curve->valuesY() };
        CurveData errorCurveData;

        // Error data
        auto errorValues  = curve->errorValuesY();
        bool hasErrorData = !errorValues.empty();

        if ( hasErrorData )
        {
            errorCurveData.name      = curve->curveExportDescription( curve->errorSummaryAddressY() );
            errorCurveData.curveType = curve->curveType();
            errorCurveData.values    = errorValues;
        }

        auto curveDataList = std::vector<CurveData>( { curveData } );
        if ( hasErrorData ) curveDataList.push_back( errorCurveData );
        if ( curve->summaryAddressY().isCalculated() )
        {
            // We have calculated data, and it we cannot assume identical time axis
            curvesData->addCurveDataNoSearch( curveCaseName, ensembleName, curve->timeStepsY(), curveDataList );
        }
        else
        {
            for ( const auto& cd : curveDataList )
            {
                curvesData->addCurveData( curveCaseName, ensembleName, curve->timeStepsY(), cd );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::prepareCaseCurvesForExport( RiaDefines::DateTimePeriod  period,
                                                       ResampleAlgorithm           algorithm,
                                                       const RimSummaryCurvesData& inputCurvesData,
                                                       RimSummaryCurvesData*       resultCurvesData )
{
    resultCurvesData->clear();

    if ( period != RiaDefines::DateTimePeriod::NONE )
    {
        // Prepare result data
        resultCurvesData->resamplePeriod = period;

        for ( size_t i = 0; i < inputCurvesData.caseIds.size(); i++ )
        {
            // Shortcuts to input data
            auto& caseId        = inputCurvesData.caseIds[i];
            auto& caseTimeSteps = inputCurvesData.timeSteps[i];
            auto& caseCurveData = inputCurvesData.allCurveData[i];

            // Prepare result data

            for ( auto& curveDataItem : caseCurveData )
            {
                const auto [resampledTime, resampledValues] =
                    RiaSummaryTools::resampledValuesForPeriod( curveDataItem.curveType, caseTimeSteps, curveDataItem.values, period );

                auto cd   = curveDataItem;
                cd.values = resampledValues;

                resultCurvesData->addCurveData( caseId, "", resampledTime, cd );
            }
        }
    }
    else
    {
        *resultCurvesData = inputCurvesData;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::appendToExportDataForCase( QString& out, const std::vector<time_t>& timeSteps, const std::vector<CurveData>& curveData )
{
    for ( size_t j = 0; j < timeSteps.size(); j++ ) // time steps & data points
    {
        if ( j == 0 )
        {
            out += "Date and time";
            for ( const auto& k : curveData ) // curves
            {
                out += "\t" + ( k.name );
            }
        }
        out += "\n";
        out += QDateTime::fromSecsSinceEpoch( timeSteps[j] ).toUTC().toString( "yyyy-MM-dd hh:mm:ss " );

        for ( const auto& k : curveData ) // curves
        {
            QString valueText;
            if ( j < k.values.size() )
            {
                valueText = QString::number( k.values[j], 'g', RimSummaryPlot::precision() );
            }
            out += "\t" + valueText.rightJustified( 13 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::appendToExportData( QString& out, const std::vector<RimSummaryCurvesData>& curvesData, bool showTimeAsLongString )
{
    RimSummaryCurvesData data = RimSummaryCurvesData::concatCurvesData( curvesData );

    if ( data.resamplePeriod != RiaDefines::DateTimePeriod::NONE )
    {
        time_t minTimeStep = std::numeric_limits<time_t>::max();
        time_t maxTimeStep = 0;

        for ( auto& timeSteps : data.timeSteps )
        {
            if ( !timeSteps.empty() )
            {
                if ( timeSteps.front() < minTimeStep ) minTimeStep = timeSteps.front();
                if ( timeSteps.back() > maxTimeStep ) maxTimeStep = timeSteps.back();
            }
        }

        auto allTimeSteps = RiaTimeHistoryCurveResampler::timeStepsFromTimeRange( data.resamplePeriod, minTimeStep, maxTimeStep );

        const size_t threshold = 50000;
        if ( allTimeSteps.size() > threshold && RiaGuiApplication::isRunning() )
        {
            QString questionStr = QString( "This operation will produce %1 text lines. Do you want to continue?" ).arg( allTimeSteps.size() );

            auto reply =
                QMessageBox::question( nullptr, "Summary Text Export", questionStr, QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
            if ( reply != QMessageBox::Yes ) return;
        }

        out += "\n\n";
        out += "Date and time";
        for ( size_t i = 0; i < data.caseIds.size(); i++ )
        {
            for ( auto& j : data.allCurveData[i] )
            {
                out += "\t" + j.name;
            }
        }
        out += "\n";

        std::vector<size_t> currIndexes( data.caseIds.size() );
        for ( auto& i : currIndexes )
            i = 0;

        for ( auto timeStep : allTimeSteps )
        {
            QDateTime timseStepUtc = QDateTime::fromSecsSinceEpoch( timeStep ).toUTC();
            QString   timeText;

            if ( showTimeAsLongString )
            {
                timeText = timseStepUtc.toString( "yyyy-MM-dd hh:mm:ss " );
            }
            else
            {
                // Subtract one day to make sure the period is reported using the previous period as label
                QDateTime oneDayEarlier = timseStepUtc.addDays( -1 );

                QChar zeroChar( 48 );

                switch ( data.resamplePeriod )
                {
                    default:
                        // Fall through to NONE
                    case RiaDefines::DateTimePeriod::NONE:
                        timeText = timseStepUtc.toString( "yyyy-MM-dd hh:mm:ss " );
                        break;
                    case RiaDefines::DateTimePeriod::DAY:
                        timeText = oneDayEarlier.toString( "yyyy-MM-dd " );
                        break;
                    case RiaDefines::DateTimePeriod::WEEK:
                    {
                        timeText       = oneDayEarlier.toString( "yyyy" );
                        int weekNumber = oneDayEarlier.date().weekNumber();
                        timeText += QString( "-W%1" ).arg( weekNumber, 2, 10, zeroChar );
                        break;
                    }
                    case RiaDefines::DateTimePeriod::MONTH:
                        timeText = oneDayEarlier.toString( "yyyy-MM" );
                        break;
                    case RiaDefines::DateTimePeriod::QUARTER:
                    {
                        int quarterNumber = oneDayEarlier.date().month() / 3;
                        timeText          = oneDayEarlier.toString( "yyyy" );
                        timeText += QString( "-Q%1" ).arg( quarterNumber );
                        break;
                    }
                    case RiaDefines::DateTimePeriod::HALFYEAR:
                    {
                        int halfYearNumber = oneDayEarlier.date().month() / 6;
                        timeText           = oneDayEarlier.toString( "yyyy" );
                        timeText += QString( "-H%1" ).arg( halfYearNumber );
                        break;
                    }
                    case RiaDefines::DateTimePeriod::YEAR:
                        timeText = oneDayEarlier.toString( "yyyy" );
                        break;
                    case RiaDefines::DateTimePeriod::DECADE:
                        timeText = oneDayEarlier.toString( "yyyy" );
                        break;
                }
            }
            out += timeText;

            for ( size_t i = 0; i < data.caseIds.size(); i++ ) // cases
            {
                // Check is time step exists in curr case
                size_t& currIndex      = currIndexes[i];
                bool    timeStepExists = currIndex < data.timeSteps[i].size() && timeStep == data.timeSteps[i][currIndex];

                for ( auto& j : data.allCurveData[i] ) // vectors
                {
                    QString valueText;
                    if ( timeStepExists )
                    {
                        valueText = QString::number( j.values[currIndex], 'g', RimSummaryPlot::precision() );
                    }
                    else
                    {
                        valueText = "NULL";
                    }
                    out += "\t" + valueText.rightJustified( 13 );
                }

                if ( timeStepExists && currIndex < data.timeSteps[i].size() ) currIndex++;
            }
            out += "\n";
        }
    }
    else
    {
        for ( size_t i = 0; i < data.caseIds.size(); i++ )
        {
            out += "\n\n";
            if ( !data.caseIds[i].isEmpty() )
            {
                out += "Case: " + data.caseIds[i];
                out += "\n";
            }

            appendToExportDataForCase( out, data.timeSteps[i], data.allCurveData[i] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurvesData RimSummaryCurvesData::concatCurvesData( const std::vector<RimSummaryCurvesData>& curvesData )
{
    CVF_ASSERT( !curvesData.empty() );

    RiaDefines::DateTimePeriod period = curvesData.front().resamplePeriod;
    RimSummaryCurvesData       resultCurvesData;

    resultCurvesData.resamplePeriod = period;

    for ( auto curvesDataItem : curvesData )
    {
        if ( curvesDataItem.caseIds.empty() ) continue;

        CVF_ASSERT( curvesDataItem.resamplePeriod == period );

        resultCurvesData.caseIds.insert( resultCurvesData.caseIds.end(), curvesDataItem.caseIds.begin(), curvesDataItem.caseIds.end() );
        resultCurvesData.timeSteps.insert( resultCurvesData.timeSteps.end(), curvesDataItem.timeSteps.begin(), curvesDataItem.timeSteps.end() );
        resultCurvesData.allCurveData.insert( resultCurvesData.allCurveData.end(),
                                              curvesDataItem.allCurveData.begin(),
                                              curvesDataItem.allCurveData.end() );
    }
    return resultCurvesData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryCurvesData::findCaseIndexForCaseId( const QString& caseId, size_t timeStepCount )
{
    size_t casePosInList = cvf::UNDEFINED_SIZE_T;

    for ( size_t i = 0; i < caseIds.size(); i++ )
    {
        if ( caseId == caseIds[i] && timeSteps[i].size() == timeStepCount ) casePosInList = i;
    }

    return casePosInList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurvesData::createCaseId( const QString& caseName, const QString& ensembleName )
{
    QString caseId = caseName;
    if ( !ensembleName.isEmpty() ) caseId += QString( " (%1)" ).arg( ensembleName );

    return caseId;
}
