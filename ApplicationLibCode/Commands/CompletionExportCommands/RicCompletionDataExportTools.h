/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicExportCompletionDataSettingsUi.h"

#include <QDateTime>
#include <QString>

#include <map>
#include <optional>
#include <vector>

class RifTextDataTableFormatter;
class RimPerforationInterval;
class RimWellPath;

//==================================================================================================
/// Building blocks used when generating and writing COMPDAT, COMPLUMP and WPIMULT data
//==================================================================================================
class RicCompletionDataExportTools
{
public:
    static std::vector<RigCompletionData> generatePerforationsCompdatValues( const RimWellPath*                                wellPath,
                                                                             const std::vector<const RimPerforationInterval*>& intervals,
                                                                             const RicExportCompletionDataSettingsUi&          settings,
                                                                             const std::optional<QDateTime>& exportDate = std::nullopt );

    static RigCompletionData combineEclipseCellCompletions( const std::vector<RigCompletionData>&    completions,
                                                            const RicExportCompletionDataSettingsUi& settings );

    static std::vector<RigCompletionData> mainGridCompletions( const std::vector<RigCompletionData>& allCompletions );

    static std::map<QString, std::vector<RigCompletionData>> subGridsCompletions( const std::vector<RigCompletionData>& allCompletions );

    static void exportCompdatTableUsingFormatter( RifTextDataTableFormatter&            formatter,
                                                  const QString&                        gridName,
                                                  const std::vector<RigCompletionData>& completionData );

    static void exportComplumpTableUsingFormatter( RifTextDataTableFormatter&            formatter,
                                                   const QString&                        gridName,
                                                   const std::vector<RigCompletionData>& completionData );

    static void exportWpimultTableUsingFormatter( RifTextDataTableFormatter&            formatter,
                                                  const QString&                        gridName,
                                                  const std::vector<RigCompletionData>& completionData );

    static void appendCompletionData( std::map<size_t, std::vector<RigCompletionData>>* completionData,
                                      const std::vector<RigCompletionData>&             data );

    static RimWellPath* topLevelWellPath( const RigCompletionData& completion );
};
