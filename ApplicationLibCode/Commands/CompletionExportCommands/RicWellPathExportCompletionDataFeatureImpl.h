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

#include "cvfVector2.h"
#include "cvfVector3.h"

#include <QDateTime>
#include <QFile>

#include <gsl/gsl>

#include <memory>
#include <optional>
#include <vector>

class RicMswCompletion;
class RigCell;
class RigEclipseCaseData;
class RigMainGrid;
class RimEclipseCase;
class RimFishbones;
class RimPerforationInterval;
class RimWellPath;
class RimWellPathValve;
class RimWellPathFracture;
class RimNonDarcyPerforationParameters;
class RifTextDataTableFormatter;
class RigVirtualPerforationTransmissibilities;
class SubSegmentIntersectionInfo;

//==================================================================================================
///
//==================================================================================================
using QFilePtr = std::shared_ptr<QFile>;

//==================================================================================================
///
//==================================================================================================
class RicWellPathExportCompletionDataFeatureImpl
{
public:
    static void exportCompletions( const std::vector<RimWellPath*>& wellPaths, const RicExportCompletionDataSettingsUi& exportSettings );

    static std::vector<RigCompletionData> computeStaticCompletionsForWellPath( RimWellPath* wellPath, RimEclipseCase* eclipseCase );

    static std::vector<RigCompletionData>
        computeDynamicCompletionsForWellPath( RimWellPath* wellPath, RimEclipseCase* eclipseCase, size_t timeStepIndex );

    static std::vector<RigCompletionData>
        completionDataForWellPath( RimWellPath* wellPath, RimEclipseCase* eCase, const std::optional<QDateTime>& exportDate = std::nullopt );

    static std::pair<double, cvf::Vec2i> wellPathUpperGridIntersectionIJ( gsl::not_null<const RimEclipseCase*> gridCase,
                                                                          gsl::not_null<const RimWellPath*>    wellPath,
                                                                          const QString&                       gridName = "" );

    static std::optional<QDateTime> exportDateForTimeStep( const RimEclipseCase* eclipseCase, size_t timeStepIndex );

private:
    static std::vector<RigCompletionData> generatePerforationsCompdatValues( gsl::not_null<const RimWellPath*>                 wellPath,
                                                                             const std::vector<const RimPerforationInterval*>& intervals,
                                                                             const RicExportCompletionDataSettingsUi&          settings,
                                                                             const std::optional<QDateTime>& exportDate = std::nullopt );

    static RigCompletionData combineEclipseCellCompletions( const std::vector<RigCompletionData>&    completions,
                                                            const RicExportCompletionDataSettingsUi& settings );

    static std::vector<RigCompletionData> mainGridCompletions( const std::vector<RigCompletionData>& allCompletions );

    static std::map<QString, std::vector<RigCompletionData>> subGridsCompletions( const std::vector<RigCompletionData>& allCompletions );

    static void exportWellPathFractureReport( RimEclipseCase*                                   sourceCase,
                                              QFilePtr                                          exportFile,
                                              const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems );

    static void exportWelspecsToFile( RimEclipseCase*                       gridCase,
                                      QFilePtr                              exportFile,
                                      const std::vector<RigCompletionData>& completions,
                                      bool                                  exportDataSourceAsComment );

    static void exportWelspeclToFile( RimEclipseCase*                                          gridCase,
                                      QFilePtr                                                 exportFile,
                                      const std::map<QString, std::vector<RigCompletionData>>& completions );

    static void sortAndExportCompletionsToFile( RimEclipseCase*                                      eclipseCase,
                                                const QString&                                       exportFolder,
                                                const QString&                                       fileName,
                                                const std::vector<RigCompletionData>&                completions,
                                                const std::vector<RicWellPathFractureReportItem>&    wellPathFractureReportItems,
                                                RicExportCompletionDataSettingsUi::CompdatExportType exportType,
                                                bool                                                 exportDataSourceAsComment,
                                                bool                                                 exportWelspec );

    static void exportCompdatAndWpimultTables( RimEclipseCase*                                          sourceCase,
                                               QFilePtr                                                 exportFile,
                                               const std::map<QString, std::vector<RigCompletionData>>& completionsPerGrid,
                                               RicExportCompletionDataSettingsUi::CompdatExportType     exportType,
                                               bool                                                     exportDataSourceAsComment );

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

    static void exportCarfinForTemporaryLgrs( const RimEclipseCase* sourceCase, const QString& folder );

    static RimWellPath* topLevelWellPath( const RigCompletionData& completion );
};
