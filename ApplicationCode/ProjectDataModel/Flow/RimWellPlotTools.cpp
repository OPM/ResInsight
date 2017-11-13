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

#include "RimWellPlotTools.h"

#include "RiaApplication.h"
#include "RiaWellNameComparer.h"

#include "RifReaderEclipseRft.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigSimWellData.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogRftCurve.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include <regex>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::set<QString> RimWellPlotTools::PRESSURE_DATA_NAMES = { "PRESSURE", "PRES_FORM" };

const std::set<QString> RimWellPlotTools::OIL_CHANNEL_NAMES = { "QOZT", "QOIL", "^.*\\D_QOIL" };
const std::set<QString> RimWellPlotTools::GAS_CHANNEL_NAMES = { "QGZT", "QGAS", "^.*\\D_QGAS" };
const std::set<QString> RimWellPlotTools::WATER_CHANNEL_NAMES = { "QWZT", "QWAT", "^.*\\D_QWAT" };
const std::set<QString> RimWellPlotTools::TOTAL_CHANNEL_NAMES = { "QTZT", "QTOT", "^.*\\D_QTOT" };

std::set<QString> RimWellPlotTools::FLOW_DATA_NAMES = {};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class StaticFieldsInitializer
{
public:
    StaticFieldsInitializer()
    {
        // Init static list
        RimWellPlotTools::FLOW_DATA_NAMES.insert(RimWellPlotTools::OIL_CHANNEL_NAMES.begin(), RimWellPlotTools::OIL_CHANNEL_NAMES.end());
        RimWellPlotTools::FLOW_DATA_NAMES.insert(RimWellPlotTools::GAS_CHANNEL_NAMES.begin(), RimWellPlotTools::GAS_CHANNEL_NAMES.end());
        RimWellPlotTools::FLOW_DATA_NAMES.insert(RimWellPlotTools::WATER_CHANNEL_NAMES.begin(), RimWellPlotTools::WATER_CHANNEL_NAMES.end());
        RimWellPlotTools::FLOW_DATA_NAMES.insert(RimWellPlotTools::TOTAL_CHANNEL_NAMES.begin(), RimWellPlotTools::TOTAL_CHANNEL_NAMES.end());
    }
} staticFieldsInitializer;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::hasPressureData(const RimWellLogFile* wellLogFile)
{
    for (RimWellLogFileChannel* const wellLogChannel : wellLogFile->wellLogChannels())
    {
        if (isPressureChannel(wellLogChannel)) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::hasPressureData(RimWellPath* wellPath)
{
    for (RimWellLogFile* const wellLogFile : wellPath->wellLogFiles())
    {
        if (hasPressureData(wellLogFile))
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<size_t, QString> RimWellPlotTools::pressureResultDataInfo(const RigEclipseCaseData* eclipseCaseData)
{
    if (eclipseCaseData != nullptr)
    {
        for (const auto& pressureDataName : PRESSURE_DATA_NAMES)
        {
            size_t index = eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->
                findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, pressureDataName);
            if (index != cvf::UNDEFINED_SIZE_T)
            {
                return std::make_pair(index, pressureDataName);
            }
        }
    }
    return std::make_pair(cvf::UNDEFINED_SIZE_T, "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::isPressureChannel(RimWellLogFileChannel* channel)
{
    for (const auto& pressureDataName : PRESSURE_DATA_NAMES)
    {
        if (QString::compare(channel->name(), pressureDataName, Qt::CaseInsensitive) == 0) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::hasPressureData(RimEclipseResultCase* gridCase)
{
    return pressureResultDataInfo(gridCase->eclipseCaseData()).first != cvf::UNDEFINED_SIZE_T;
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::hasFlowData(const RimWellLogFile* wellLogFile)
{
    for (RimWellLogFileChannel* const wellLogChannel : wellLogFile->wellLogChannels())
    {
        if (isFlowChannel(wellLogChannel)) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::hasFlowData(RimWellPath* wellPath)
{
    for (RimWellLogFile* const wellLogFile : wellPath->wellLogFiles())
    {
        if (hasFlowData(wellLogFile))
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::isFlowChannel(RimWellLogFileChannel* channel)
{
    return tryMatchChannelName(FLOW_DATA_NAMES, channel->name());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::isOilFlowChannel(const QString& channelName)
{
    return tryMatchChannelName(OIL_CHANNEL_NAMES, channelName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::isGasFlowChannel(const QString& channelName)
{
    return tryMatchChannelName(GAS_CHANNEL_NAMES, channelName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::isWaterFlowChannel(const QString& channelName)
{
    return tryMatchChannelName(WATER_CHANNEL_NAMES, channelName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::hasFlowData(RimEclipseResultCase* gridCase)
{
    const RigEclipseCaseData* const eclipseCaseData = gridCase->eclipseCaseData();

    for (const QString& channelName : FLOW_DATA_NAMES)
    {
        size_t resultIndex = eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->
            findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, channelName);

        if (resultIndex != cvf::UNDEFINED_SIZE_T) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FlowPhase RimWellPlotTools::flowPhaseFromChannelName(const QString& channelName)
{
    if (tryMatchChannelName(OIL_CHANNEL_NAMES, channelName)) return FLOW_PHASE_OIL;
    if (tryMatchChannelName(GAS_CHANNEL_NAMES, channelName)) return FLOW_PHASE_GAS;
    if (tryMatchChannelName(WATER_CHANNEL_NAMES, channelName)) return FLOW_PHASE_WATER;
    if (tryMatchChannelName(TOTAL_CHANNEL_NAMES, channelName)) return FLOW_PHASE_TOTAL;
    return FLOW_PHASE_NONE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPlotTools::addTimeStepToMap(std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& destMap,
                                      const std::pair<QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepToAdd)
{
    auto timeStepMapToAdd = std::map<QDateTime, std::set<RifDataSourceForRftPlt>>{ timeStepToAdd };
    addTimeStepsToMap(destMap, timeStepMapToAdd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPlotTools::addTimeStepsToMap(std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& destMap,
                                       const std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepsToAdd)
{
    for (const auto& timeStepPair : timeStepsToAdd)
    {
        if (timeStepPair.first.isValid())
        {
            if (destMap.count(timeStepPair.first) == 0)
            {
                destMap.insert(std::make_pair(timeStepPair.first, std::set<RifDataSourceForRftPlt>()));
            }
            auto addresses = timeStepPair.second;
            destMap[timeStepPair.first].insert(addresses.begin(), addresses.end());
        }
    }
}
#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> RimWellPlotTools::wellLogFilesContainingPressure(const QString& wellPathName)
{
    std::vector<RimWellLogFile*> wellLogFiles;
    const RimProject* const project = RiaApplication::instance()->project();

    for (const auto& oilField : project->oilFields)
    {
        auto wellPathsVector = std::vector<RimWellPath*>(oilField->wellPathCollection()->wellPaths.begin(), oilField->wellPathCollection()->wellPaths.end());

        for (const auto& wellPath : wellPathsVector)
        {
            bool hasPressure = false;
            const std::vector<RimWellLogFile*> files = wellPath->wellLogFiles();

            for (RimWellLogFile* const file : files)
            {
                size_t timeStepCount = timeStepsMapFromWellLogFile(file).size();    // todo: only one timestep

                if (timeStepCount == 0) continue;
                if (RiaWellNameComparer::tryFindMatchingWellPath(wellPathName).isEmpty()) continue;

                if (hasPressureData(file))
                {
                    wellLogFiles.push_back(file);
                }
            }
        }
    }
    return wellLogFiles;
}
#endif

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> RimWellPlotTools::wellLogFilesContainingPressure(const QString& simWellName)
{
    std::vector<RimWellLogFile*> wellLogFiles;
    const RimProject* const project = RiaApplication::instance()->project();
    std::vector<RimWellPath*> wellPaths = project->allWellPaths();

    for (auto wellPath : wellPaths)
    {
        if (simWellName == wellPath->associatedSimulationWell())
        {
            const std::vector<RimWellLogFile*> files = wellPath->wellLogFiles();

            for (RimWellLogFile* file : files)
            {
                if (hasPressureData(file))
                {
                    wellLogFiles.push_back(file);
                } 
            }
        }
    }

    return wellLogFiles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFileChannel* RimWellPlotTools::getPressureChannelFromWellFile(const RimWellLogFile* wellLogFile)
{
    if (wellLogFile != nullptr)
    {
        for (RimWellLogFileChannel* const channel : wellLogFile->wellLogChannels())
        {
            if (isPressureChannel(channel))
            {
                return channel;
            }
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> RimWellPlotTools::wellLogFilesContainingFlow(const QString& wellPathName)
{
    std::vector<RimWellLogFile*> wellLogFiles;
    const RimProject* const project = RiaApplication::instance()->project();
    std::vector<RimWellPath*> wellPaths = project->allWellPaths();

    for ( auto wellPath : wellPaths )
    {
        if ( wellPath->name() == wellPathName )
        {
            std::vector<RimWellLogFile*> files = wellPath->wellLogFiles();

            for ( RimWellLogFile* file : files )
            {
                if ( hasFlowData(file) )
                {
                    wellLogFiles.push_back(file);
                }
            }
        }
    }
    return wellLogFiles;
}

#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> RimWellPlotTools::wellLogFilesContainingFlow(const QString& wellPathName)
{
    std::vector<RimWellLogFile*> wellLogFiles;
    const RimProject* const project = RiaApplication::instance()->project();

    for (const auto& wellPath : project->allWellPaths())
    {
        bool hasPressure = false;
        const std::vector<RimWellLogFile*> files = wellPath->wellLogFiles();

        for (RimWellLogFile* const file : files)
        {
            size_t timeStepCount = timeStepsMapFromWellLogFile(file).size();    // todo: only one timestep

            if (timeStepCount == 0) continue;
            if (RiaWellNameComparer::tryFindMatchingWellPath(wellPathName).isEmpty()) continue;

            if (hasFlowData(file))
            {
                wellLogFiles.push_back(file);
            }
        }
    }
    return wellLogFiles;
}
#endif
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFileChannel*> RimWellPlotTools::getFlowChannelsFromWellFile(const RimWellLogFile* wellLogFile)
{
    std::vector<RimWellLogFileChannel*> channels;
    if (wellLogFile != nullptr)
    {
        for (RimWellLogFileChannel* const channel : wellLogFile->wellLogChannels())
        {
            if (isFlowChannel(channel))
            {
                channels.push_back(channel);
            }
        }
    }
    return channels;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellPlotTools::wellPathFromWellLogFile(const RimWellLogFile* wellLogFile)
{
    RimProject* const project = RiaApplication::instance()->project();
    for (const auto& oilField : project->oilFields)
    {
        auto wellPaths = std::vector<RimWellPath*>(oilField->wellPathCollection()->wellPaths.begin(), oilField->wellPathCollection()->wellPaths.end());

        for (const auto& wellPath : wellPaths)
        {
            for (RimWellLogFile* const file : wellPath->wellLogFiles())
            {
                if (file == wellLogFile)
                {
                    return wellPath;
                }
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseResultCase*> RimWellPlotTools::gridCasesForWell(const QString& simWellName)
{
    std::vector<RimEclipseResultCase*> cases;
    const RimProject* project = RiaApplication::instance()->project();

    for (RimEclipseCase* eclCase : project->eclipseCases())
    {
        RimEclipseResultCase* resultCase = dynamic_cast<RimEclipseResultCase*>(eclCase);
        if (resultCase != nullptr)
        {
            if ( eclCase->eclipseCaseData()->findSimWellData(simWellName) )
            {
                cases.push_back(resultCase);
                break;
            }
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseResultCase*> RimWellPlotTools::rftCasesForWell(const QString& simWellName)
{
    std::vector<RimEclipseResultCase*> cases;
    const RimProject* project = RiaApplication::instance()->project();

    for (RimEclipseCase* eclCase : project->eclipseCases())
    {
        RimEclipseResultCase* resultCase = dynamic_cast<RimEclipseResultCase*>(eclCase);

        if (resultCase 
            && resultCase->rftReader() 
            && resultCase->rftReader()->wellNames().count(simWellName))
        {
            cases.push_back(resultCase);
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RimWellPlotTools::timeStepsFromRftCase(RimEclipseResultCase* rftCase,
                                                           const QString& simWellName)
{
    std::set<QDateTime> timeSteps;
    RifReaderEclipseRft* const reader = rftCase->rftReader();
    if (reader != nullptr)
    {
        for (const QDateTime& timeStep : reader->availableTimeSteps(simWellName, RifEclipseRftAddress::PRESSURE))
        {
            timeSteps.insert(timeStep);
        }
    }
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RimWellPlotTools::timeStepsFromGridCase(RimEclipseCase* gridCase)
{
    const RigEclipseCaseData* const eclipseCaseData = gridCase->eclipseCaseData();
    std::pair<size_t, QString> resultDataInfo = pressureResultDataInfo(eclipseCaseData);

    std::set<QDateTime> timeSteps;
    if (resultDataInfo.first != cvf::UNDEFINED_SIZE_T)
    {
        for (const QDateTime& timeStep : eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->timeStepDates(resultDataInfo.first))
        {
            timeSteps.insert(timeStep);
        }
    }
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RimWellPlotTools::timeStepFromWellLogFile(RimWellLogFile* wellLogFile)
{
    QDateTime timeStep = wellLogFile->date();
    return timeStep;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifDataSourceForRftPlt>> RimWellPlotTools::timeStepsMapFromRftCase(RimEclipseResultCase* rftCase, const QString& simWellName)
{
    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timeStepsMap;
    RifReaderEclipseRft* const reader = rftCase->rftReader();
    if (reader != nullptr)
    {
        for (const QDateTime& timeStep : reader->availableTimeSteps(simWellName, RifEclipseRftAddress::PRESSURE))
        {
            if (timeStepsMap.count(timeStep) == 0)
            {
                timeStepsMap.insert(std::make_pair(timeStep, std::set<RifDataSourceForRftPlt>()));
            }
            timeStepsMap[timeStep].insert(RifDataSourceForRftPlt(RifDataSourceForRftPlt::RFT, rftCase));
        }
    }
    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifDataSourceForRftPlt>> RimWellPlotTools::timeStepsMapFromGridCase(RimEclipseCase* gridCase)
{
    const RigEclipseCaseData* const eclipseCaseData = gridCase->eclipseCaseData();
    std::pair<size_t, QString> resultDataInfo = pressureResultDataInfo(eclipseCaseData);

    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timeStepsMap;
    if (resultDataInfo.first != cvf::UNDEFINED_SIZE_T)
    {
        for (const QDateTime& timeStep : eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->timeStepDates(resultDataInfo.first))
        {
            if (timeStepsMap.count(timeStep) == 0)
            {
                timeStepsMap.insert(std::make_pair(timeStep, std::set<RifDataSourceForRftPlt>()));
            }
            timeStepsMap[timeStep].insert(RifDataSourceForRftPlt(RifDataSourceForRftPlt::GRID, gridCase));
        }
    }
    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifDataSourceForRftPlt> > RimWellPlotTools::timeStepsMapFromWellLogFile(RimWellLogFile* wellLogFile)
{
    std::map<QDateTime, std::set<RifDataSourceForRftPlt> > timeStepsMap;

    QDateTime timeStep = wellLogFile->date();

    if (timeStepsMap.count(timeStep) == 0)
    {
        timeStepsMap.insert(std::make_pair(timeStep, std::set<RifDataSourceForRftPlt>()));
    }
    timeStepsMap[timeStep].insert(RifDataSourceForRftPlt(RifDataSourceForRftPlt::OBSERVED, wellLogFile));

    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifDataSourceForRftPlt>>
RimWellPlotTools::adjacentTimeSteps(const std::vector<std::pair<QDateTime, std::set<RifDataSourceForRftPlt>>>& allTimeSteps,
                                  const std::pair<QDateTime, std::set<RifDataSourceForRftPlt>>& searchTimeStepPair)
{
    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timeStepsMap;

    if (allTimeSteps.size() > 0)
    {
        auto itr = std::find_if(allTimeSteps.begin(), allTimeSteps.end(),
                                [searchTimeStepPair](const std::pair<QDateTime, std::set<RifDataSourceForRftPlt>>& dt)
        {
            return dt.first > searchTimeStepPair.first;
        });

        auto itrEnd = itr != allTimeSteps.end() ? itr + 1 : itr;

        for (itr = itrEnd - 1; itr != allTimeSteps.begin() && (*itr).first >= searchTimeStepPair.first; itr--);
        auto itrFirst = itr;

        timeStepsMap.insert(itrFirst, itrEnd);
    }

    // Add searched time step in case it is not included
    addTimeStepToMap(timeStepsMap, searchTimeStepPair);

    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::mapContainsTimeStep(const std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& map, const QDateTime& timeStep)
{
    return std::find_if(map.begin(), map.end(), [timeStep](const std::pair<QDateTime, std::set<RifDataSourceForRftPlt>>& pair)
    {
        return pair.first == timeStep;
    }) != map.end();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaRftPltCurveDefinition RimWellPlotTools::curveDefFromCurve(const RimWellLogCurve* curve)
{
    const RimWellLogRftCurve* rftCurve = dynamic_cast<const RimWellLogRftCurve*>(curve);
    const RimWellLogExtractionCurve* gridCurve = dynamic_cast<const RimWellLogExtractionCurve*>(curve);
    const RimWellLogFileCurve* wellLogFileCurve = dynamic_cast<const RimWellLogFileCurve*>(curve);

    if (rftCurve != nullptr)
    {
        RimEclipseResultCase* rftCase = dynamic_cast<RimEclipseResultCase*>(rftCurve->eclipseResultCase());
        if (rftCase != nullptr)
        {
            const RifEclipseRftAddress rftAddress = rftCurve->rftAddress();
            const QDateTime timeStep = rftAddress.timeStep();
            return RiaRftPltCurveDefinition(RifDataSourceForRftPlt(RifDataSourceForRftPlt::RFT, rftCase), timeStep);
        }
    }
    else if (gridCurve != nullptr)
    {
        RimEclipseResultCase* gridCase = dynamic_cast<RimEclipseResultCase*>(gridCurve->rimCase());
        if (gridCase != nullptr)
        {
            size_t timeStepIndex = gridCurve->currentTimeStep();
            const std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepsMap = timeStepsMapFromGridCase(gridCase);
            auto timeStepsVector = std::vector<std::pair<QDateTime, std::set<RifDataSourceForRftPlt>>>(
                timeStepsMap.begin(), timeStepsMap.end());
            if (timeStepIndex < timeStepsMap.size())
            {
                return RiaRftPltCurveDefinition(RifDataSourceForRftPlt(RifDataSourceForRftPlt::GRID, gridCase),
                                      timeStepsVector[timeStepIndex].first);
            }
        }
    }
    else if (wellLogFileCurve != nullptr)
    {
        const RimWellPath* const wellPath = wellLogFileCurve->wellPath();
        RimWellLogFile* const wellLogFile = wellLogFileCurve->wellLogFile();

        if (wellLogFile != nullptr)
        {
            const QDateTime date = wellLogFile->date();

            if (date.isValid())
            {
                return RiaRftPltCurveDefinition(RifDataSourceForRftPlt(RifDataSourceForRftPlt::OBSERVED, wellLogFile), date);
            }
        }
    }
    return RiaRftPltCurveDefinition(RifDataSourceForRftPlt(), QDateTime());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellPlotTools::wellPathByWellPathNameOrSimWellName(const QString& wellPathNameOrSimwellName)
{
    RimProject* proj = RiaApplication::instance()->project();
    RimWellPath* wellPath = proj->wellPathByName(wellPathNameOrSimwellName);

    return wellPath != nullptr ? wellPath : proj->wellPathFromSimulationWell(wellPathNameOrSimwellName);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPlotTools::simWellName(const QString& wellPathNameOrSimWellName)
{
    RimWellPath* wellPath = wellPathByWellPathNameOrSimWellName(wellPathNameOrSimWellName);
    return wellPath != nullptr ? wellPath->associatedSimulationWell() : wellPathNameOrSimWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::tryMatchChannelName(const std::set<QString>& channelNames, const QString& channelNameToMatch)
{
    auto itr = std::find_if(channelNames.begin(), channelNames.end(), [&](const QString& channelName)
    {
        if (channelName.startsWith('^'))
        {
            std::regex pattern(channelName.toStdString());
            return std::regex_match(channelNameToMatch.toStdString(), pattern);
        }
        else
        {
            return (bool)channelName.contains(channelNameToMatch, Qt::CaseInsensitive);
        }
    });
    return itr != channelNames.end();
}
