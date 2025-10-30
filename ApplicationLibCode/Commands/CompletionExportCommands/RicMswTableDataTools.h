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

#include "cvfVector3.h"

#include <gsl/gsl>

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

// New data collection functions (replace formatter versions)
void collectWelsegsData( RigMswTableData&  tableData,
                         RicMswExportInfo& exportInfo,
                         double            maxSegmentLength,
                         bool              exportCompletionSegmentsAfterMainBore );

void collectWelsegsDataRecursively( RigMswTableData&             tableData,
                                    RicMswExportInfo&            exportInfo,
                                    gsl::not_null<RicMswBranch*> branch,
                                    gsl::not_null<int*>          segmentNumber,
                                    double                       maxSegmentLength,
                                    bool                         exportCompletionSegmentsAfterMainBore,
                                    RicMswSegment*               connectedToSegment );

void collectCompsegData( RigMswTableData& tableData, RicMswExportInfo& exportInfo, bool exportSubGridIntersections );

void collectCompsegDataByType( RigMswTableData&                                   tableData,
                               RicMswExportInfo&                                  exportInfo,
                               gsl::not_null<const RicMswBranch*>                 branch,
                               bool                                               exportSubGridIntersections,
                               const std::set<RigCompletionData::CompletionType>& exportCompletionTypes,
                               gsl::not_null<std::set<size_t>*>                   intersectedCells );

void collectWsegvalvData( RigMswTableData& tableData, RicMswExportInfo& exportInfo );

void collectWsegvalvDataRecursively( RigMswTableData& tableData, gsl::not_null<RicMswBranch*> branch, const std::string& wellNameForExport );

void collectWsegAicdData( RigMswTableData& tableData, RicMswExportInfo& exportInfo );

void collectWsegAicdDataRecursively( RigMswTableData& tableData, RicMswExportInfo& exportInfo, gsl::not_null<const RicMswBranch*> branch );

// Helper functions for data collection
void collectWelsegsSegment( RigMswTableData&             tableData,
                            RicMswSegment*               segment,
                            const RicMswSegment*         previousSegment,
                            RicMswExportInfo&            exportInfo,
                            double                       maxSegmentLength,
                            gsl::not_null<RicMswBranch*> branch,
                            int*                         segmentNumber,
                            QString                      branchDescription );

void collectValveWelsegsSegment( RigMswTableData&     tableData,
                                 const RicMswSegment* outletSegment,
                                 RicMswValve*         valve,
                                 RicMswExportInfo&    exportInfo,
                                 double               maxSegmentLength,
                                 int*                 segmentNumber );

void collectCompletionsForSegment( RigMswTableData&                    tableData,
                                   gsl::not_null<const RicMswSegment*> outletSegment,
                                   gsl::not_null<RicMswSegment*>       segment,
                                   RicMswValve**                       outletValve,
                                   RicMswExportInfo&                   exportInfo,
                                   double                              maxSegmentLength,
                                   int*                                segmentNumber );

void collectCompletionWelsegsSegments( RigMswTableData&                    tableData,
                                       gsl::not_null<const RicMswSegment*> outletSegment,
                                       gsl::not_null<RicMswCompletion*>    completion,
                                       RicMswExportInfo&                   exportInfo,
                                       double                              maxSegmentLength,
                                       int*                                segmentNumber );

} // namespace RicMswTableDataTools
