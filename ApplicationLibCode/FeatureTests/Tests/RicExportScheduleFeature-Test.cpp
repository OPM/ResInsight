/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "gtest/gtest.h"

#include "RiaFeatureTestModelBuilder.h"

#include "CompletionExportCommands/RicExportScheduleFeature.h"

#include "RimKeywordEvent.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellEventInsertDate.h"
#include "RimWellEventTimeline.h"
#include "RimWellPathCollection.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
/// Curated test for RicExportScheduleFeature, which writes the schedule generated from the well
/// event timeline to file. The interactive part (the save file dialog) is skipped by calling
/// exportScheduleToFile() directly; the feature is only queried for whether it accepts the
/// selection.
//--------------------------------------------------------------------------------------------------
class RicExportScheduleFeatureTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }

    static RimWellEventTimeline* timelineWithEvents()
    {
        RimOilField* oilField = RimProject::current()->activeOilField();
        if ( !oilField || !oilField->wellPathCollection() ) return nullptr;

        RimWellEventTimeline* timeline = oilField->wellPathCollection()->eventTimeline();
        if ( !timeline ) return nullptr;

        const QDateTime firstDate( QDate( 2024, 1, 1 ), QTime( 0, 0, 0 ) );
        auto*           keywordEvent = timeline->addKeywordEvent( firstDate, "RPTRST" );
        keywordEvent->addIntItem( "BASIC", 2 );

        auto* insertDate = timeline->addInsertDateEvent( QDateTime( QDate( 2024, 7, 1 ), QTime( 0, 0, 0 ) ) );
        insertDate->setComment( "Mid-year summary report" );

        return timeline;
    }
};

TEST_F( RicExportScheduleFeatureTest, ExportWritesScheduleForTimelineEvents )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::combinedModel();
    ASSERT_TRUE( model.eclipseCase != nullptr );

    RimWellEventTimeline* timeline = timelineWithEvents();
    ASSERT_TRUE( timeline != nullptr );

    caf::SelectionManager::instance()->setSelectedItem( timeline );

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicExportScheduleFeature" );
    ASSERT_TRUE( feature != nullptr );
    EXPECT_TRUE( feature->canFeatureBeExecuted() );

    // Without a related selection (e.g. from the File -> Export menu) the timeline of the
    // project's well path collection is used.
    caf::SelectionManager::instance()->clearAll();
    EXPECT_TRUE( feature->canFeatureBeExecuted() );

    QTemporaryDir temporaryDir;
    ASSERT_TRUE( temporaryDir.isValid() );
    const QString fileName = temporaryDir.filePath( "schedule.SCH" );

    auto exportedFile = RicExportScheduleFeature::exportScheduleToFile( *timeline, *model.eclipseCase, fileName );
    ASSERT_TRUE( exportedFile.has_value() ) << exportedFile.error().toStdString();
    EXPECT_EQ( fileName.toStdString(), exportedFile->toStdString() );

    QFile file( fileName );
    ASSERT_TRUE( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
    const QString content = QTextStream( &file ).readAll();

    // The first (earliest) date is written as a comment, the inserted date as a DATES keyword
    // followed by the comment of the insert-date event. Columns are aligned, so the records are
    // padded: "    1  'JUL'  2024 /".
    EXPECT_TRUE( content.contains( "-- Date: 1 JAN 2024" ) );
    EXPECT_TRUE( content.contains( "RPTRST" ) );
    EXPECT_TRUE( content.contains( "BASIC=2" ) );
    EXPECT_TRUE( content.contains( "DATES" ) );
    EXPECT_TRUE( content.contains( "'JUL'" ) );
    EXPECT_TRUE( content.contains( "-- Mid-year summary report" ) );
    // Aligned output adds a column-header comment per keyword.
    EXPECT_TRUE( content.contains( "--DAY" ) );
}

TEST_F( RicExportScheduleFeatureTest, ExportOfEmptyTimelineFails )
{
    FeatureTestModel model = RiaFeatureTestModelBuilder::combinedModel();
    ASSERT_TRUE( model.eclipseCase != nullptr );

    RimOilField* oilField = RimProject::current()->activeOilField();
    ASSERT_TRUE( oilField != nullptr && oilField->wellPathCollection() != nullptr );
    RimWellEventTimeline* timeline = oilField->wellPathCollection()->eventTimeline();
    ASSERT_TRUE( timeline != nullptr );

    // No events: the feature is not offered, and a direct export reports why.
    caf::SelectionManager::instance()->setSelectedItem( timeline );
    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicExportScheduleFeature" );
    ASSERT_TRUE( feature != nullptr );
    EXPECT_FALSE( feature->canFeatureBeExecuted() );

    QTemporaryDir temporaryDir;
    ASSERT_TRUE( temporaryDir.isValid() );

    auto exportedFile = RicExportScheduleFeature::exportScheduleToFile( *timeline, *model.eclipseCase, temporaryDir.filePath( "empty.SCH" ) );
    ASSERT_FALSE( exportedFile.has_value() );
    EXPECT_EQ( std::string( "No events found in timeline" ), exportedFile.error().toStdString() );
}
