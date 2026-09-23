/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
//  A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicExportScheduleFeature.h"

#include "RicScheduleDataGenerator.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaQStringFormatter.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellEventTimeline.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"
#include "RiuMessageDialog.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <format>

CAF_CMD_SOURCE_INIT( RicExportScheduleFeature, "RicExportScheduleFeature" );

namespace
{
const QString exportDialogName = "SCHEDULE_EXPORT";
const QString featureTitle     = "Export Schedule";
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<QString, QString>
    RicExportScheduleFeature::exportScheduleToFile( const RimWellEventTimeline& timeline, RimEclipseCase& eclipseCase, const QString& fileName )
{
    if ( fileName.isEmpty() ) return std::unexpected( QString( "No file name specified" ) );

    // Fixed defaults: multi-segment-well keywords for every well with events, the first
    // (simulation start) date as a comment, and aligned columns for readability.
    auto                         wellPaths = timeline.getWellPathsWithEvents();
    std::set<const RimWellPath*> mswWells( wellPaths.begin(), wellPaths.end() );

    auto scheduleText = RicScheduleDataGenerator::generateScheduleForTimeline( timeline, eclipseCase, mswWells, true, true );
    if ( !scheduleText ) return std::unexpected( scheduleText.error() );

    QFile file( fileName );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        return std::unexpected( QString( "Could not open file for writing: %1" ).arg( fileName ) );
    }

    QTextStream stream( &file );
    stream << *scheduleText;
    stream.flush();

    if ( stream.status() != QTextStream::Ok || file.error() != QFileDevice::NoError )
    {
        return std::unexpected( QString( "Could not write schedule to file: %1 (%2)" ).arg( fileName, file.errorString() ) );
    }

    file.close();
    return fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventTimeline* RicExportScheduleFeature::timelineToExport()
{
    RimWellEventTimeline* timeline = caf::SelectionManager::instance()->selectedItemOfType<RimWellEventTimeline>();
    if ( !timeline )
    {
        RimWellPathCollection* wellPathCollection = caf::SelectionManager::instance()->selectedItemOfType<RimWellPathCollection>();
        if ( !wellPathCollection )
        {
            // Invoked without a related selection, e.g. from the File -> Export menu.
            if ( auto* project = RimProject::current(); project && project->activeOilField() )
            {
                wellPathCollection = project->activeOilField()->wellPathCollection();
            }
        }

        if ( wellPathCollection ) timeline = wellPathCollection->eventTimeline();
    }

    if ( timeline && timeline->eventCount() > 0 ) return timeline;

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicExportScheduleFeature::defaultEclipseCase()
{
    if ( auto* eclipseView = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeReservoirView() ) )
    {
        if ( eclipseView->eclipseCase() ) return eclipseView->eclipseCase();
    }

    if ( auto* project = RimProject::current() )
    {
        auto eclipseCases = project->eclipseCases();
        if ( !eclipseCases.empty() ) return eclipseCases.front();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportScheduleFeature::isCommandEnabled() const
{
    return timelineToExport() != nullptr && defaultEclipseCase() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportScheduleFeature::onActionTriggered( bool isChecked )
{
    QWidget* parentWidget = Riu3DMainWindowTools::mainWindowWidget();

    RimWellEventTimeline* timeline = timelineToExport();
    if ( !timeline )
    {
        RiuMessageDialog::showError( parentWidget, featureTitle, "The well event timeline has no events to export." );
        return;
    }

    RimEclipseCase* eclipseCase = defaultEclipseCase();
    if ( !eclipseCase )
    {
        RiuMessageDialog::showError( parentWidget, featureTitle, "Schedule export requires a loaded Eclipse case." );
        return;
    }

    RiaApplication* app         = RiaApplication::instance();
    QString         defaultDir  = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( exportDialogName );
    QString         defaultFile = defaultDir + "/schedule.SCH";

    QString fileName = RiuFileDialogTools::getSaveFileName( parentWidget,
                                                            featureTitle,
                                                            defaultFile,
                                                            "Eclipse Schedule Files (*.SCH);;Include Files (*.inc);;All Files (*.*)" );
    if ( fileName.isEmpty() ) return;

    app->setLastUsedDialogDirectory( exportDialogName, QFileInfo( fileName ).absolutePath() );

    auto exportedFile = exportScheduleToFile( *timeline, *eclipseCase, fileName );
    if ( !exportedFile )
    {
        RiaLogging::error( std::format( "Schedule export failed: {}", exportedFile.error() ) );
        RiuMessageDialog::showError( parentWidget, featureTitle, exportedFile.error() );
        return;
    }

    RiaLogging::info( std::format( "Exported schedule based on {} well event(s) to {}", timeline->eventCount(), *exportedFile ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportScheduleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Schedule from Events" );
    actionToSetup->setIcon( QIcon( ":/Save.svg" ) );
}
