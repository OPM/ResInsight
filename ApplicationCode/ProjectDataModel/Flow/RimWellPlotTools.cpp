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

#include "RifReaderEclipseRft.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::set<QString> RimWellPlotTools::PRESSURE_DATA_NAMES = { "PRESSURE", "PRES_FORM" };

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
void RimWellPlotTools::addTimeStepToMap(std::map<QDateTime, std::set<RifWellRftAddress>>& destMap,
                                      const std::pair<QDateTime, std::set<RifWellRftAddress>>& timeStepToAdd)
{
    auto timeStepMapToAdd = std::map<QDateTime, std::set<RifWellRftAddress>>{ timeStepToAdd };
    addTimeStepsToMap(destMap, timeStepMapToAdd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPlotTools::addTimeStepsToMap(std::map<QDateTime, std::set<RifWellRftAddress>>& destMap,
                                       const std::map<QDateTime, std::set<RifWellRftAddress>>& timeStepsToAdd)
{
    for (const auto& timeStepPair : timeStepsToAdd)
    {
        if (timeStepPair.first.isValid())
        {
            if (destMap.count(timeStepPair.first) == 0)
            {
                destMap.insert(std::make_pair(timeStepPair.first, std::set<RifWellRftAddress>()));
            }
            auto addresses = timeStepPair.second;
            destMap[timeStepPair.first].insert(addresses.begin(), addresses.end());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> RimWellPlotTools::wellLogFilesContainingPressure(const QString& wellName)
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
                size_t timeStepCount = timeStepsMapFromWellLogFile(file).size();

                if (timeStepCount == 0) continue;
                if (QString::compare(file->wellName(), wellName) != 0) continue;

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
std::vector<RimEclipseResultCase*> RimWellPlotTools::gridCasesForWell(const QString& wellName)
{
    std::vector<RimEclipseResultCase*> cases;
    const RimProject* const project = RiaApplication::instance()->project();

    for (RimEclipseCase* const eclCase : project->eclipseCases())
    {
        RimEclipseResultCase* resultCase = dynamic_cast<RimEclipseResultCase*>(eclCase);
        if (resultCase != nullptr)
        {
            cases.push_back(resultCase);
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseResultCase*> RimWellPlotTools::rftCasesForWell(const QString& wellName)
{
    std::vector<RimEclipseResultCase*> cases;
    const RimProject* const project = RiaApplication::instance()->project();

    for (RimEclipseCase* const eclCase : project->eclipseCases())
    {
        RimEclipseResultCase* resultCase = dynamic_cast<RimEclipseResultCase*>(eclCase);
        if (resultCase != nullptr && resultCase->rftReader() != nullptr)
        {
            cases.push_back(resultCase);
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RimWellPlotTools::timeStepsFromRftCase(RimEclipseResultCase* rftCase, const QString& wellName)
{
    std::set<QDateTime> timeSteps;
    RifReaderEclipseRft* const reader = rftCase->rftReader();
    if (reader != nullptr)
    {
        for (const QDateTime& timeStep : reader->availableTimeSteps(wellName, RifEclipseRftAddress::PRESSURE))
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
std::map<QDateTime, std::set<RifWellRftAddress>> RimWellPlotTools::timeStepsMapFromRftCase(RimEclipseResultCase* rftCase, const QString& wellName)
{
    std::map<QDateTime, std::set<RifWellRftAddress>> timeStepsMap;
    RifReaderEclipseRft* const reader = rftCase->rftReader();
    if (reader != nullptr)
    {
        for (const QDateTime& timeStep : reader->availableTimeSteps(wellName, RifEclipseRftAddress::PRESSURE))
        {
            if (timeStepsMap.count(timeStep) == 0)
            {
                timeStepsMap.insert(std::make_pair(timeStep, std::set<RifWellRftAddress>()));
            }
            timeStepsMap[timeStep].insert(RifWellRftAddress(RifWellRftAddress::RFT, rftCase));
        }
    }
    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifWellRftAddress>> RimWellPlotTools::timeStepsMapFromGridCase(RimEclipseCase* gridCase)
{
    const RigEclipseCaseData* const eclipseCaseData = gridCase->eclipseCaseData();
    std::pair<size_t, QString> resultDataInfo = pressureResultDataInfo(eclipseCaseData);

    std::map<QDateTime, std::set<RifWellRftAddress>> timeStepsMap;
    if (resultDataInfo.first != cvf::UNDEFINED_SIZE_T)
    {
        for (const QDateTime& timeStep : eclipseCaseData->results(RiaDefines::MATRIX_MODEL)->timeStepDates(resultDataInfo.first))
        {
            if (timeStepsMap.count(timeStep) == 0)
            {
                timeStepsMap.insert(std::make_pair(timeStep, std::set<RifWellRftAddress>()));
            }
            timeStepsMap[timeStep].insert(RifWellRftAddress(RifWellRftAddress::GRID, gridCase));
        }
    }
    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifWellRftAddress> > RimWellPlotTools::timeStepsMapFromWellLogFile(RimWellLogFile* wellLogFile)
{
    std::map<QDateTime, std::set<RifWellRftAddress> > timeStepsMap;

    QDateTime timeStep = wellLogFile->date();

    if (timeStepsMap.count(timeStep) == 0)
    {
        timeStepsMap.insert(std::make_pair(timeStep, std::set<RifWellRftAddress>()));
    }
    timeStepsMap[timeStep].insert(RifWellRftAddress(RifWellRftAddress::OBSERVED, wellLogFile));

    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifWellRftAddress>>
RimWellPlotTools::adjacentTimeSteps(const std::vector<std::pair<QDateTime, std::set<RifWellRftAddress>>>& allTimeSteps,
                                  const std::pair<QDateTime, std::set<RifWellRftAddress>>& searchTimeStepPair)
{
    std::map<QDateTime, std::set<RifWellRftAddress>> timeStepsMap;

    if (allTimeSteps.size() > 0)
    {
        auto itr = std::find_if(allTimeSteps.begin(), allTimeSteps.end(),
                                [searchTimeStepPair](const std::pair<QDateTime, std::set<RifWellRftAddress>>& dt)
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
bool RimWellPlotTools::mapContainsTimeStep(const std::map<QDateTime, std::set<RifWellRftAddress>>& map, const QDateTime& timeStep)
{
    return std::find_if(map.begin(), map.end(), [timeStep](const std::pair<QDateTime, std::set<RifWellRftAddress>>& pair)
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
            return RiaRftPltCurveDefinition(RifWellRftAddress(RifWellRftAddress::RFT, rftCase), timeStep);
        }
    }
    else if (gridCurve != nullptr)
    {
        RimEclipseResultCase* gridCase = dynamic_cast<RimEclipseResultCase*>(gridCurve->rimCase());
        if (gridCase != nullptr)
        {
            size_t timeStepIndex = gridCurve->currentTimeStep();
            const std::map<QDateTime, std::set<RifWellRftAddress>>& timeStepsMap = timeStepsMapFromGridCase(gridCase);
            auto timeStepsVector = std::vector<std::pair<QDateTime, std::set<RifWellRftAddress>>>(
                timeStepsMap.begin(), timeStepsMap.end());
            if (timeStepIndex < timeStepsMap.size())
            {
                return RiaRftPltCurveDefinition(RifWellRftAddress(RifWellRftAddress::GRID, gridCase),
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
                return RiaRftPltCurveDefinition(RifWellRftAddress(RifWellRftAddress::OBSERVED, wellLogFile), date);
            }
        }
    }
    return RiaRftPltCurveDefinition(RifWellRftAddress(), QDateTime());
}
