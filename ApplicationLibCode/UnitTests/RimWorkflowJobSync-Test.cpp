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
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RimWorkflow.h"
#include "RimWorkflowDefinitionTools.h"
#include "RimWorkflowJob.h"
#include "RimWorkflowTaskInput.h"
#include "RimWorkflowValidationTools.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

namespace
{
RimWorkflowDefinition loadDataDefinition()
{
    QFile file( QString( "%1/RimWorkflowDefinition/load_data.json" ).arg( TEST_DATA_DIR ) );
    EXPECT_TRUE( file.open( QIODevice::ReadOnly ) );
    auto definition = RimWorkflowDefinitionTools::fromJson( QJsonDocument::fromJson( file.readAll() ).object() );
    EXPECT_TRUE( definition.has_value() );
    return definition.value_or( RimWorkflowDefinition() );
}

QJsonArray graphTasks( const RimWorkflowDefinition& definition )
{
    return RimWorkflowDefinitionTools::graphFromDefinition( definition, true ).value( "tasks" ).toArray();
}

QString pathValue( const RimWorkflowJob& job, const QString& taskName )
{
    return job.inputValues().value( taskName ).toObject().value( "path" ).toString();
}
} // namespace

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowJobSync, oneTaskInputPerTask )
{
    const auto     definition = loadDataDefinition();
    RimWorkflowJob job;
    job.syncTaskInputs( graphTasks( definition ) );

    ASSERT_EQ( 4u, job.taskInputs().size() );
    EXPECT_EQ( "connect_to_resinsight", job.taskInputs()[0]->taskName() );
    EXPECT_EQ( 0u, job.taskInputs()[0]->count() );
    EXPECT_EQ( "resinsight.load_model", job.taskInput( "load_model" )->taskType() );
    EXPECT_EQ( "/path/to/grid.roff", pathValue( job, "load_model" ) );

    // Tasks without config fields are not written
    EXPECT_FALSE( job.inputValues().contains( "connect_to_resinsight" ) );
    EXPECT_EQ( QJsonArray( { "/path/to/property.roffasc" } ),
               job.literalInputValues().value( "import_grid_property" ).toObject().value( "paths" ).toArray() );

    QTemporaryDir directory;
    const QString path = job.writeInputYaml( directory.filePath( "input.yaml" ) );
    QFile         file( path );
    ASSERT_TRUE( file.open( QIODevice::ReadOnly ) );
    EXPECT_EQ( job.inputValues(), QJsonDocument::fromJson( file.readAll() ).object() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowJobSync, valuesFollowRenamesAndRemovals )
{
    auto           definition = loadDataDefinition();
    RimWorkflowJob job;
    job.syncTaskInputs( graphTasks( definition ) );
    RimWorkflowTaskInput* loadModel = job.taskInput( "load_model" );

    // The binding object and its value are kept when the task is renamed
    auto renamed = RimWorkflowDefinitionTools::renameTask( definition, "load_model", "load_grid" );
    ASSERT_TRUE( renamed.has_value() );
    renamed->inputs["load_grid"] = QJsonObject();
    job.syncTaskInputs( graphTasks( renamed.value() ), { { "load_model", "load_grid" } } );
    EXPECT_EQ( loadModel, job.taskInput( "load_grid" ) );
    EXPECT_EQ( "/path/to/grid.roff", pathValue( job, "load_grid" ) );

    // Removing the task keeps its values aside, so undo brings them back
    auto removed = RimWorkflowDefinitionTools::removeTask( renamed.value(), "load_grid" );
    ASSERT_TRUE( removed.has_value() );
    job.syncTaskInputs( graphTasks( removed.value() ) );
    EXPECT_EQ( nullptr, job.taskInput( "load_grid" ) );

    job.syncTaskInputs( graphTasks( renamed.value() ) );
    EXPECT_EQ( "/path/to/grid.roff", pathValue( job, "load_grid" ) );

    // And through a rename back
    job.syncTaskInputs( graphTasks( removed.value() ) );
    auto restored = renamed.value();
    ASSERT_TRUE( restored.findNode( "load_grid" ) );
    restored.findNode( "load_grid" )->name = "load_model";
    restored.inputs.remove( "load_grid" );
    for ( auto& edge : restored.edges )
    {
        if ( edge.from == "load_grid" ) edge.from = "load_model";
    }
    job.syncTaskInputs( graphTasks( removed.value() ), { { "load_grid", "load_model" } } );
    job.syncTaskInputs( graphTasks( restored ) );
    EXPECT_EQ( "/path/to/grid.roff", pathValue( job, "load_model" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowEditing, undoRedoAndDirtyState )
{
    RimWorkflow workflow;
    workflow.setUnsavedDefinition( loadDataDefinition() );
    EXPECT_TRUE( workflow.isEditable() );
    EXPECT_TRUE( workflow.isDirty() );
    EXPECT_FALSE( workflow.canUndo() );
    ASSERT_EQ( 1u, workflow.jobs().size() );
    RimWorkflowJob* job = workflow.jobs().front();
    EXPECT_EQ( "/path/to/grid.roff", pathValue( *job, "load_model" ) );

    const int revision = workflow.revision();
    auto      result   = workflow.applyEdit( RimWorkflowDefinitionTools::renameTask( workflow.definition(), "load_model", "load_grid" ),
                                             { { "load_model", "load_grid" } } );
    ASSERT_TRUE( result.has_value() );
    EXPECT_GT( workflow.revision(), revision );
    EXPECT_TRUE( workflow.canUndo() );
    EXPECT_EQ( "/path/to/grid.roff", pathValue( *job, "load_grid" ) );
    EXPECT_TRUE( workflow.uiName().endsWith( " *" ) );

    workflow.undo();
    EXPECT_TRUE( workflow.definition().findNode( "load_model" ) );
    EXPECT_EQ( "/path/to/grid.roff", pathValue( *job, "load_model" ) );
    EXPECT_TRUE( workflow.canRedo() );

    workflow.redo();
    EXPECT_TRUE( workflow.definition().findNode( "load_grid" ) );
    EXPECT_FALSE( workflow.canRedo() );

    // Failed edits change nothing
    const auto before = workflow.definition();
    EXPECT_FALSE( workflow.applyEdit( RimWorkflowDefinitionTools::renameTask( before, "load_grid", "bad name" ) ).has_value() );
    EXPECT_EQ( before, workflow.definition() );

    // Read-only workflows cannot be edited
    RimWorkflow readOnly;
    auto        readOnlyDefinition = loadDataDefinition();
    readOnlyDefinition.editable    = false;
    readOnly.loadFromRegistered( QJsonObject{ { "id", "test.workflow" },
                                              { "name", "workflow" },
                                              { "definition", RimWorkflowDefinitionTools::toJson( readOnlyDefinition ) } } );
    EXPECT_FALSE( readOnly.isEditable() );
    EXPECT_FALSE( readOnly.isDirty() );
    EXPECT_FALSE( readOnly.applyEdit( RimWorkflowDefinitionTools::setResultTask( readOnly.definition(), "" ) ).has_value() );
    auto runSource = readOnly.prepareRunSource();
    ASSERT_TRUE( runSource.has_value() );
    EXPECT_EQ( QStringList( { "run", "--registered", "test.workflow" } ), runSource->baseArgs );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowEditing, editabilityOfInstalledAndDuplicatedWorkflows )
{
    // An installed workflow is read-only even when its definition is editable,
    // but keeps the editable flag so it can be duplicated
    RimWorkflow installed;
    installed.loadFromRegistered( QJsonObject{ { "id", "test.workflow" },
                                               { "name", "workflow" },
                                               { "definition", RimWorkflowDefinitionTools::toJson( loadDataDefinition() ) } } );
    EXPECT_FALSE( installed.isEditable() );
    EXPECT_TRUE( installed.definition().editable );
    EXPECT_FALSE( installed.definition().readOnlyReasons.isEmpty() );

    RimWorkflow duplicate;
    duplicate.setUnsavedDefinition( installed.definition() );
    EXPECT_TRUE( duplicate.isEditable() );
    EXPECT_TRUE( duplicate.definition().readOnlyReasons.isEmpty() );

    // Unsupported structure (e.g. nested workflows) keeps a duplicate read-only
    auto nested                  = loadDataDefinition();
    nested.nodes.front().extra   = QJsonObject{ { "workflow", "other" } };
    const QStringList structural = RimWorkflowDefinitionTools::structuralReadOnlyReasons( nested );
    ASSERT_EQ( 1, structural.size() );
    EXPECT_TRUE( structural.front().contains( "workflow" ) );

    RimWorkflow nestedWorkflow;
    nestedWorkflow.setUnsavedDefinition( nested );
    EXPECT_FALSE( nestedWorkflow.isEditable() );
    EXPECT_EQ( structural, nestedWorkflow.definition().readOnlyReasons );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowValidationTools, issuesFromHelperResult )
{
    const QStringList nodes{ "load_model", "load_model_2", "import" };

    const QJsonObject describeError{ { "message", "Missing config" },
                                     { "task", "load_model" },
                                     { "issues", QJsonArray{ QJsonObject{ { "field", "path" }, { "code", "missing" } } } } };
    auto              result =
        RimWorkflowValidationTools::issuesFromHelperResult( QJsonObject{ { "status", "ok" }, { "describe_error", describeError } }, nodes );
    EXPECT_FALSE( result.describeSucceeded );
    ASSERT_EQ( 1u, result.issues.size() );
    EXPECT_EQ( "load_model", result.issues[0].task );
    EXPECT_EQ( "path", result.issues[0].field );
    EXPECT_EQ( RimWorkflowIssue::Severity::Error, result.issues[0].severity );

    // The task is found from a quoted name in the message, preferring the longest match
    result =
        RimWorkflowValidationTools::issuesFromHelperResult( QJsonObject{ { "status", "invalid" },
                                                                         { "error",
                                                                           QJsonObject{ { "message", "Task 'load_model_2' has a bad input" } } } },
                                                            nodes );
    ASSERT_EQ( 1u, result.issues.size() );
    EXPECT_EQ( "load_model_2", result.issues[0].task );

    // Unknown tasks go to the workflow
    result = RimWorkflowValidationTools::issuesFromHelperResult( QJsonObject{ { "status", "invalid" },
                                                                              { "error",
                                                                                QJsonObject{ { "message", "Broken" }, { "task", "other" } } } },
                                                                 nodes );
    ASSERT_EQ( 1u, result.issues.size() );
    EXPECT_TRUE( result.issues[0].task.isEmpty() );

    // Missing config values are warnings
    const QJsonObject describe{
        { "tasks", QJsonArray{ QJsonObject{ { "name", "import" }, { "missing_config_fields", QJsonArray{ "paths" } } } } } };
    result = RimWorkflowValidationTools::issuesFromHelperResult( QJsonObject{ { "status", "ok" }, { "describe", describe } }, nodes );
    EXPECT_TRUE( result.describeSucceeded );
    ASSERT_EQ( 1u, result.issues.size() );
    EXPECT_EQ( RimWorkflowIssue::Severity::Warning, result.issues[0].severity );
    EXPECT_EQ( "paths", result.issues[0].field );
}
