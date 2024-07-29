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

#include "RifEclipseRftAddress.h"

#include "RimWellLogLasFile.h"

#include <map>
#include <set>

class RimEclipseCase;
class RimEclipseResultCase;
class RimObservedFmuRftData;
class RimSummaryEnsemble;
class RimWellLogCurve;
class RimWellLogChannel;
class RimWellLogPlot;
class RimWellPath;
class RimPressureDepthData;
class RiuWellRftPlot;
class RigEclipseCaseData;
class RigEclipseResultAddress;
class RifReaderRftInterface;

namespace RimWellPlotTools
{

enum class FlowType
{
    FLOW_TYPE_PHASE_SPLIT,
    FLOW_TYPE_TOTAL
};
enum class FlowPhase
{
    FLOW_PHASE_NONE,
    FLOW_PHASE_OIL,
    FLOW_PHASE_GAS,
    FLOW_PHASE_WATER,
    FLOW_PHASE_TOTAL
};

bool      isOilFlowChannel( const QString& channelName );
bool      isGasFlowChannel( const QString& channelName );
bool      isWaterFlowChannel( const QString& channelName );
bool      isTotalFlowChannel( const QString& channelName );
FlowPhase flowPhaseFromChannelName( const QString& channelName );

std::vector<RimWellLogFile*> wellLogFilesContainingFlow( const QString& wellName );
RimWellPath*                 wellPathByWellPathNameOrSimWellName( const QString& wellPathNameOrSimwellName );
std::vector<RimWellPath*>    wellPathsContainingFlow();

void                                                  addTimeStepsToMap( std::map<QDateTime, std::set<RifDataSourceForRftPlt>>&       destMap,
                                                                         const std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepsToAdd );
std::vector<RimWellLogFile*>                          wellLogFilesContainingPressure( const QString& wellPathNameOrSimWellName );
RimWellLogChannel*                                    getPressureChannelFromWellFile( const RimWellLogFile* wellLogFile );
RimWellPath*                                          wellPathFromWellLogFile( const RimWellLogFile* wellLogFile );
std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timeStepsMapFromGridCase( RimEclipseCase* gridCase );
RiaRftPltCurveDefinition                              curveDefFromCurve( const RimWellLogCurve* curve );

bool hasFlowData( const RimWellLog* wellLog );
bool hasAssociatedWellPath( const QString& wellName );

std::vector<RimEclipseResultCase*>  gridCasesForWell( const QString& simWellName );
std::vector<RimEclipseResultCase*>  rftCasesForWell( const QString& simWellName );
std::vector<RimSummaryEnsemble*>    rftEnsemblesForWell( const QString& simWellName );
std::vector<RimSummaryEnsemble*>    rftEnsembles();
std::vector<RimObservedFmuRftData*> observedFmuRftDataForWell( const QString& simWellName );
std::vector<RimObservedFmuRftData*> observedFmuRftData();
QString                             simWellName( const QString& wellPathNameOrSimWellName );

std::map<QDateTime, std::set<RifDataSourceForRftPlt>>
    calculateRelevantTimeStepsFromCases( const QString&                                               wellPathNameOrSimWellName,
                                         const std::vector<RifDataSourceForRftPlt>&                   selSources,
                                         const std::set<RifEclipseRftAddress::RftWellLogChannelType>& interestingRFTResults );

void calculateValueOptionsForTimeSteps( const QString&                                               wellPathNameOrSimWellName,
                                        const std::vector<RifDataSourceForRftPlt>&                   selSources,
                                        const std::set<RifEclipseRftAddress::RftWellLogChannelType>& interestingRFTResults,
                                        QList<caf::PdmOptionItemInfo>&                               options );

std::set<RiaRftPltCurveDefinition> curveDefsFromTimesteps( const QString&                             wellPathNameOrSimWellName,
                                                           const std::vector<QDateTime>&              selectedTimeStepVector,
                                                           bool                                       firstReportTimeStepIsValid,
                                                           const std::vector<RifDataSourceForRftPlt>& selectedSourcesExpanded,
                                                           const std::set<RifEclipseRftAddress::RftWellLogChannelType>& interestingRFTResults );

QString flowPlotAxisTitle( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem );
QString flowUnitText( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem );
QString flowVolumePlotAxisTitle( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem );
QString flowVolumeUnitText( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem );

QString curveUnitText( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem, FlowPhase flowPhase );

bool hasFlowData( const RimWellPath* wellPath );

std::vector<RimPressureDepthData*> pressureDepthData();
std::vector<RimPressureDepthData*> pressureDepthDataForWell( const QString& simWellName );

}; // namespace RimWellPlotTools
