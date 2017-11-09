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


#include "RifWellRftAddressQMetaType.h"
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
///  
//==================================================================================================
class RimWellPlotTools
{
    static const std::set<QString> PRESSURE_DATA_NAMES;

public:
    static bool                                     hasPressureData(const RimWellLogFile* wellLogFile);
    static bool                                     isPressureChannel(RimWellLogFileChannel* channel);
    static bool                                     hasPressureData(RimEclipseResultCase* gridCase);
    static bool                                     hasPressureData(RimWellPath* wellPath);
    static std::pair<size_t, QString>               pressureResultDataInfo(const RigEclipseCaseData* eclipseCaseData);

    static void                                     addTimeStepToMap(std::map<QDateTime, std::set<RifWellRftAddress>>& destMap,
                                                                     const std::pair<QDateTime, std::set<RifWellRftAddress>>& timeStepToAdd);
    static void                                     addTimeStepsToMap(std::map<QDateTime, std::set<RifWellRftAddress>>& destMap,
                                                                      const std::map<QDateTime, std::set<RifWellRftAddress>>& timeStepsToAdd);

    static std::vector<RimWellLogFile*>             wellLogFilesContainingPressure(const QString& wellName);
    static RimWellLogFileChannel*                   getPressureChannelFromWellFile(const RimWellLogFile* wellLogFile);

    static RimWellPath*                             wellPathFromWellLogFile(const RimWellLogFile* wellLogFile);

    static std::vector<RimEclipseResultCase*>               gridCasesForWell(const QString& wellName);
    static std::vector<RimEclipseResultCase*>               rftCasesForWell(const QString& wellName);

    static std::set<QDateTime>                              timeStepsFromRftCase(RimEclipseResultCase* rftCase, const QString& wellName);
    static std::set<QDateTime>                              timeStepsFromGridCase(RimEclipseCase* gridCase);
    static QDateTime                                        timeStepFromWellLogFile(RimWellLogFile* wellLogFile);

    static std::map<QDateTime, std::set<RifWellRftAddress>> timeStepsMapFromRftCase(RimEclipseResultCase* rftCase, const QString& wellName);
    static std::map<QDateTime, std::set<RifWellRftAddress>> timeStepsMapFromGridCase(RimEclipseCase* gridCase);
    static std::map<QDateTime, std::set<RifWellRftAddress>> timeStepsMapFromWellLogFile(RimWellLogFile* wellLogFile);
    static std::map<QDateTime, std::set<RifWellRftAddress>> adjacentTimeSteps(const std::vector<std::pair<QDateTime, std::set<RifWellRftAddress>>>& allTimeSteps,
                                                                              const std::pair<QDateTime, std::set<RifWellRftAddress>>& searchTimeStepPair);
    static bool                                             mapContainsTimeStep(const std::map<QDateTime, std::set<RifWellRftAddress>>& map, const QDateTime& timeStep);

    static RiaRftPltCurveDefinition                 curveDefFromCurve(const RimWellLogCurve* curve);

    template<typename T>
    static void appendSet(std::set<T>& destSet, const std::set<T>& setToAppend);
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
void RimWellPlotTools::appendSet(std::set<T>& destSet, const std::set<T>& setToAppend)
{
    destSet.insert(setToAppend.begin(), setToAppend.end());
}

