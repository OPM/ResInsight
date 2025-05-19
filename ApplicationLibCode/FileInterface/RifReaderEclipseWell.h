/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 Equinor ASA
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

#include <map>
#include <vector>

#include "cvfVector3.h"
#include <QDateTime>

class RifEclipseRestartDataAccess;
class RigGridBase;
class RigEclipseCaseData;

struct RigWellResultPoint;
struct SegmentPositionContribution;

// NOLINTBEGIN(modernize-use-using)
typedef struct well_conn_struct               well_conn_type;
typedef struct well_segment_struct            well_segment_type;
typedef struct well_segment_collection_struct well_segment_collection_type;
// NOLINTEND(modernize-use-using)

//==================================================================================================
//
//
//
//==================================================================================================
class RifReaderEclipseWell
{
private:
    RifReaderEclipseWell() {};

public:
    static void readWellCells( RifEclipseRestartDataAccess* restartDataAccess,
                               RigEclipseCaseData*          eclipseCaseData,
                               std::vector<QDateTime>       filteredTimeSteps,
                               std::vector<std::string>     gridNames,
                               bool                         importCompleteMswData );

    static size_t
        localGridCellIndexFromErtConnection( const RigGridBase* grid, const well_conn_type* ert_connection, const char* wellNameForErrorMsgs );

private:
    static RigWellResultPoint createWellResultPoint( const RigEclipseCaseData* eCaseData,
                                                     const RigGridBase*        grid,
                                                     const well_conn_type*     ert_connection,
                                                     const char*               wellName );
    static RigWellResultPoint createWellResultPoint( const RigEclipseCaseData* eCaseData,
                                                     const RigGridBase*        grid,
                                                     const well_conn_type*     ert_connection,
                                                     const well_segment_type*  segment,
                                                     const char*               wellName );

    static cvf::Vec3d interpolate3DPosition( const std::vector<SegmentPositionContribution>& positions );
    static void       propagatePosContribDownwards( std::map<int, std::vector<SegmentPositionContribution>>& segmentIdToPositionContrib,
                                                    const well_segment_collection_type*                      allErtSegments,
                                                    int                                                      ertSegmentId,
                                                    std::vector<SegmentPositionContribution>                 posContrib );

    static std::string ertGridName( const RigEclipseCaseData* eCaseData, size_t gridNr );
};
