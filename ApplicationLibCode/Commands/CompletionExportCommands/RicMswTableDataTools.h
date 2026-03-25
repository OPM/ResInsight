/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RigCompletionData.h"

#include "RimWellPathAicdParameters.h"
#include "RimWellPathSicdParameters.h"

#include "cvfVector3.h"

#include <gsl/gsl>
#include <optional>

#include <QDateTime>

class RicMswExportInfo;
class RigMswTableData;
class RifTextDataTableFormatter;
class RicMswBranch;
class RicMswSegment;
class RicMswValve;
class RicMswCompletion;
class RimWellPath;

//--------------------------------------------------------------------------------------------------
/// WIP
/// This is a helper class namespace for functions to generate MSW table data for well segments and completions
/// This class is based on the existing RicMswTableFormatterTools, but with functions to collect data into RigMswTableData
//--------------------------------------------------------------------------------------------------
namespace RicMswTableDataTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class AicdWsegvalveData
{
public:
    explicit AicdWsegvalveData( const QString&                             wellName,
                                const QString&                             comment,
                                int                                        segmentNumber,
                                double                                     flowScalingFactor,
                                bool                                       isOpen,
                                const std::array<double, AICD_NUM_PARAMS>& values )
        : m_wellName( wellName )
        , m_comment( comment )
        , m_segmentNumber( segmentNumber )
        , m_flowScalingFactor( flowScalingFactor )
        , m_isOpen( isOpen )
        , m_values( values )

    {
    }

