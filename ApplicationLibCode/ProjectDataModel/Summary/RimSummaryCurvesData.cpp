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
#include "RiaQDateTimeTools.h"
#include "RiaTimeHistoryCurveResampler.h"
#include "Summary/RiaSummaryCurveDefinition.h"
#include "Summary/RiaSummaryTools.h"

#include "RimAsciiDataCurve.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

#include "cafAssert.h"
#include "cvfMath.h"

#include <QMessageBox>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Header text for the time column, either date and time or time from simulation start
//--------------------------------------------------------------------------------------------------
QString timeColumnHeaderText( const std::optional<TimeFromSimulationStartInfo>& timeFromSimulationStart )
{
    if ( timeFromSimulationStart ) return QString( "Time [%1]" ).arg( timeFromSimulationStart->displayUnitText );

    return "Date and time";
}

//--------------------------------------------------------------------------------------------------
/// Text for a single time step, either date and time or time from simulation start
//--------------------------------------------------------------------------------------------------
QString timeStepText( const QDateTime& timeStep, const std::optional<TimeFromSimulationStartInfo>& timeFromSimulationStart )
{
    if ( timeFromSimulationStart )
    {
        double timeSinceSimulationStart = RimSummaryTimeAxisProperties::timeFromSimulationStart( timeFromSimulationStart->simulationStartTime,
                                                                                                 timeStep,
                                                                                                 timeFromSimulationStart->displayUnit );

        return QString::number( timeSinceSimulationStart, 'g', RimSummaryPlot::precision() );
    }

    return timeStep.toString( "yyyy-MM-dd hh:mm:ss " );
}

//--------------------------------------------------------------------------------------------------
/// Label for a resampled time step, reporting the period ending at the time step
//--------------------------------------------------------------------------------------------------
QString resampledTimeStepText( const QDateTime& timeStepUtc, RiaDefines::DateTimePeriod period )
{
    // Subtract one day to make sure the period is reported using the previous period as label
    const QDateTime oneDayEarlier = timeStepUtc.addDays( -1 );

    const QChar zeroChar( 48 );

    switch ( period )
    {
        default:
            // Fall through to NONE
        case RiaDefines::DateTimePeriod::NONE:
            return timeStepUtc.toString( "yyyy-MM-dd hh:mm:ss " );
        case RiaDefines::DateTimePeriod::DAY:
            return oneDayEarlier.toString( "yyyy-MM-dd " );
        case RiaDefines::DateTimePeriod::WEEK:
            return oneDayEarlier.toString( "yyyy" ) + QString( "-W%1" ).arg( oneDayEarlier.date().weekNumber(), 2, 10, zeroChar );
        case RiaDefines::DateTimePeriod::MONTH:
            return oneDayEarlier.toString( "yyyy-MM" );
        case RiaDefines::DateTimePeriod::QUARTER:
            return oneDayEarlier.toString( "yyyy" ) + QString( "-Q%1" ).arg( oneDayEarlier.date().month() / 3 );
        case RiaDefines::DateTimePeriod::HALFYEAR:
            return oneDayEarlier.toString( "yyyy" ) + QString( "-H%1" ).arg( oneDayEarlier.date().month() / 6 );
        case RiaDefines::DateTimePeriod::YEAR:
        case RiaDefines::DateTimePeriod::DECADE:
            return oneDayEarlier.toString( "yyyy" );
    }
}

//--------------------------------------------------------------------------------------------------
/// All time steps for the resampling period covering the time range of all cases
//--------------------------------------------------------------------------------------------------
std::vector<time_t> resampledTimeSteps( const RimSummaryCurvesData& data )
{
    time_t minTimeStep = std::numeric_limits<time_t>::max();
    time_t maxTimeStep = 0;

    for ( const auto& timeSteps : data.timeSteps )
    {
        if ( !timeSteps.empty() )
        {
            if ( timeSteps.front() < minTimeStep ) minTimeStep = timeSteps.front();
            if ( timeSteps.back() > maxTimeStep ) maxTimeStep = timeSteps.back();
        }
    }

    return RiaTimeHistoryCurveResampler::timeStepsFromTimeRange( data.resamplePeriod, minTimeStep, maxTimeStep );
}

