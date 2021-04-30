/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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
class RifTextDataTableFormatter;
class RicMswBranch;
class RicMswSegment;
class RicMswValve;
class RicMswCompletion;
class RimWellPath;

namespace RicMswTableFormatterTools
{
class CvfVec3stComparator
{
public:
    bool operator()( const cvf::Vec3st& lhs, const cvf::Vec3st& rhs ) const
    {
        if ( lhs.z() == rhs.z() )
        {
            if ( lhs.y() == rhs.y() )
            {
                return lhs.x() < rhs.x();
            }
            return lhs.y() < rhs.y();
        }
        return lhs.z() < rhs.z();
    }
};

void generateWelsegsTable( RifTextDataTableFormatter& formatter,
                           RicMswExportInfo&          exportInfo,
                           double                     maxSegmentLength,
                           bool                       exportCompletionSegmentsAfterMainBore );

void writeWelsegsSegmentsRecursively( RifTextDataTableFormatter&   formatter,
                                      RicMswExportInfo&            exportInfo,
                                      gsl::not_null<RicMswBranch*> branch,
                                      gsl::not_null<int*>          segmentNumber,
                                      double                       maxSegmentLength,
                                      bool                         exportCompletionSegmentsAfterMainBore,
                                      RicMswSegment*               connectedToSegment );

void writeWelsegsSegment( RicMswSegment*               segment,
                          const RicMswSegment*         previousSegment,
                          RifTextDataTableFormatter&   formatter,
                          RicMswExportInfo&            exportInfo,
                          double                       maxSegmentLength,
                          gsl::not_null<RicMswBranch*> branch,
                          int*                         segmentNumber );

void writeValveWelsegsSegment( const RicMswSegment*       outletSegment,
                               RicMswValve*               valve,
                               RifTextDataTableFormatter& formatter,
                               RicMswExportInfo&          exportInfo,
                               double                     maxSegmentLength,
                               int*                       segmentNumber );

void writeCompletionWelsegsSegments( gsl::not_null<const RicMswSegment*>    outletSegment,
                                     gsl::not_null<const RicMswCompletion*> completion,
                                     RifTextDataTableFormatter&             formatter,
                                     RicMswExportInfo&                      exportInfo,
                                     double                                 maxSegmentLength,
                                     int*                                   segmentNumber );

void writeCompletionsForSegment( gsl::not_null<const RicMswSegment*> outletSegment,
                                 gsl::not_null<RicMswSegment*>       segment,
                                 RicMswValve**                       outletValve,
                                 RifTextDataTableFormatter&          formatter,
                                 RicMswExportInfo&                   exportInfo,
                                 double                              maxSegmentLength,
                                 int*                                segmentNumber );

void writeWelsegsCompletionCommentHeader( RifTextDataTableFormatter&        formatter,
                                          RigCompletionData::CompletionType completionType );

void generateCompsegTables( RifTextDataTableFormatter& formatter, RicMswExportInfo& exportInfo, bool exportLgrData );

void generateCompsegTable( RifTextDataTableFormatter&                         formatter,
                           RicMswExportInfo&                                  exportInfo,
                           gsl::not_null<const RicMswBranch*>                 branch,
                           bool                                               exportSubGridIntersections,
                           const std::set<RigCompletionData::CompletionType>& exportCompletionTypes,
                           gsl::not_null<bool*>                               headerGenerated,
                           gsl::not_null<std::set<size_t>*>                   intersectedCells );

void generateCompsegHeader( RifTextDataTableFormatter&        formatter,
                            RicMswExportInfo&                 exportInfo,
                            RigCompletionData::CompletionType completionType,
                            bool                              exportSubGridIntersections );

void generateWsegvalvTable( RifTextDataTableFormatter& formatter, RicMswExportInfo& exportInfo );

void generateWsegvalvTableRecursively( RifTextDataTableFormatter&   formatter,
                                       gsl::not_null<RicMswBranch*> branch,
                                       bool&                        isHeaderWritten,
                                       const QString&               wellNameForExport );

void generateWsegAicdTable( RifTextDataTableFormatter& formatter, RicMswExportInfo& exportInfo );

std::vector<std::pair<double, double>> createSubSegmentMDPairs( double startMD, double endMD, double maxSegmentLength );

double tvdFromMeasuredDepth( gsl::not_null<const RimWellPath*> wellPath, double measuredDepth );

void writeWsegvalHeader( RifTextDataTableFormatter& formatter );

} // namespace RicMswTableFormatterTools
