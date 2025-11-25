/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicWellPathExportMswCompletionsImpl.h"

#include "RiaLogging.h"

#include "RicExportCompletionDataSettingsUi.h"
#include "RicWellPathExportCompletionsFileTools.h"
#include "RicWellPathExportMswTableData.h"

#include "RimEclipseCase.h"
#include "RimWellPath.h"

#include "CompletionsMsw/RigMswDataFormatter.h"
#include "CompletionsMsw/RigMswTableData.h"
#include "CompletionsMsw/RigMswUnifiedData.h"

#include "RifTextDataTableFormatter.h"

#include <QFileInfo>

namespace internal
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigMswUnifiedData extractUnifiedMswData( RimEclipseCase* eclipseCase, int timeStep, const std::vector<RimWellPath*>& wellPaths )
{
    RigMswUnifiedData unifiedData;

    for ( RimWellPath* wellPath : wellPaths )
    {
        bool exportAfterMainbore = true;
        auto wellData = RicWellPathExportMswTableData::extractSingleWellMswData( eclipseCase, wellPath, timeStep, exportAfterMainbore );
        if ( wellData.has_value() )
        {
            unifiedData.addWellData( std::move( wellData.value() ) );
        }
    }

    return unifiedData;
}

//--------------------------------------------------------------------------------------------------
/// Export MSW data to unified files using new data extraction approach
//--------------------------------------------------------------------------------------------------
void exportUnifiedMswData( const RicExportCompletionDataSettingsUi& exportSettings,
                           const QString&                           exportFolder,
                           const std::vector<RimWellPath*>&         wellPaths )
{
    // Extract all MSW data using new approach
    RigMswUnifiedData unifiedData = extractUnifiedMswData( exportSettings.caseToApply, exportSettings.timeStep, wellPaths );

    if ( unifiedData.isEmpty() )
    {
        RiaLogging::warning( "No MSW data to export." );
        return;
    }

    auto generateFileName = []( const RicExportCompletionDataSettingsUi& exportSettings, const QString& postFix ) -> std::pair<QString, QString>
    {
        QFileInfo fi( exportSettings.customFileName() );
        if ( !exportSettings.customFileName().isEmpty() )
        {
            auto candidate = fi.baseName() + postFix;
            return { candidate, fi.suffix() };
        }
        else
        {
            return { QString( "UnifiedCompletions_%1_%2" ).arg( exportSettings.caseToApply->caseUserDescription() ).arg( postFix ), "" };
        }
    };

    // Set up file names
    auto [fileName, suffix] = generateFileName( exportSettings, "_MSW" );

    // Create main grid file
    auto mainGridFile =
        RicWellPathExportCompletionsFileTools::openFileForExport( exportFolder, fileName, suffix, exportSettings.exportDataSourceAsComment() );

    // Write main grid data
    {
        QTextStream               stream( mainGridFile.get() );
        RifTextDataTableFormatter formatter( stream );
        formatter.setOptionalComment( exportSettings.exportDataSourceAsComment() );

        RigMswDataFormatter::formatMswTables( formatter, unifiedData );
    }

    // Create LGR file if needed
    if ( unifiedData.hasAnyLgrData() )
    {
        auto [lgrFileName, suffix] = generateFileName( exportSettings, "_MSW_LGR" );

        auto lgrFile = RicWellPathExportCompletionsFileTools::openFileForExport( exportFolder,
                                                                                 lgrFileName,
                                                                                 suffix,
                                                                                 exportSettings.exportDataSourceAsComment() );

        QTextStream               stream( lgrFile.get() );
        RifTextDataTableFormatter formatter( stream );
        formatter.setOptionalComment( exportSettings.exportDataSourceAsComment() );

        for ( const auto& tableData : unifiedData.wellDataList() )
        {
            RigMswDataFormatter::formatCompsegsTable( formatter, tableData, true ); // LGR only
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Export MSW data to separate files per well using new data extraction approach
//--------------------------------------------------------------------------------------------------
void exportSplitMswData( const RicExportCompletionDataSettingsUi& exportSettings,
                         const QString&                           exportFolder,
                         const std::vector<RimWellPath*>&         wellPaths )
{
    for ( const auto& wellPath : wellPaths )
    {
        // Extract data for single well
        auto wellDataResult = RicWellPathExportMswTableData::extractSingleWellMswData( exportSettings.caseToApply,
                                                                                       wellPath,
                                                                                       exportSettings.timeStep,
                                                                                       exportSettings.exportCompletionWelspecAfterMainBore() );

        if ( !wellDataResult.has_value() )
        {
            continue; // Skip wells with no MSW data
        }

        auto wellData = wellDataResult.value();

        // Set up file names
        QString wellFileName = QString( "%1_Completions_MSW_%2" ).arg( wellPath->name(), exportSettings.caseToApply->caseUserDescription() );

        // Create main grid file
        auto mainGridFile = RicWellPathExportCompletionsFileTools::openFileForExport( exportFolder,
                                                                                      wellFileName,
                                                                                      "",
                                                                                      exportSettings.exportDataSourceAsComment() );

        // Write main grid data
        {
            QTextStream               stream( mainGridFile.get() );
            RifTextDataTableFormatter formatter( stream );
            formatter.setOptionalComment( exportSettings.exportDataSourceAsComment() );

            RigMswDataFormatter::formatMswTables( formatter, wellData );
        }

        // Create LGR file if needed
        if ( wellData.hasLgrData() )
        {
            auto lgrFile = RicWellPathExportCompletionsFileTools::openFileForExport( exportFolder,
                                                                                     wellFileName + "_LGR",
                                                                                     "",
                                                                                     exportSettings.exportDataSourceAsComment() );

            QTextStream               stream( lgrFile.get() );
            RifTextDataTableFormatter formatter( stream );
            formatter.setOptionalComment( exportSettings.exportDataSourceAsComment() );

            RigMswDataFormatter::formatCompsegsTable( formatter, wellData, true ); // LGR only
        }
    }
}

}; // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForAllCompletions( const RicExportCompletionDataSettingsUi& exportSettings,
                                                                               const std::vector<RimWellPath*>&         wellPaths )
{
    // There are two modes of MSW export, either split on well or unified. The split on completion type is used for export of COMPDAT
    if ( exportSettings.fileSplit() == RicExportCompletionDataSettingsUi::ExportSplit::SPLIT_ON_WELL ||
         exportSettings.fileSplit() == RicExportCompletionDataSettingsUi::ExportSplit::SPLIT_ON_WELL_AND_COMPLETION_TYPE )
    {
        internal::exportSplitMswData( exportSettings, exportSettings.folder(), wellPaths );
    }
    else if ( exportSettings.fileSplit() == RicExportCompletionDataSettingsUi::ExportSplit::UNIFIED_FILE )
    {
        internal::exportUnifiedMswData( exportSettings, exportSettings.folder(), wellPaths );
    }
}
