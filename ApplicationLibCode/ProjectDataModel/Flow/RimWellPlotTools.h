/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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
#include "RiaRftPltCurveDefinition.h"

#include "RifDataSourceForRftPltQMetaType.h"
#include "RifEclipseRftAddress.h"

#include "RimWellLogLasFile.h"

#include <QMetaType>

#include <map>
#include <set>

class RimEclipseCase;
class RimEclipseResultCase;
class RimObservedFmuRftData;
class RimSummaryCaseCollection;
class RimWellLogCurve;
class RimWellLogChannel;
class RimWellLogPlot;
class RimWellPath;
class RimPressureDepthData;
class RiuWellRftPlot;
class RigEclipseCaseData;
class RigEclipseResultAddress;
class RifReaderRftInterface;

//==================================================================================================
///
//==================================================================================================
enum FlowType
{
    FLOW_TYPE_PHASE_SPLIT,
    FLOW_TYPE_TOTAL
};
enum FlowPhase
{
    FLOW_PHASE_NONE,
    FLOW_PHASE_OIL,
    FLOW_PHASE_GAS,
    FLOW_PHASE_WATER,
    FLOW_PHASE_TOTAL
};

//==================================================================================================
///
//==================================================================================================
class RimWellPlotTools
{
public:
    // PLT Only
    static bool      isOilFlowChannel( const QString& channelName );
    static bool      isGasFlowChannel( const QString& channelName );
    static bool      isWaterFlowChannel( const QString& channelName );
    static bool      isTotalFlowChannel( const QString& channelName );
    static FlowPhase flowPhaseFromChannelName( const QString& channelName );

    static std::vector<RimWellLogFile*> wellLogFilesContainingFlow( const QString& wellName );
    static RimWellPath*                 wellPathByWellPathNameOrSimWellName( const QString& wellPathNameOrSimwellName );
    static std::vector<RimWellPath*>    wellPathsContainingFlow();

    // RFT Only
private:
    static std::pair<RigEclipseResultAddress, QString> pressureResultDataInfo( const RigEclipseCaseData* eclipseCaseData );

public:
    static void                         addTimeStepsToMap( std::map<QDateTime, std::set<RifDataSourceForRftPlt>>&       destMap,
                                                           const std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepsToAdd );
    static std::vector<RimWellLogFile*> wellLogFilesContainingPressure( const QString& wellPathNameOrSimWellName );
    static RimWellLogChannel*           getPressureChannelFromWellFile( const RimWellLogFile* wellLogFile );
    static RimWellPath*                 wellPathFromWellLogFile( const RimWellLogFile* wellLogFile );
    static std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timeStepsMapFromGridCase( RimEclipseCase* gridCase );
    static RiaRftPltCurveDefinition                              curveDefFromCurve( const RimWellLogCurve* curve );

    // others
    static bool hasFlowData( const RimWellLog* wellLog );
    static bool hasAssociatedWellPath( const QString& wellName );

    // Both
    static std::vector<RimEclipseResultCase*>     gridCasesForWell( const QString& simWellName );
    static std::vector<RimEclipseResultCase*>     rftCasesForWell( const QString& simWellName );
    static std::vector<RimSummaryCaseCollection*> rftEnsemblesForWell( const QString& simWellName );
    static std::vector<RimSummaryCaseCollection*> rftEnsembles();
    static std::vector<RimObservedFmuRftData*>    observedFmuRftDataForWell( const QString& simWellName );
    static std::vector<RimObservedFmuRftData*>    observedFmuRftData();
    static QString                                simWellName( const QString& wellPathNameOrSimWellName );

    static std::map<QDateTime, std::set<RifDataSourceForRftPlt>>
        calculateRelevantTimeStepsFromCases( const QString&                                               wellPathNameOrSimWellName,
                                             const std::vector<RifDataSourceForRftPlt>&                   selSources,
                                             const std::set<RifEclipseRftAddress::RftWellLogChannelType>& interestingRFTResults );

    static void calculateValueOptionsForTimeSteps( const QString&                                               wellPathNameOrSimWellName,
                                                   const std::vector<RifDataSourceForRftPlt>&                   selSources,
                                                   const std::set<RifEclipseRftAddress::RftWellLogChannelType>& interestingRFTResults,
                                                   QList<caf::PdmOptionItemInfo>&                               options );

    static std::set<RiaRftPltCurveDefinition>
        curveDefsFromTimesteps( const QString&                                               wellPathNameOrSimWellName,
                                const std::vector<QDateTime>&                                selectedTimeStepVector,
                                bool                                                         firstReportTimeStepIsValid,
                                const std::vector<RifDataSourceForRftPlt>&                   selectedSourcesExpanded,
                                const std::set<RifEclipseRftAddress::RftWellLogChannelType>& interestingRFTResults );

    static QString flowPlotAxisTitle( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem );
    static QString flowUnitText( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem );
    static QString flowVolumePlotAxisTitle( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem );
    static QString flowVolumeUnitText( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem );

    static QString
        curveUnitText( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem, FlowPhase flowPhase );

    static bool hasFlowData( const RimWellPath* wellPath );

    static std::vector<RimPressureDepthData*> pressureDepthData();
    static std::vector<RimPressureDepthData*> pressureDepthDataForWell( const QString& simWellName );

private:
    friend class StaticFieldsInitializer;
    static const std::set<QString> PRESSURE_DATA_NAMES;

    static const std::set<QString> OIL_CHANNEL_NAMES;
    static const std::set<QString> GAS_CHANNEL_NAMES;
    static const std::set<QString> WATER_CHANNEL_NAMES;
    static const std::set<QString> TOTAL_CHANNEL_NAMES;

    static std::set<QString> FLOW_DATA_NAMES;

    static bool                hasPressureData( const RimWellLogFile* wellLogFile );
    static bool                isPressureChannel( RimWellLogChannel* channel );
    static bool                hasPressureData( RimEclipseResultCase* gridCase );
    static bool                hasPressureData( RimWellPath* wellPath );
    static bool                hasFlowData( RimEclipseResultCase* gridCase );
    static bool                isFlowChannel( RimWellLogChannel* channel );
    static bool                tryMatchChannelName( const std::set<QString>& channelNames, const QString& channelNameToMatch );
    static std::set<QDateTime> findMatchingOrAdjacentTimeSteps( const std::set<QDateTime>& baseTimeLine,
                                                                const std::set<QDateTime>& availableTimeSteps );
    static std::set<QDateTime> availableSimWellTimesteps( RimEclipseCase* eclCase, const QString& simWellName, bool addFirstReportTimeStep );

    static RifReaderRftInterface* rftReaderInterface( RimEclipseCase* eclipseCase );
};
