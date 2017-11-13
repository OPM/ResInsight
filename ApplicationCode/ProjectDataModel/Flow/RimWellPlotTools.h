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


#include "RifDataSourceForRftPltQMetaType.h"
#include "RiaRftPltCurveDefinition.h"

#include <QMetaType>

#include <map>
#include <set>

class RimEclipseCase;
class RimEclipseResultCase;
class RimWellLogCurve;
class RimWellLogFileChannel;
class RimWellLogPlot;
class RimWellPath;
class RiuWellRftPlot;
class RigEclipseCaseData;

//==================================================================================================
///  
//==================================================================================================
enum FlowType { FLOW_TYPE_TOTAL, FLOW_TYPE_PHASE_SPLIT };
enum FlowPhase { FLOW_PHASE_NONE, FLOW_PHASE_OIL, FLOW_PHASE_GAS, FLOW_PHASE_WATER, FLOW_PHASE_TOTAL };

//==================================================================================================
///  
//==================================================================================================
class RimWellPlotTools
{
    static const std::set<QString> PRESSURE_DATA_NAMES;

    static const std::set<QString> OIL_CHANNEL_NAMES;
    static const std::set<QString> GAS_CHANNEL_NAMES;
    static const std::set<QString> WATER_CHANNEL_NAMES;
    static const std::set<QString> TOTAL_CHANNEL_NAMES;

    static std::set<QString> FLOW_DATA_NAMES;

public:
    static bool                                     hasPressureData(const RimWellLogFile* wellLogFile);
    static bool                                     isPressureChannel(RimWellLogFileChannel* channel);
    static bool                                     hasPressureData(RimEclipseResultCase* gridCase);
    static bool                                     hasPressureData(RimWellPath* wellPath);
    static std::pair<size_t, QString>               pressureResultDataInfo(const RigEclipseCaseData* eclipseCaseData);

    static bool                                     hasFlowData(const RimWellLogFile* wellLogFile);
    static bool                                     isFlowChannel(RimWellLogFileChannel* channel);
    static bool                                     isOilFlowChannel(const QString& channelName);
    static bool                                     isGasFlowChannel(const QString& channelName);
    static bool                                     isWaterFlowChannel(const QString& channelName);
    static bool                                     hasFlowData(RimEclipseResultCase* gridCase);
    static bool                                     hasFlowData(RimWellPath* wellPath);
    static FlowPhase                                flowPhaseFromChannelName(const QString& channelName);

    static void                                     addTimeStepToMap(std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& destMap,
                                                                     const std::pair<QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepToAdd);
    static void                                     addTimeStepsToMap(std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& destMap,
                                                                      const std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepsToAdd);

    static std::vector<RimWellLogFile*>             wellLogFilesContainingPressure(const QString& simWellName);
    static RimWellLogFileChannel*                   getPressureChannelFromWellFile(const RimWellLogFile* wellLogFile);

    static std::vector<RimWellLogFile*>             wellLogFilesContainingFlow(const QString& wellName);
    static std::vector<RimWellLogFileChannel*>      getFlowChannelsFromWellFile(const RimWellLogFile* wellLogFile);

    static RimWellPath*                             wellPathFromWellLogFile(const RimWellLogFile* wellLogFile);

    static std::vector<RimEclipseResultCase*>               gridCasesForWell(const QString& simWellName);
    static std::vector<RimEclipseResultCase*>               rftCasesForWell(const QString& simWellName);

    static std::set<QDateTime>                              timeStepsFromRftCase(RimEclipseResultCase* rftCase, const QString& simWellName);
    static std::set<QDateTime>                              timeStepsFromGridCase(RimEclipseCase* gridCase);
    static QDateTime                                        timeStepFromWellLogFile(RimWellLogFile* wellLogFile);

    static std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timeStepsMapFromRftCase(RimEclipseResultCase* rftCase, const QString& simWellName);
    static std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timeStepsMapFromGridCase(RimEclipseCase* gridCase);
    static std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timeStepsMapFromWellLogFile(RimWellLogFile* wellLogFile);
    static std::map<QDateTime, std::set<RifDataSourceForRftPlt>> adjacentTimeSteps(const std::vector<std::pair<QDateTime, std::set<RifDataSourceForRftPlt>>>& allTimeSteps,
                                                                              const std::pair<QDateTime, std::set<RifDataSourceForRftPlt>>& searchTimeStepPair);
    static bool                                             mapContainsTimeStep(const std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& map, const QDateTime& timeStep);

    static RiaRftPltCurveDefinition                 curveDefFromCurve(const RimWellLogCurve* curve);

    static RimWellPath*                             wellPathByWellPathNameOrSimWellName(const QString& wellPathNameOrSimwellName);
    static QString                                  simWellName(const QString& wellPathNameOrSimWellName);
    static bool                                     tryMatchChannelName(const std::set<QString>& channelNames, const QString& channelNameToMatch);

    template<typename T>
    static void appendSet(std::set<T>& destSet, const std::set<T>& setToAppend);

    friend class StaticFieldsInitializer;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
void RimWellPlotTools::appendSet(std::set<T>& destSet, const std::set<T>& setToAppend)
{
    destSet.insert(setToAppend.begin(), setToAppend.end());
}

