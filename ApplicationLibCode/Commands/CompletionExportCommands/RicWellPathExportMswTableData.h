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

#include "Tools/enum_bitmask.hpp"

#include <expected>
#include <optional>
#include <vector>

#include <QDateTime>

class RigActiveCellInfo;
class RimEclipseCase;
class RimWellPath;
class RigMswTableData;
class RimMswCompletionParameters;
class RigWellPath;

struct WellPathCellIntersectionInfo;

//--------------------------------------------------------------------------------------------------
/// This class is responsible for exporting well path MSW table data
/// This class is based on RicWellPathExportMswCompletionsImpl
//--------------------------------------------------------------------------------------------------
class RicWellPathExportMswTableData
{
public:
    enum class CompletionType
    {
        NONE         = 0x00,
        PERFORATIONS = 0x01,
        FISHBONES    = 0x02,
        FRACTURES    = 0x04,
        ALL          = PERFORATIONS | FISHBONES | FRACTURES
    };

    // The intention is to extract MSW data from a single well. Any handling of multiple wells is supposed to be managed in a different class
    static std::expected<RigMswTableData, std::string> extractSingleWellMswData( RimEclipseCase* eclipseCase,
                                                                                 RimWellPath*    wellPath,
                                                                                 bool exportCompletionsAfterMainBoreSegments = true,
                                                                                 CompletionType completionType = CompletionType::ALL,
                                                                                 const std::optional<QDateTime>& exportDate = std::nullopt );

    static std::expected<RigMswTableData, std::string> extractSingleWellMsw( RimEclipseCase* eclipseCase,
                                                                             RimWellPath*    wellPath,
                                                                             bool            exportCompletionsAfterMainBoreSegments = true,
                                                                             CompletionType  completionType = CompletionType::ALL,
                                                                             const std::optional<QDateTime>& exportDate = std::nullopt );

    static CompletionType convertFromExportSettings( const class RicExportCompletionDataSettingsUi& settings );

    static std::vector<WellPathCellIntersectionInfo> generateCellSegments( const RimEclipseCase* eclipseCase, const RimWellPath* wellPath );

    static std::vector<WellPathCellIntersectionInfo> filterIntersections( const std::vector<WellPathCellIntersectionInfo>& intersections,
                                                                          double                                           initialMD,
                                                                          const RigWellPath*                               wellPathGeometry,
                                                                          const RimEclipseCase*                            eclipseCase );

    static std::vector<RimWellPath*> wellPathsWithTieIn( const RimWellPath* wellPath );

    static double computeIntitialMeasuredDepth( const RimEclipseCase*                            eclipseCase,
                                                const RimWellPath*                               wellPath,
                                                const RimMswCompletionParameters*                mswParameters,
                                                const std::vector<WellPathCellIntersectionInfo>& allIntersections );
};

ENABLE_BITMASK_OPERATORS( RicWellPathExportMswTableData::CompletionType )
