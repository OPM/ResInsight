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
#include "RimWorkflowDefinitionTools.h"
#include "RimWorkflowPortCompatibility.h"
#include "RimWorkflowTaskCatalog.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

//
// Fixtures in TestData/RimWorkflowDefinition are produced by the Python helper (run from
// GrpcInterface/Python, with absolute paths replaced by `test_data/taskmaestro_workflows`):
//   python -m rips.taskmaestro_helper load rips/tests/test_data/taskmaestro_workflows/resinsight_completions --describe > completions.json
//   python -m rips.taskmaestro_helper load rips/tests/test_data/taskmaestro_workflows/resinsight_load_data --describe > load_data.json
//   python -m rips.taskmaestro_helper catalog > catalog.json
//
namespace
{
QJsonObject fixture( const QString& name )
{
    QFile file( QString( "%1/RimWorkflowDefinition/%2.json" ).arg( TEST_DATA_DIR ).arg( name ) );
    if ( !file.open( QIODevice::ReadOnly ) ) return {};
    return QJsonDocument::fromJson( file.readAll() ).object();
}

RimWorkflowDefinition definitionFixture( const QString& name )
{
    auto definition = RimWorkflowDefinitionTools::fromJson( fixture( name ) );
    EXPECT_TRUE( definition.has_value() );
    return definition.value_or( RimWorkflowDefinition() );
}

RimWorkflowTaskCatalog catalogFixture()
{
    return RimWorkflowTaskCatalog::fromJson( fixture( "catalog" ) );
}

QJsonObject taskByName( const QJsonObject& graph, const QString& name )
{
    for ( const QJsonValue& task : graph.value( "tasks" ).toArray() )
    {
        if ( task.toObject().value( "name" ).toString() == name ) return task.toObject();
    }
    return {};
}

RimWorkflowDefinition edited( const RimWorkflowDefinitionTools::EditResult& result )
{
    EXPECT_TRUE( result.has_value() ) << ( result ? "" : result.error().toStdString() );
    return result.value_or( RimWorkflowDefinition() );
}

bool hasEdge( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionEdge& edge )
{
    return std::find( definition.edges.begin(), definition.edges.end(), edge ) != definition.edges.end();
}

QStringList issueMessages( const std::vector<RimWorkflowIssue>& issues )
{
    QStringList messages;
    for ( const auto& issue : issues )
        messages.append( QString( "%1.%2: %3" ).arg( issue.task, issue.field, issue.message ) );
    return messages;
}
} // namespace

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinition, fromJson )
{
    const auto definition = definitionFixture( "completions" );
    EXPECT_EQ( "resinsight_completions", definition.name );
    EXPECT_TRUE( definition.editable );
    ASSERT_EQ( 7u, definition.nodes.size() );
    EXPECT_EQ( 13u, definition.edges.size() );
    EXPECT_TRUE( definition.resultTask.isEmpty() );

    const auto* addPerf = definition.findNode( "add_perf_1" );
    ASSERT_TRUE( addPerf );
    EXPECT_EQ( "resinsight.add_perforation", addPerf->taskId );
    EXPECT_EQ( QStringList( { "event_date", "start_md", "end_md" } ), addPerf->configFields );
    EXPECT_EQ( addPerf->configFields, addPerf->explicitConfigFields );
    EXPECT_FALSE( definition.describe.isEmpty() );

    EXPECT_FALSE( RimWorkflowDefinitionTools::fromJson( QJsonObject{ { "format", 2 }, { "nodes", QJsonArray() } } ).has_value() );
    EXPECT_FALSE( RimWorkflowDefinitionTools::fromJson( QJsonObject{ { "name", "x" } } ).has_value() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinition, jsonRoundTrip )
{
    for ( const QString& name : { "completions", "load_data" } )
    {
        const auto definition = definitionFixture( name );
        const auto roundTrip  = RimWorkflowDefinitionTools::fromJson( RimWorkflowDefinitionTools::toJson( definition ) );
        ASSERT_TRUE( roundTrip.has_value() );
        EXPECT_EQ( definition, roundTrip.value() ) << name.toStdString();
    }
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinition, normalizeKeepsLoadedExamples )
{
    for ( const QString& name : { "completions", "load_data" } )
    {
        const auto definition = definitionFixture( name );
        const auto normalized = RimWorkflowDefinitionTools::normalize( definition );
        for ( size_t i = 0; i < definition.nodes.size(); ++i )
        {
            EXPECT_EQ( definition.nodes[i].configFields, normalized.nodes[i].configFields ) << definition.nodes[i].name.toStdString();
        }
        EXPECT_EQ( normalized, RimWorkflowDefinitionTools::normalize( normalized ) );
        EXPECT_TRUE( RimWorkflowDefinitionTools::validate( definition ).empty() )
            << issueMessages( RimWorkflowDefinitionTools::validate( definition ) ).join( "\n" ).toStdString();
    }
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinition, graphFromDefinition )
{
    const auto definition = definitionFixture( "completions" );
    const auto graph      = RimWorkflowDefinitionTools::graphFromDefinition( definition, true );
    EXPECT_EQ( "resinsight_completions", graph.value( "name" ).toString() );
    EXPECT_TRUE( graph.value( "editable" ).toBool() );
    EXPECT_EQ( "export_completions", graph.value( "result_task" ).toString() );
    EXPECT_EQ( 7, graph.value( "tasks" ).toArray().size() );
    EXPECT_EQ( 13, graph.value( "edges" ).toArray().size() );
    EXPECT_TRUE( graph.value( "issues" ).toArray().isEmpty() );

    const QJsonObject exportTask = taskByName( graph, "export_completions" );
    EXPECT_TRUE( exportTask.value( "is_result" ).toBool() );
    EXPECT_EQ( 2, exportTask.value( "config_fields" ).toArray().size() );
    EXPECT_EQ( QJsonArray( { "export_file", "well_path_names" } ), exportTask.value( "outputs" ).toArray() );

    // The whole input port comes first, followed by the fields
    const QJsonArray ports = exportTask.value( "input_ports" ).toArray();
    ASSERT_EQ( 7, ports.size() );
    EXPECT_EQ( "", ports[0].toObject().value( "name" ).toString() );
    EXPECT_EQ( "resinsight", ports[1].toObject().value( "name" ).toString() );
    EXPECT_TRUE( ports[1].toObject().value( "wired" ).toBool() );

    // The connect task's opaque `value` is not a port of its own
    const QJsonObject connect = taskByName( graph, "connect_to_resinsight" );
    EXPECT_TRUE( connect.value( "outputs" ).toArray().isEmpty() );
    EXPECT_FALSE( connect.value( "accepts_input" ).toBool() );

    EXPECT_FALSE( RimWorkflowDefinitionTools::graphFromDefinition( definition, false ).value( "editable" ).toBool() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowPortCompatibility, fieldConnections )
{
    const auto definition = definitionFixture( "completions" );
    using RimWorkflowPortCompatibility::canConnect;

    EXPECT_TRUE( canConnect( definition, "connect_to_resinsight", "", "select_eclipse_case", "resinsight" ) );
    EXPECT_TRUE( canConnect( definition, "select_eclipse_case", "", "select_well_path_1", "grid_case" ) );

    // A GridCase is not a RipsInstance
    EXPECT_FALSE( canConnect( definition, "select_eclipse_case", "", "select_well_path_1", "resinsight" ) );
    // A perforation is not a well path
    EXPECT_FALSE( canConnect( definition, "add_perf_1", "", "add_perf_2", "well_path" ) );
    // Cycles and self-loops
    EXPECT_FALSE( canConnect( definition, "export_completions", "", "select_eclipse_case", "case" ) );
    EXPECT_FALSE( canConnect( definition, "add_perf_1", "", "add_perf_1", "well_path" ) );
    // Unknown ports and tasks
    EXPECT_FALSE( canConnect( definition, "select_eclipse_case", "", "select_well_path_1", "no_such_input" ) );
    EXPECT_FALSE( canConnect( definition, "no_such_task", "", "select_well_path_1", "grid_case" ) );
    // A task cannot take both its whole input and fields from other tasks
    EXPECT_FALSE( canConnect( definition, "select_well_path_1", "", "add_perf_1", "" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowPortCompatibility, wholeConnections )
{
    const auto catalog    = catalogFixture();
    auto       definition = RimWorkflowDefinitionTools::createEmpty( "test" );
    for ( const QString& taskId : { "resinsight.connect", "resinsight.load_model", "resinsight.close_project", "resinsight.exit" } )
    {
        definition.taskTypes.insert( taskId, catalog.taskType( taskId ) );
        definition.nodes.push_back( { .name = taskId.mid( 11 ), .taskId = taskId } );
    }
    using RimWorkflowPortCompatibility::canConnect;

    // RipsInstance -> close_project (RipsInstance) and exit (RipsInstance)
    EXPECT_TRUE( canConnect( definition, "connect", "", "close_project", "" ) );
    EXPECT_TRUE( canConnect( definition, "close_project", "", "exit", "" ) );
    // GridCase -> RipsInstance is not compatible
    EXPECT_FALSE( canConnect( definition, "load_model", "", "close_project", "" ) );
    // Whole RipsInstance into a field input
    EXPECT_TRUE( canConnect( definition, "connect", "", "load_model", "resinsight" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinitionEdit, addTaskWiresResInsight )
{
    const auto catalog    = catalogFixture();
    auto       definition = RimWorkflowDefinitionTools::createEmpty( "new_workflow" );

    QString added;
    definition = edited( RimWorkflowDefinitionTools::addTask( definition, catalog, "resinsight.load_model", &added ) );
    EXPECT_EQ( "load_model", added );
    ASSERT_EQ( 2u, definition.nodes.size() );
    EXPECT_EQ( "connect_to_resinsight", definition.nodes[0].name );
    EXPECT_TRUE( hasEdge( definition, { .from = "connect_to_resinsight", .to = "load_model", .input = "resinsight" } ) );
    EXPECT_EQ( QStringList( { "path" } ), definition.findNode( "load_model" )->configFields );
    EXPECT_TRUE( definition.inputs.contains( "load_model" ) );
    EXPECT_TRUE( definition.taskTypes.contains( "resinsight.load_model" ) );

    // A second instance gets a unique name and reuses the connect task
    definition = edited( RimWorkflowDefinitionTools::addTask( definition, catalog, "resinsight.load_model", &added ) );
    EXPECT_EQ( "load_model_2", added );
    EXPECT_EQ( 3u, definition.nodes.size() );
    EXPECT_TRUE( hasEdge( definition, { .from = "connect_to_resinsight", .to = "load_model_2", .input = "resinsight" } ) );

    // Two end tasks and no result task
    auto issues = RimWorkflowDefinitionTools::validate( definition );
    ASSERT_EQ( 1u, issues.size() ) << issueMessages( issues ).join( "\n" ).toStdString();
    EXPECT_TRUE( issues[0].task.isEmpty() );

    definition = edited( RimWorkflowDefinitionTools::setResultTask( definition, "load_model_2" ) );
    EXPECT_TRUE( RimWorkflowDefinitionTools::validate( definition ).empty() );

    // The exit task takes the whole RipsInstance
    definition = edited( RimWorkflowDefinitionTools::addTask( definition, catalog, "resinsight.exit", &added ) );
    EXPECT_EQ( "exit_resinsight", added );
    EXPECT_TRUE( hasEdge( definition, { .from = "connect_to_resinsight", .to = "exit_resinsight" } ) );
    EXPECT_TRUE( definition.findNode( "exit_resinsight" )->configFields.isEmpty() );

    EXPECT_FALSE( RimWorkflowDefinitionTools::addTask( definition, catalog, "no.such.task" ).has_value() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinitionEdit, connectionsUpdateConfigFields )
{
    auto                            definition = definitionFixture( "completions" );
    const RimWorkflowDefinitionEdge gridCase{ .from = "select_eclipse_case", .to = "select_well_path_1", .input = "grid_case" };

    // A disconnected required input that can be entered in ResInsight becomes a config field
    definition = edited( RimWorkflowDefinitionTools::disconnect( definition, gridCase ) );
    EXPECT_EQ( QStringList( { "well_path", "grid_case" } ), definition.findNode( "select_well_path_1" )->configFields );
    EXPECT_FALSE( RimWorkflowDefinitionTools::disconnect( definition, gridCase ).has_value() );

    definition = edited( RimWorkflowDefinitionTools::connect( definition, gridCase ) );
    EXPECT_EQ( QStringList( { "well_path" } ), definition.findNode( "select_well_path_1" )->configFields );

    // Connecting an input that is already connected replaces the connection
    definition = edited(
        RimWorkflowDefinitionTools::connect( definition, { .from = "add_perf_2", .to = "export_completions", .input = "perforation_1" } ) );
    EXPECT_FALSE( hasEdge( definition, { .from = "add_perf_1", .to = "export_completions", .input = "perforation_1" } ) );
    EXPECT_EQ( 13u, definition.edges.size() );

    // add_perf_1 is now an end task, and the perforation input of export is not configurable
    auto issues = RimWorkflowDefinitionTools::validate( definition );
    EXPECT_EQ( 1u, issues.size() ) << issueMessages( issues ).join( "\n" ).toStdString();

    definition = edited(
        RimWorkflowDefinitionTools::disconnect( definition, { .from = "add_perf_2", .to = "export_completions", .input = "perforation_1" } ) );
    issues           = RimWorkflowDefinitionTools::validate( definition );
    bool mustConnect = false;
    for ( const auto& issue : issues )
        mustConnect = mustConnect || ( issue.task == "export_completions" && issue.field == "perforation_1" );
    EXPECT_TRUE( mustConnect ) << issueMessages( issues ).join( "\n" ).toStdString();

    const auto graph = RimWorkflowDefinitionTools::graphFromDefinition( definition, true );
    EXPECT_FALSE( taskByName( graph, "export_completions" ).value( "issues" ).toArray().isEmpty() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinitionEdit, removeAndRename )
{
    auto definition = definitionFixture( "load_data" );
    EXPECT_EQ( "import_grid_property", definition.resultTask );

    definition.inputs["load_model"] = QJsonObject{ { "path", "/data/grid.roff" } };
    definition                      = edited( RimWorkflowDefinitionTools::renameTask( definition, "load_model", "load_grid" ) );
    EXPECT_TRUE( definition.findNode( "load_grid" ) );
    EXPECT_FALSE( definition.inputs.contains( "load_model" ) );
    EXPECT_EQ( "/data/grid.roff", definition.inputs["load_grid"].toObject().value( "path" ).toString() );
    EXPECT_TRUE( hasEdge( definition, { .from = "load_grid", .to = "import_grid_property", .input = "grid_case" } ) );

    definition = edited( RimWorkflowDefinitionTools::renameTask( definition, "import_grid_property", "import_props" ) );
    EXPECT_EQ( "import_props", definition.resultTask );

    EXPECT_FALSE( RimWorkflowDefinitionTools::renameTask( definition, "load_grid", "import_props" ).has_value() );
    EXPECT_FALSE( RimWorkflowDefinitionTools::renameTask( definition, "load_grid", "2bad" ).has_value() );
    EXPECT_FALSE( RimWorkflowDefinitionTools::renameTask( definition, "load_grid", "with space" ).has_value() );
    EXPECT_FALSE( RimWorkflowDefinitionTools::renameWorkflow( definition, "bad-name" ).has_value() );
    EXPECT_EQ( "renamed", edited( RimWorkflowDefinitionTools::renameWorkflow( definition, "renamed" ) ).name );

    definition = edited( RimWorkflowDefinitionTools::removeTask( definition, "import_props" ) );
    EXPECT_TRUE( definition.resultTask.isEmpty() );
    EXPECT_FALSE( definition.inputs.contains( "import_props" ) );
    for ( const auto& edge : definition.edges )
        EXPECT_NE( "import_props", edge.to );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinitionEdit, optionalInputs )
{
    const auto catalog    = catalogFixture();
    auto       definition = RimWorkflowDefinitionTools::createEmpty( "views" );
    QString    added;
    definition = edited( RimWorkflowDefinitionTools::addTask( definition, catalog, "resinsight.configure_grid_view", &added ) );
    EXPECT_FALSE( definition.findNode( added )->configFields.contains( "time_step" ) );

    definition = edited( RimWorkflowDefinitionTools::setOptionalInputConfigured( definition, added, "time_step", true ) );
    EXPECT_TRUE( definition.findNode( added )->configFields.contains( "time_step" ) );
    definition = edited( RimWorkflowDefinitionTools::setOptionalInputConfigured( definition, added, "time_step", false ) );
    EXPECT_FALSE( definition.findNode( added )->configFields.contains( "time_step" ) );

    EXPECT_FALSE( RimWorkflowDefinitionTools::setOptionalInputConfigured( definition, added, "result_type", true ).has_value() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinitionEdit, readOnlyDefinitionsAreNotEdited )
{
    auto definition     = definitionFixture( "load_data" );
    definition.editable = false;
    EXPECT_FALSE( RimWorkflowDefinitionTools::removeTask( definition, "load_model" ).has_value() );
    EXPECT_FALSE( RimWorkflowDefinitionTools::renameWorkflow( definition, "other" ).has_value() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinitionEdit, names )
{
    EXPECT_TRUE( RimWorkflowDefinitionTools::isIdentifier( "load_model_2" ) );
    EXPECT_FALSE( RimWorkflowDefinitionTools::isIdentifier( "2load" ) );
    EXPECT_FALSE( RimWorkflowDefinitionTools::isIdentifier( "" ) );
    EXPECT_EQ( "resinsight_load_model", RimWorkflowDefinitionTools::identifierFrom( "resinsight.load-model" ) );
    EXPECT_EQ( "_3d", RimWorkflowDefinitionTools::identifierFrom( "3d" ) );
}