    QString                             m_wellName;
    QString                             m_comment;
    int                                 m_segmentNumber;
    double                              m_flowScalingFactor;
    bool                                m_isOpen;
    std::array<double, AICD_NUM_PARAMS> m_values;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class SicdWsegvalveData
{
public:
    explicit SicdWsegvalveData( const QString&                             wellName,
                                const QString&                             comment,
                                int                                        segmentNumber,
                                double                                     flowScalingFactor,
                                bool                                       isOpen,
                                const std::array<double, SICD_NUM_PARAMS>& values )
        : m_wellName( wellName )
        , m_comment( comment )
        , m_segmentNumber( segmentNumber )
        , m_flowScalingFactor( flowScalingFactor )
        , m_isOpen( isOpen )
        , m_values( values )

    {
    }

    QString                             m_wellName;
    QString                             m_comment;
    int                                 m_segmentNumber;
    double                              m_flowScalingFactor;
    bool                                m_isOpen;
    std::array<double, SICD_NUM_PARAMS> m_values;
};

// New data collection functions (replace formatter versions)
void collectWelsegsData( RigMswTableData&                              tableData,
                         RicMswExportInfo&                             exportInfo,
                         double                                        maxSegmentLength,
                         const std::vector<std::pair<double, double>>& customSegmentIntervals,
                         bool                                          exportCompletionSegmentsAfterMainBore,
                         const std::optional<QDateTime>&               exportDate = std::nullopt );

void collectWelsegsDataRecursively( RigMswTableData&                              tableData,
                                    RicMswExportInfo&                             exportInfo,
                                    gsl::not_null<RicMswBranch*>                  branch,
                                    gsl::not_null<int*>                           segmentNumber,
                                    double                                        maxSegmentLength,
                                    const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                    bool                                          exportCompletionSegmentsAfterMainBore,
                                    RicMswSegment*                                connectedToSegment,
                                    const std::optional<QDateTime>&               exportDate = std::nullopt );

void collectCompsegData( RigMswTableData&                tableData,
                         RicMswExportInfo&               exportInfo,
                         bool                            exportSubGridIntersections,
                         const std::optional<QDateTime>& exportDate = std::nullopt );

void collectCompsegDataByType( RigMswTableData&                                   tableData,
                               RicMswExportInfo&                                  exportInfo,
                               gsl::not_null<const RicMswBranch*>                 branch,
                               bool                                               exportSubGridIntersections,
                               const std::set<RigCompletionData::CompletionType>& exportCompletionTypes,
                               gsl::not_null<std::set<size_t>*>                   intersectedCells,
                               const std::optional<QDateTime>&                    exportDate = std::nullopt );

void collectWsegvalvData( RigMswTableData& tableData, RicMswExportInfo& exportInfo, const std::optional<QDateTime>& exportDate = std::nullopt );

void collectStandaloneWsegvalvDataRecursively( RigMswTableData& tableData, RicMswBranch* branch, std::string wellName );

void collectWsegvalvDataRecursively( RigMswTableData&                tableData,
                                     gsl::not_null<RicMswBranch*>    branch,
                                     const std::string&              wellNameForExport,
                                     const std::optional<QDateTime>& exportDate = std::nullopt );

void collectWsegAicdData( RigMswTableData& tableData, RicMswExportInfo& exportInfo, const std::optional<QDateTime>& exportDate = std::nullopt );
void collectWsegSicdData( RigMswTableData& tableData, RicMswExportInfo& exportInfo, const std::optional<QDateTime>& exportDate = std::nullopt );

// Helper functions for data collection

void collectWelsegsSegment( RigMswTableData&                              tableData,
                            RicMswSegment*                                segment,
                            const RicMswSegment*                          previousSegment,
                            RicMswExportInfo&                             exportInfo,
                            double                                        maxSegmentLength,
                            const std::vector<std::pair<double, double>>& customSegmentIntervals,
                            gsl::not_null<RicMswBranch*>                  branch,
                            int*                                          segmentNumber,
                            QString                                       branchDescription,
                            const std::optional<QDateTime>&               exportDate = std::nullopt );

void collectValveWelsegsSegment( RigMswTableData&                              tableData,
                                 const RicMswSegment*                          outletSegment,
                                 RicMswValve*                                  valve,
                                 RicMswExportInfo&                             exportInfo,
                                 double                                        maxSegmentLength,
                                 const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                 int*                                          segmentNumber );

void collectCompletionsForSegment( RigMswTableData&                              tableData,
                                   gsl::not_null<const RicMswSegment*>           outletSegment,
                                   gsl::not_null<RicMswSegment*>                 segment,
                                   RicMswValve**                                 outletValve,
                                   RicMswExportInfo&                             exportInfo,
                                   double                                        maxSegmentLength,
                                   const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                   int*                                          segmentNumber );

void collectCompletionWelsegsSegments( RigMswTableData&                              tableData,
                                       gsl::not_null<const RicMswSegment*>           outletSegment,
                                       gsl::not_null<RicMswCompletion*>              completion,
                                       RicMswExportInfo&                             exportInfo,
                                       double                                        maxSegmentLength,
                                       const std::vector<std::pair<double, double>>& customSegmentIntervals,
                                       int*                                          segmentNumber );

void generateWsegAicdTableRecursively( RicMswExportInfo&                                 exportInfo,
                                       gsl::not_null<const RicMswBranch*>                branch,
                                       std::map<size_t, std::vector<AicdWsegvalveData>>& aicdValveData,
                                       const std::optional<QDateTime>&                   exportDate = std::nullopt );

void generateWsegSicdTableRecursively( RicMswExportInfo&                                 exportInfo,
                                       gsl::not_null<const RicMswBranch*>                branch,
                                       std::map<size_t, std::vector<SicdWsegvalveData>>& sicdValveData,
                                       const std::optional<QDateTime>&                   exportDate = std::nullopt );

std::vector<std::pair<double, double>> createSubSegmentMDPairs( double                                        startMD,
                                                                double                                        endMD,
                                                                double                                        maxSegmentLength,
                                                                const std::vector<std::pair<double, double>>& customSegmentIntervals = {} );

double tvdFromMeasuredDepth( gsl::not_null<const RimWellPath*> wellPath, double measuredDepth );

inline constexpr double valveSegmentLength = 0.1;

} // namespace RicMswTableDataTools
