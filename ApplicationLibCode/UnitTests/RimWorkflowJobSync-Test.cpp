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
#include "RimWorkflowFieldBinding.h"
#include "RimWorkflowHelperProcess.h"
#include "RimWorkflowJob.h"
#include "RimWorkflowSchemaTools.h"
#include "RimWorkflowTaskInput.h"
#include "RimWorkflowValidationTools.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmUiCommandSystemProxy.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
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
    auto definition = RimWorkflowDefinitionTools::fromJson( RimWorkflowSchemaTools::parseWithPropertyOrder( file.readAll() ).object() );
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

void readThroughProjectFile( const RimWorkflowTaskInput& source, RimWorkflowTaskInput& target )
{
    for ( size_t i = 0; i < source.items().size(); ++i )
    {
        target.items()[i]->readObjectFromXmlString( source.items()[i]->writeObjectToXmlString(), caf::PdmDefaultObjectFactory::instance() );
        target.items()[i]->initAfterReadRecursively();
    }
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
TEST( RimWorkflowJobSync, duplicatedTaskGetsJobValues )
{
    const auto     definition = loadDataDefinition();
    RimWorkflowJob job;
    job.syncTaskInputs( graphTasks( definition ) );

    // A job value that differs from input.yaml
    QMap<QString, QJsonValue> detached;
    RimWorkflowTaskInput*     loadModel = job.taskInput( "load_model" );
    QJsonArray                fields    = graphTasks( definition )[1].toObject().value( "config_fields" ).toArray();
    ASSERT_EQ( "load_model", graphTasks( definition )[1].toObject().value( "name" ).toString() );
    for ( qsizetype i = 0; i < fields.size(); ++i )
    {
        QJsonObject field = fields[i].toObject();
        if ( field.value( "name" ).toString() == "path" ) field["default"] = "/edited/grid.roff";
        fields[i] = field;
    }
    loadModel->syncFromSchema( QJsonArray{}, detached );
    detached.clear();
    loadModel->syncFromSchema( fields, detached );
    ASSERT_EQ( "/edited/grid.roff", pathValue( job, "load_model" ) );

    QString    name;
    const auto duplicated = RimWorkflowDefinitionTools::duplicateTask( definition, "load_model", &name );
    ASSERT_TRUE( duplicated.has_value() );
    job.syncTaskInputs( graphTasks( duplicated.value() ) );
    EXPECT_EQ( "/path/to/grid.roff", pathValue( job, name ) );

    job.copyTaskInputValues( "load_model", name );
    EXPECT_EQ( "/edited/grid.roff", pathValue( job, name ) );
    EXPECT_EQ( job.inputValues().value( "load_model" ), job.inputValues().value( name ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowJobSync, untypedFieldValuesAreLiterals )
{
    const QString explicitNull = RimWorkflowSchemaTools::explicitNullKey();

    // Text entered before the fields were untyped is read as a literal
    RimWorkflowTaskInput input;
    input.buildFromSchema( QJsonArray{ QJsonObject{ { "name", "flag" }, { "type", "string" }, { "default", "False" } },
                                       QJsonObject{ { "name", "nothing" }, { "type", "string" }, { "default", "None" } },
                                       QJsonObject{ { "name", "text" }, { "type", "string" }, { "default", "False" } } } );
    const QJsonArray          untyped{ QJsonObject{ { "name", "flag" }, { "type", "any" } },
                              QJsonObject{ { "name", "nothing" }, { "type", "any" } },
                              QJsonObject{ { "name", "text" }, { "type", "string" } } };
    QMap<QString, QJsonValue> detached;
    input.syncFromSchema( untyped, detached );

    const QJsonObject expected{ { "flag", false }, { "nothing", QJsonValue::Null }, { "text", "False" } };
    EXPECT_EQ( expected, input.jsonValues() );

    // Values, including an entered None, survive a sync
    input.syncFromSchema( untyped, detached );
    EXPECT_EQ( expected, input.jsonValues() );

    // ... and a removal and return of the fields
    input.syncFromSchema( QJsonArray{}, detached );
    EXPECT_TRUE( input.jsonValues().isEmpty() );
    input.syncFromSchema( untyped, detached );
    EXPECT_EQ( expected, input.jsonValues() );

    // New bindings keep the type of typed values; a model default of None is not a value
    RimWorkflowTaskInput typed;
    typed.buildFromSchema(
        QJsonArray{ QJsonObject{ { "name", "quoted" }, { "type", "any" }, { "default", "true" } },
                    QJsonObject{ { "name", "number" }, { "type", "any" }, { "default", "1" } },
                    QJsonObject{ { "name", "flag" }, { "type", "any" }, { "default", true } },
                    QJsonObject{ { "name", "model_none" }, { "type", "any" }, { "default", QJsonValue::Null } },
                    QJsonObject{ { "name", "entered_none" }, { "type", "any" }, { "default", QJsonValue::Null }, { explicitNull, true } } } );
    const QJsonObject expectedTyped{ { "quoted", "true" }, { "number", "1" }, { "flag", true }, { "entered_none", QJsonValue::Null } };
    EXPECT_EQ( expectedTyped, typed.jsonValues() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowJobSync, inputYamlNullIsExplicit )
{
    const QJsonObject inputSchema{ { "properties", QJsonObject{ { "value", QJsonObject{ { "default", QJsonValue::Null } } } } } };
    const QString     explicitNull = RimWorkflowSchemaTools::explicitNullKey();

    EXPECT_FALSE( RimWorkflowSchemaTools::configFieldSchema( "value", inputSchema, {}, {} ).value( explicitNull ).toBool() );
    EXPECT_TRUE( RimWorkflowSchemaTools::configFieldSchema( "value", inputSchema, {}, QJsonObject{ { "value", QJsonValue::Null } } )
                     .value( explicitNull )
                     .toBool() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowJobSync, listFieldsAreEditedAsLists )
{
    const QJsonObject
        inputSchema{ { "$defs", QJsonObject{ { "Item", QJsonObject{ { "type", "object" } } } } },
                     { "properties",
                       QJsonObject{ { "ints", QJsonObject{ { "type", "array" }, { "items", QJsonObject{ { "type", "integer" } } } } },
                                    { "numbers",
                                      QJsonObject{
                                          { "anyOf",
                                            QJsonArray{ QJsonObject{ { "type", "array" }, { "items", QJsonObject{ { "type", "number" } } } },
                                                        QJsonObject{ { "type", "null" } } } } } },
                                    { "names", QJsonObject{ { "type", "array" }, { "items", QJsonObject{ { "type", "string" } } } } },
                                    { "items", QJsonObject{ { "type", "array" }, { "items", QJsonObject{ { "$ref", "#/$defs/Item" } } } } } } } };

    auto fieldSchema = [&]( const QString& name ) { return RimWorkflowSchemaTools::configFieldSchema( name, inputSchema, {}, {} ); };
    EXPECT_EQ( "integer", fieldSchema( "ints" ).value( "item_type" ).toString() );
    EXPECT_EQ( "number", fieldSchema( "numbers" ).value( "item_type" ).toString() );
    EXPECT_EQ( "string", fieldSchema( "names" ).value( "item_type" ).toString() );
    EXPECT_FALSE( fieldSchema( "items" ).contains( "item_type" ) );

    // Values in the old JSON text field are moved to the list when the field becomes a typed list
    RimWorkflowTaskInput input;
    input.buildFromSchema(
        QJsonArray{ QJsonObject{ { "name", "ints" }, { "type", "array" }, { "default", QJsonArray{ 1, 2, 3 } } },
                    QJsonObject{ { "name", "items" }, { "type", "array" }, { "default", QJsonArray{ QJsonObject{ { "a", 1 } } } } } } );
    const QJsonObject expected{ { "ints", QJsonArray{ 1, 2, 3 } }, { "items", QJsonArray{ QJsonObject{ { "a", 1 } } } } };
    EXPECT_EQ( expected, input.jsonValues() );

    QJsonObject typedInts = fieldSchema( "ints" );
    typedInts.remove( "default" );
    QJsonObject untypedItems = fieldSchema( "items" );
    untypedItems.remove( "default" );
    QMap<QString, QJsonValue> detached;
    input.syncFromSchema( QJsonArray{ typedInts, untypedItems }, detached );
    EXPECT_EQ( expected, input.jsonValues() );
    ASSERT_EQ( 2u, input.items().size() );
    EXPECT_EQ( "IntValues", input.items()[0]->valueField()->keyword() );
    EXPECT_EQ( "Value", input.items()[1]->valueField()->keyword() );
    EXPECT_EQ( "[1,2,3]", input.items()[0]->displayValue() );

    // Number and string lists keep their values
    RimWorkflowTaskInput lists;
    QJsonObject          numbers = fieldSchema( "numbers" );
    numbers["default"]           = QJsonArray{ 1.5, -2.0 };
    QJsonObject names            = fieldSchema( "names" );
    names["default"]             = QJsonArray{ "a", "b c" };
    lists.buildFromSchema( QJsonArray{ numbers, names } );
    const QJsonObject expectedLists{ { "numbers", QJsonArray{ 1.5, -2.0 } }, { "names", QJsonArray{ "a", "b c" } } };
    EXPECT_EQ( expectedLists, lists.jsonValues() );
    lists.syncFromSchema( QJsonArray{ numbers, names }, detached );
    EXPECT_EQ( expectedLists, lists.jsonValues() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowJobSync, listValuesAreKeptExactly )
{
    const QJsonObject inputSchema{
        { "properties",
          QJsonObject{ { "ints", QJsonObject{ { "type", "array" }, { "items", QJsonObject{ { "type", "integer" } } } } },
                       { "names", QJsonObject{ { "type", "array" }, { "items", QJsonObject{ { "type", "string" } } } } },
                       { "optional_ints",
                         QJsonObject{ { "type", "array" },
                                      { "items",
                                        QJsonObject{ { "anyOf",
                                                       QJsonArray{ QJsonObject{ { "type", "integer" } },
                                                                   QJsonObject{ { "type", "null" } } } } } } } } } } };
    auto fieldSchema = [&]( const QString& name, const QJsonArray& value )
    {
        QJsonObject schema = RimWorkflowSchemaTools::configFieldSchema( name, inputSchema, {}, {} );
        schema["default"]  = value;
        return schema;
    };

    // The list editors cannot hold null items
    EXPECT_FALSE( fieldSchema( "optional_ints", {} ).contains( "item_type" ) );

    // Values the list editors cannot hold exactly are edited as JSON text
    const QJsonArray exactFields{ fieldSchema( "ints", QJsonArray{ 1, 2 } ), fieldSchema( "names", QJsonArray{ "a", "b c" } ) };
    const QJsonArray textFields{ fieldSchema( "ints", QJsonArray{ 2147483648.0, 1.5 } ), fieldSchema( "names", QJsonArray{ "", " ", "a" } ) };
    for ( const QJsonArray& fields : { exactFields, textFields } )
    {
        RimWorkflowTaskInput input;
        input.buildFromSchema( fields );
        const QJsonObject expected{ { "ints", fields[0].toObject().value( "default" ) }, { "names", fields[1].toObject().value( "default" ) } };
        EXPECT_EQ( expected, input.jsonValues() );
        EXPECT_EQ( fields == exactFields ? "IntValues" : "Value", input.items()[0]->valueField()->keyword() );
        EXPECT_EQ( fields == exactFields ? "StringValues" : "Value", input.items()[1]->valueField()->keyword() );

        // The values survive the project file format
        RimWorkflowTaskInput read;
        read.buildFromSchema( fields );
        readThroughProjectFile( input, read );
        EXPECT_EQ( expected, read.jsonValues() );
    }

    // An edited list is stored
    RimWorkflowTaskInput input;
    input.buildFromSchema( exactFields );
    caf::PdmUiCommandSystemProxy::instance()->setUiValueToField( input.items()[1]->valueField()->uiCapability(),
                                                                 QVariantList{ QString( "d e" ), QString( "f" ) } );
    RimWorkflowTaskInput read;
    read.buildFromSchema( exactFields );
    readThroughProjectFile( input, read );
    EXPECT_EQ( QJsonArray( { "d e", "f" } ), read.jsonValues().value( "names" ) );

    // ... and copied to a duplicated task
    RimWorkflowTaskInput copy;
    copy.buildFromSchema( exactFields );
    copy.copyValuesFrom( input );
    EXPECT_EQ( QJsonArray( { "d e", "f" } ), copy.jsonValues().value( "names" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowEditing, copyPythonModules )
{
    QTemporaryDir directory;
    ASSERT_TRUE( directory.isValid() );
    const QDir root( directory.path() );
    auto       write = [&root]( const QString& path, const QByteArray& content )
    {
        QDir().mkpath( QFileInfo( root.absoluteFilePath( path ) ).absolutePath() );
        QFile file( root.absoluteFilePath( path ) );
        return file.open( QIODevice::WriteOnly ) && file.write( content ) == content.size();
    };
    ASSERT_TRUE( write( "source/tasks.py", "new" ) );
    ASSERT_TRUE( write( "source/workflow.yaml", "" ) );
    ASSERT_TRUE( write( "source/helpers/__init__.py", "" ) );
    ASSERT_TRUE( write( "source/helpers/math.py", "" ) );
    ASSERT_TRUE( write( "source/helpers/__pycache__/math.pyc", "" ) );
    ASSERT_TRUE( write( "source/data/values.py", "" ) );
    ASSERT_TRUE( write( "target/tasks.py", "existing" ) );

    ASSERT_TRUE( RimWorkflow::copyPythonModules( root.absoluteFilePath( "source" ), root.absoluteFilePath( "target" ) ).has_value() );
    EXPECT_TRUE( root.exists( "target/helpers/__init__.py" ) );
    EXPECT_TRUE( root.exists( "target/helpers/math.py" ) );
    EXPECT_FALSE( root.exists( "target/helpers/__pycache__" ) );
    EXPECT_FALSE( root.exists( "target/data" ) );
    EXPECT_FALSE( root.exists( "target/workflow.yaml" ) );

    QFile existing( root.absoluteFilePath( "target/tasks.py" ) );
    ASSERT_TRUE( existing.open( QIODevice::ReadOnly ) );
    EXPECT_EQ( "existing", existing.readAll() );

    // A target that cannot be created is reported
    ASSERT_TRUE( write( "blocked", "" ) );
    EXPECT_FALSE( RimWorkflow::copyPythonModules( root.absoluteFilePath( "source" ), root.absoluteFilePath( "blocked/target" ) ).has_value() );
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

    using Renames = QMap<QString, QString>;
    EXPECT_EQ( ( Renames{ { "load_grid", "load_model" } } ), workflow.undoRenames() );
    EXPECT_TRUE( workflow.redoRenames().isEmpty() );

    workflow.undo();
    EXPECT_EQ( ( Renames{ { "load_model", "load_grid" } } ), workflow.redoRenames() );
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
    auto runSource = readOnly.prepareRunSource( {} );
    ASSERT_TRUE( runSource.has_value() );
    EXPECT_EQ( QStringList( { "run", "--registered", "test.workflow" } ), runSource->baseArgs );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowEditing, hiddenAndCollapsedTasksFollowRenamesAndRemovals )
{
    RimWorkflow workflow;
    workflow.setUnsavedDefinition( loadDataDefinition() );
    workflow.setTaskHidden( "connect_to_resinsight", true );
    workflow.setTaskHidden( "load_model", true );
    workflow.setTaskCollapsed( "load_model", true );
    EXPECT_FALSE( workflow.canUndo() );

    ASSERT_TRUE( workflow
                     .applyEdit( RimWorkflowDefinitionTools::renameTask( workflow.definition(), "load_model", "load_grid" ),
                                 { { "load_model", "load_grid" } } )
                     .has_value() );
    EXPECT_EQ( ( QStringList{ "connect_to_resinsight", "load_grid" } ), workflow.hiddenTasks() );
    EXPECT_EQ( ( QStringList{ "load_grid" } ), workflow.collapsedTasks() );

    workflow.undo();
    EXPECT_EQ( ( QStringList{ "connect_to_resinsight", "load_model" } ), workflow.hiddenTasks() );
    EXPECT_TRUE( workflow.isTaskCollapsed( "load_model" ) );

    ASSERT_TRUE( workflow.applyEdit( RimWorkflowDefinitionTools::removeTask( workflow.definition(), "load_model" ) ).has_value() );
    EXPECT_EQ( ( QStringList{ "connect_to_resinsight" } ), workflow.hiddenTasks() );
    EXPECT_TRUE( workflow.collapsedTasks().isEmpty() );

    workflow.setTaskHidden( "connect_to_resinsight", false );
    EXPECT_TRUE( workflow.hiddenTasks().isEmpty() );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowHelperProcess, invalidResultKeepsTaskAndField )
{
    const QByteArray invalid = R"({"status": "invalid", "error": {"message": "Bad value", "task": "load_model", "field": "path"}})";

    const auto result = RimWorkflowHelperProcess::invalidResult( invalid + "\n" );
    ASSERT_TRUE( result.has_value() );
    const auto validation = RimWorkflowValidationTools::issuesFromHelperResult( *result, { "load_model" } );
    EXPECT_FALSE( validation.describeSucceeded );
    ASSERT_EQ( 1u, validation.issues.size() );
    EXPECT_EQ( "load_model", validation.issues[0].task );
    EXPECT_EQ( "path", validation.issues[0].field );

    // parseOutput() reduces the same output to a message
    const auto parsed = RimWorkflowHelperProcess::parseOutput( 2, invalid, {} );
    ASSERT_FALSE( parsed.has_value() );
    EXPECT_TRUE( parsed.error().startsWith( "Bad value" ) );

    EXPECT_FALSE( RimWorkflowHelperProcess::invalidResult( R"({"status": "ok"})" ).has_value() );
    EXPECT_FALSE( RimWorkflowHelperProcess::invalidResult( "Traceback ..." ).has_value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowValidationTools, issuesWithJobValues )
{
    const QJsonObject missingCase{ { "task", "select" },
                                   { "field", "case" },
                                   { "severity", "warning" },
                                   { "message", RimWorkflowValidationTools::missingValueMessage( "case" ) } };
    const QJsonObject otherIssue{ { "task", "select" }, { "field", "" }, { "severity", "error" }, { "message", "Broken" } };

    // A value in the job removes the stale warning from describe
    QJsonArray issues = RimWorkflowValidationTools::issuesWithJobValues( "select", { missingCase, otherIssue }, { { "case", true } } );
    ASSERT_EQ( 1, issues.size() );
    EXPECT_EQ( "Broken", issues[0].toObject().value( "message" ).toString() );

    // A missing value in the job gives one warning, even if describe also reported it
    issues = RimWorkflowValidationTools::issuesWithJobValues( "select", { missingCase }, { { "case", false } } );
    ASSERT_EQ( 1, issues.size() );
    EXPECT_EQ( "case", issues[0].toObject().value( "field" ).toString() );
    EXPECT_EQ( "warning", issues[0].toObject().value( "severity" ).toString() );

    // Fields the job does not bind keep the warnings from describe
    issues = RimWorkflowValidationTools::issuesWithJobValues( "select", { missingCase }, {} );
    EXPECT_EQ( 1, issues.size() );
}