//--------------------------------------------------------------------------------------------------
/// Asks the user to confirm before producing a very large number of rows
//--------------------------------------------------------------------------------------------------
bool confirmRowCount( size_t rowCount )
{
    const size_t threshold = 50000;
    if ( rowCount <= threshold || !RiaGuiApplication::isRunning() ) return true;

    QString questionStr = QString( "This operation will produce %1 text lines. Do you want to continue?" ).arg( rowCount );

    auto reply = QMessageBox::question( RiaGuiApplication::widgetToUseAsParent(),
                                        "Summary Text Export",
                                        questionStr,
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::No );
    return reply == QMessageBox::Yes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableDoubleFormatting summaryValueFormat()
{
    return RifTextDataTableDoubleFormatting( RIF_CONSISE, RimSummaryPlot::precision() );
}

//--------------------------------------------------------------------------------------------------
/// Time column for report tables, either date and time or time from simulation start
//--------------------------------------------------------------------------------------------------
RiaReportColumn reportTimeColumn( const std::optional<TimeFromSimulationStartInfo>& timeFromSimulationStart, bool usePeriodLabels )
{
    if ( timeFromSimulationStart )
    {
        return RiaReportColumn{ .title = "Time", .unit = timeFromSimulationStart->displayUnitText, .doubleFormat = summaryValueFormat() };
    }

    return RiaReportColumn{ .title = usePeriodLabels ? QString( "Period" ) : QString( "Date and time" ), .alignment = LEFT };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaReportCellValue reportTimeCell( const QDateTime& timeStep, const std::optional<TimeFromSimulationStartInfo>& timeFromSimulationStart )
{
    if ( timeFromSimulationStart )
    {
        return RimSummaryTimeAxisProperties::timeFromSimulationStart( timeFromSimulationStart->simulationStartTime,
                                                                      timeStep,
                                                                      timeFromSimulationStart->displayUnit );
    }

    return timeStep;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::populateTimeHistoryCurvesData( std::vector<RimGridTimeHistoryCurve*> curves, RimSummaryCurvesData* curvesData )
{
    CAF_ASSERT( curvesData );

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
    CAF_ASSERT( curvesData );

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
        CAF_ASSERT( timeSteps[existingCaseIndex].size() == curveData.values.size() );

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
QString RimSummaryCurvesData::createTextForExport( const std::vector<RimSummaryCurve*>&              curves,
                                                   const std::vector<RimAsciiDataCurve*>&            asciiCurves,
                                                   const std::vector<RimGridTimeHistoryCurve*>&      gridCurves,
                                                   RiaDefines::DateTimePeriod                        resamplingPeriod,
                                                   bool                                              showTimeAsLongString,
                                                   const std::optional<TimeFromSimulationStartInfo>& timeFromSimulationStart )
{
    if ( curves.empty() && asciiCurves.empty() && gridCurves.empty() ) return {};

    QString out;

    RimSummaryCurvesData summaryCurvesData;
    RimSummaryCurvesData summaryCurvesObsData;
    RimSummaryCurvesData gridCurvesData;
    RimSummaryCurvesData::populateSummaryCurvesData( curves, SummaryCurveType::CURVE_TYPE_SUMMARY, &summaryCurvesData );
    RimSummaryCurvesData::populateSummaryCurvesData( curves, SummaryCurveType::CURVE_TYPE_OBSERVED, &summaryCurvesObsData );
    RimSummaryCurvesData::populateTimeHistoryCurvesData( gridCurves, &gridCurvesData );

    RimSummaryCurvesData::appendToExportData( out, { summaryCurvesObsData }, showTimeAsLongString, timeFromSimulationStart );

    std::vector<RimSummaryCurvesData> exportData( 2 );

    RimSummaryCurvesData::prepareCaseCurvesForExport( resamplingPeriod, summaryCurvesData, &exportData[0] );
    RimSummaryCurvesData::prepareCaseCurvesForExport( resamplingPeriod, gridCurvesData, &exportData[1] );

    RimSummaryCurvesData::appendToExportData( out, exportData, showTimeAsLongString, timeFromSimulationStart );

    {
        // Handle observed data pasted into plot from clipboard

        RimSummaryCurvesData asciiCurvesData;
        RimSummaryCurvesData::populateAsciiDataCurvesData( asciiCurves, &asciiCurvesData );

        RimSummaryCurvesData::appendToExportData( out, { asciiCurvesData }, showTimeAsLongString, timeFromSimulationStart );
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
std::vector<RiaReportTable> RimSummaryCurvesData::createReportTables( const std::vector<RimSummaryCurve*>&         curves,
                                                                      const std::vector<RimAsciiDataCurve*>&       asciiCurves,
                                                                      const std::vector<RimGridTimeHistoryCurve*>& gridCurves,
                                                                      RiaDefines::DateTimePeriod                   resamplingPeriod,
                                                                      bool                                         showTimeAsLongString,
                                                                      const std::optional<TimeFromSimulationStartInfo>& timeFromSimulationStart )
{
    if ( curves.empty() && asciiCurves.empty() && gridCurves.empty() ) return {};

    std::vector<RiaReportTable> tables;

    RimSummaryCurvesData summaryCurvesData;
    RimSummaryCurvesData summaryCurvesObsData;
    RimSummaryCurvesData gridCurvesData;
    RimSummaryCurvesData::populateSummaryCurvesData( curves, SummaryCurveType::CURVE_TYPE_SUMMARY, &summaryCurvesData );
    RimSummaryCurvesData::populateSummaryCurvesData( curves, SummaryCurveType::CURVE_TYPE_OBSERVED, &summaryCurvesObsData );
    RimSummaryCurvesData::populateTimeHistoryCurvesData( gridCurves, &gridCurvesData );

    RimSummaryCurvesData::appendReportTables( tables, { summaryCurvesObsData }, showTimeAsLongString, timeFromSimulationStart );

    std::vector<RimSummaryCurvesData> exportData( 2 );
    RimSummaryCurvesData::prepareCaseCurvesForExport( resamplingPeriod, summaryCurvesData, &exportData[0] );
    RimSummaryCurvesData::prepareCaseCurvesForExport( resamplingPeriod, gridCurvesData, &exportData[1] );

    RimSummaryCurvesData::appendReportTables( tables, exportData, showTimeAsLongString, timeFromSimulationStart );

    // Observed data pasted into plot from clipboard
    RimSummaryCurvesData asciiCurvesData;
    RimSummaryCurvesData::populateAsciiDataCurvesData( asciiCurves, &asciiCurvesData );
    RimSummaryCurvesData::appendReportTables( tables, { asciiCurvesData }, showTimeAsLongString, timeFromSimulationStart );

    return tables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaReportTable> RimSummaryCurvesData::createReportTablesForCrossPlotCurves( const std::vector<RimSummaryCurve*>& curves )
{
    std::vector<RiaReportTable> tables;

    for ( const auto& curve : curves )
    {
        const auto curveAddress = curve->curveAddress();
        const auto xValues      = curve->valuesX();
        const auto yValues      = curve->valuesY();
        if ( xValues.size() != yValues.size() ) continue;

        RiaReportTable table;
        table.title   = curve->curveExportDescription( {} );
        table.columns = { RiaReportColumn{ .title        = QString::fromStdString( curveAddress.summaryAddressX().vectorName() ),
                                           .doubleFormat = summaryValueFormat() },
                          RiaReportColumn{ .title        = QString::fromStdString( curveAddress.summaryAddressY().vectorName() ),
                                           .doubleFormat = summaryValueFormat() } };

        table.rows.reserve( xValues.size() );
        for ( size_t i = 0; i < xValues.size(); i++ )
        {
            table.rows.push_back( { xValues[i], yValues[i] } );
        }

        tables.push_back( std::move( table ) );
    }

    return tables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::populateSummaryCurvesData( std::vector<RimSummaryCurve*> curves,
                                                      SummaryCurveType              curveType,
                                                      RimSummaryCurvesData*         curvesData )
{
    CAF_ASSERT( curvesData );

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

        // Error data - skip for ensemble curves
        // A check if data is present is done in RifSummaryReaderInterface::hasAddress. For ensemble file readers, m_allResultAddresses will
        // be empty for all realizations except one. To avoid display of error data for ensemble curves, we do an explicit check here.
        // https://github.com/OPM/ResInsight/issues/13055
        bool hasErrorData = false;
        if ( !curve->isEnsembleCurve() )
        {
            auto errorValues = curve->errorValuesY();
            hasErrorData     = !errorValues.empty();

            if ( hasErrorData )
            {
                errorCurveData.name      = curve->curveExportDescription( curve->errorSummaryAddressY() );
                errorCurveData.curveType = curve->curveType();
                errorCurveData.values    = errorValues;
            }
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
                if ( cd.values.empty() ) continue;

                curvesData->addCurveData( curveCaseName, ensembleName, curve->timeStepsY(), cd );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::prepareCaseCurvesForExport( RiaDefines::DateTimePeriod  period,
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
void RimSummaryCurvesData::appendToExportDataForCase( QString&                                          out,
                                                      const std::vector<time_t>&                        timeSteps,
                                                      const std::vector<CurveData>&                     curveData,
                                                      const std::optional<TimeFromSimulationStartInfo>& timeFromSimulationStart )
{
    for ( size_t j = 0; j < timeSteps.size(); j++ ) // time steps & data points
    {
        if ( j == 0 )
        {
            out += timeColumnHeaderText( timeFromSimulationStart );
            for ( const auto& k : curveData ) // curves
            {
                out += "\t" + ( k.name );
            }
        }
        out += "\n";
        out += timeStepText( RiaQDateTimeTools::fromTime_t( timeSteps[j] ), timeFromSimulationStart );

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
void RimSummaryCurvesData::appendToExportData( QString&                                          out,
                                               const std::vector<RimSummaryCurvesData>&          curvesData,
                                               bool                                              showTimeAsLongString,
                                               const std::optional<TimeFromSimulationStartInfo>& timeFromSimulationStart )
{
    RimSummaryCurvesData data = RimSummaryCurvesData::concatCurvesData( curvesData );

    if ( data.resamplePeriod != RiaDefines::DateTimePeriod::NONE )
    {
        auto allTimeSteps = resampledTimeSteps( data );
        if ( !confirmRowCount( allTimeSteps.size() ) ) return;

        out += "\n\n";
        out += timeColumnHeaderText( timeFromSimulationStart );
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

            if ( timeFromSimulationStart )
            {
                timeText = timeStepText( timseStepUtc, timeFromSimulationStart );
            }
            else if ( showTimeAsLongString )
            {
                timeText = timseStepUtc.toString( "yyyy-MM-dd hh:mm:ss " );
            }
            else
            {
                timeText = resampledTimeStepText( timseStepUtc, data.resamplePeriod );
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

            appendToExportDataForCase( out, data.timeSteps[i], data.allCurveData[i], timeFromSimulationStart );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Resampled data is reported in one table with all cases, otherwise there is one table per case
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesData::appendReportTables( std::vector<RiaReportTable>&                      tables,
                                               const std::vector<RimSummaryCurvesData>&          curvesData,
                                               bool                                              showTimeAsLongString,
                                               const std::optional<TimeFromSimulationStartInfo>& timeFromSimulationStart )
{
    RimSummaryCurvesData data = RimSummaryCurvesData::concatCurvesData( curvesData );
    if ( data.caseIds.empty() ) return;

    const auto valueFormat = summaryValueFormat();

    if ( data.resamplePeriod != RiaDefines::DateTimePeriod::NONE )
    {
        const auto allTimeSteps = resampledTimeSteps( data );
        if ( !confirmRowCount( allTimeSteps.size() ) ) return;

        const bool usePeriodLabels = !timeFromSimulationStart && !showTimeAsLongString;

        RiaReportTable table;
        table.columns.push_back( reportTimeColumn( timeFromSimulationStart, usePeriodLabels ) );
        for ( const auto& caseCurveData : data.allCurveData )
        {
            for ( const auto& curveData : caseCurveData )
            {
                table.columns.push_back( RiaReportColumn{ .title = curveData.name, .doubleFormat = valueFormat } );
            }
        }

        std::vector<size_t> currIndexes( data.caseIds.size(), 0 );

        table.rows.reserve( allTimeSteps.size() );
        for ( auto timeStep : allTimeSteps )
        {
            const QDateTime timeStepUtc = QDateTime::fromSecsSinceEpoch( timeStep ).toUTC();

            std::vector<RiaReportCellValue> row;
            row.reserve( table.columns.size() );
            if ( usePeriodLabels )
            {
                row.emplace_back( resampledTimeStepText( timeStepUtc, data.resamplePeriod ).trimmed() );
            }
            else
            {
                row.push_back( reportTimeCell( timeStepUtc, timeFromSimulationStart ) );
            }

            for ( size_t i = 0; i < data.caseIds.size(); i++ )
            {
                size_t&    currIndex      = currIndexes[i];
                const bool timeStepExists = currIndex < data.timeSteps[i].size() && timeStep == data.timeSteps[i][currIndex];

                for ( const auto& curveData : data.allCurveData[i] )
                {
                    if ( timeStepExists && currIndex < curveData.values.size() )
                    {
                        row.emplace_back( curveData.values[currIndex] );
                    }
                    else
                    {
                        row.emplace_back( std::monostate() );
                    }
                }

                if ( timeStepExists ) currIndex++;
            }

            table.rows.push_back( std::move( row ) );
        }

        tables.push_back( std::move( table ) );
        return;
    }

    for ( size_t i = 0; i < data.caseIds.size(); i++ )
    {
        const auto& timeSteps     = data.timeSteps[i];
        const auto& caseCurveData = data.allCurveData[i];

        RiaReportTable table;
        if ( !data.caseIds[i].isEmpty() ) table.title = "Case: " + data.caseIds[i];

        table.columns.push_back( reportTimeColumn( timeFromSimulationStart, false ) );
        for ( const auto& curveData : caseCurveData )
        {
            table.columns.push_back( RiaReportColumn{ .title = curveData.name, .doubleFormat = valueFormat } );
        }

        table.rows.reserve( timeSteps.size() );
        for ( size_t j = 0; j < timeSteps.size(); j++ )
        {
            std::vector<RiaReportCellValue> row;
            row.reserve( table.columns.size() );
            row.push_back( reportTimeCell( RiaQDateTimeTools::fromTime_t( timeSteps[j] ), timeFromSimulationStart ) );

            for ( const auto& curveData : caseCurveData )
            {
                if ( j < curveData.values.size() )
                {
                    row.emplace_back( curveData.values[j] );
                }
                else
                {
                    row.emplace_back( std::monostate() );
                }
            }

            table.rows.push_back( std::move( row ) );
        }

        tables.push_back( std::move( table ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurvesData RimSummaryCurvesData::concatCurvesData( const std::vector<RimSummaryCurvesData>& curvesData )
{
    CAF_ASSERT( !curvesData.empty() );

    RiaDefines::DateTimePeriod period = curvesData.front().resamplePeriod;
    RimSummaryCurvesData       resultCurvesData;

    resultCurvesData.resamplePeriod = period;

    for ( auto curvesDataItem : curvesData )
    {
        if ( curvesDataItem.caseIds.empty() ) continue;

        CAF_ASSERT( curvesDataItem.resamplePeriod == period );

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
