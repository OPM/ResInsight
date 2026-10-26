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
#include "RimWorkflowDescribeTools.h"
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
// conditional.json needs taskmaestro with conditions support and the test workflows on PYTHONPATH:
//   PYTHONPATH=.:rips/tests/test_data/taskmaestro_workflows python -m rips.taskmaestro_helper load
//       rips/tests/test_data/taskmaestro_workflows/conditional --describe > conditional.json
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

QJsonObject definitionTaskByName( const QJsonObject& graph, const QString& name )
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

    const QJsonObject exportTask = definitionTaskByName( graph, "export_completions" );
    EXPECT_TRUE( exportTask.value( "is_result" ).toBool() );
    EXPECT_EQ( 2, exportTask.value( "config_fields" ).toArray().size() );
    EXPECT_EQ( QJsonArray( { "export_file", "well_path_names" } ), exportTask.value( "outputs" ).toArray() );

    // The whole input port comes first, followed by the fields
    const QJsonArray ports = exportTask.value( "input_ports" ).toArray();
    ASSERT_EQ( 7, ports.size() );
    EXPECT_EQ( "", ports[0].toObject().value( "name" ).toString() );
    EXPECT_EQ( "resinsight", ports[1].toObject().value( "name" ).toString() );
    EXPECT_TRUE( ports[1].toObject().value( "wired" ).toBool() );

    // Type names for labelling the ports
    EXPECT_EQ( "RipsInstance", exportTask.value( "input_types" ).toObject().value( "resinsight" ).toString() );
    EXPECT_EQ( "str", exportTask.value( "output_types" ).toObject().value( "export_file" ).toString() );

    // Icons of ResInsight objects, found through `ObjectModel[Class]` and rips bases
    const QJsonObject exportIcons = exportTask.value( "input_icons" ).toObject();
    EXPECT_EQ( ":/AppLogo48x48.png", exportIcons.value( "resinsight" ).toString() );
    EXPECT_EQ( ":/codicons/symbol-string.svg", exportTask.value( "output_icons" ).toObject().value( "export_file" ).toString() );
    EXPECT_EQ( ":/Case48x48.png",
               definitionTaskByName( graph, "select_eclipse_case" ).value( "output_icons" ).toObject().value( "" ).toString() );
    EXPECT_EQ( ":/Well.svg", definitionTaskByName( graph, "select_well_path_1" ).value( "output_icons" ).toObject().value( "" ).toString() );
    EXPECT_EQ( ":/Case48x48.png",
               RimWorkflowPortCompatibility::iconResource( QJsonObject{ { "x-ri-python-type", "rips.generated.generated_classes.Reservoir" } },
                                                           {} ) );
    EXPECT_EQ( ":/3DView16x16.png",
               RimWorkflowPortCompatibility::iconResource( QJsonObject{ { "anyOf",
                                                                          QJsonArray{ QJsonObject{ { "type", "null" } },
                                                                                      QJsonObject{ { "x-ri-python-bases",
                                                                                                     QJsonArray{ "pkg.GridView",
                                                                                                                 "taskmaestro.object_model."
                                                                                                                 "ObjectModel["
                                                                                                                 "View]" } } } } } },
                                                           {} ) );

    // Plain values get an icon for their kind; models without a ResInsight object get none
    auto valueIcon = []( const QJsonObject& schema ) { return RimWorkflowPortCompatibility::iconResource( schema, {} ); };
    EXPECT_EQ( ":/codicons/symbol-string.svg", valueIcon( { { "type", "string" } } ) );
    EXPECT_EQ( ":/codicons/symbol-numeric.svg", valueIcon( { { "type", "integer" } } ) );
    EXPECT_EQ( ":/codicons/symbol-numeric.svg", valueIcon( { { "type", "number" } } ) );
    EXPECT_EQ( ":/codicons/calendar.svg", valueIcon( { { "type", "string" }, { "format", "date" } } ) );
    EXPECT_EQ( ":/codicons/file.svg", valueIcon( { { "type", "string" }, { "format", "path" } } ) );
    EXPECT_EQ( ":/codicons/folder.svg", valueIcon( { { "type", "string" }, { "format", "directory-path" } } ) );
    EXPECT_EQ( ":/codicons/symbol-numeric.svg",
               valueIcon( { { "anyOf", QJsonArray{ QJsonObject{ { "type", "number" } }, QJsonObject{ { "type", "null" } } } } } ) );
    EXPECT_EQ( ":/codicons/symbol-string.svg", valueIcon( { { "type", "array" }, { "items", QJsonObject{ { "type", "string" } } } } ) );
    EXPECT_EQ( ":/codicons/check.svg", valueIcon( { { "type", "boolean" } } ) );
    EXPECT_TRUE(
        valueIcon( { { "type", "object" }, { "properties", QJsonObject{ { "a", QJsonObject{ { "type", "string" } } } } } } ).isEmpty() );

    // The connect task's opaque `value` is not a port of its own
    const QJsonObject connect = definitionTaskByName( graph, "connect_to_resinsight" );
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
    EXPECT_FALSE( definitionTaskByName( graph, "export_completions" ).value( "issues" ).toArray().isEmpty() );
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

namespace
{
QJsonObject numberModel( const QString& title, const QString& field, const QJsonObject& fieldSchema )
{
    return QJsonObject{ { "type", "object" },
                        { "title", title },
                        { "properties", QJsonObject{ { field, fieldSchema } } },
                        { "required", QJsonArray{ field } } };
}

// produce(name: str) -> value: float; scale(name: str, factor: float) -> value: float;
// sum_list(values: list[float]); sum_dict(values: dict[str, float])
RimWorkflowDefinition collectDefinition()
{
    const QJsonObject number{ { "type", "number" } };
    const QJsonObject output = numberModel( "Value", "value", number );
    const QJsonObject nameInput{ { "type", "object" },
                                 { "title", "Named" },
                                 { "properties", QJsonObject{ { "name", QJsonObject{ { "type", "string" } } }, { "factor", number } } },
                                 { "required", QJsonArray{ "name", "factor" } } };

    auto definition = RimWorkflowDefinitionTools::createEmpty( "collect" );
    definition.taskTypes.insert( "t.produce", QJsonObject{ { "name", "produce" }, { "input_schema", nameInput }, { "output_schema", output } } );
    definition.taskTypes.insert( "t.sum_list",
                                 QJsonObject{ { "name", "sum_list" },
                                              { "input_schema",
                                                numberModel( "ListInput", "values", QJsonObject{ { "type", "array" }, { "items", number } } ) },
                                              { "output_schema", output } } );
    definition.taskTypes.insert( "t.sum_dict",
                                 QJsonObject{ { "name", "sum_dict" },
                                              { "input_schema",
                                                numberModel( "DictInput",
                                                             "values",
                                                             QJsonObject{ { "type", "object" }, { "additionalProperties", number } } ) },
                                              { "output_schema", output } } );
    for ( const auto& [name, taskId] : std::vector<std::pair<QString, QString>>{ { "a", "t.produce" },
                                                                                 { "b", "t.produce" },
                                                                                 { "c", "t.produce" },
                                                                                 { "sum_list", "t.sum_list" },
                                                                                 { "sum_dict", "t.sum_dict" } } )
    {
        definition.nodes.push_back( { .name = name, .taskId = taskId } );
        definition.inputs.insert( name, QJsonObject() );
    }
    definition.taskTypes.insert( "t.gather",
                                 QJsonObject{ { "name", "gather" },
                                              { "input_schema",
                                                QJsonObject{ { "type", "object" },
                                                             { "title", "GatherInput" },
                                                             { "properties",
                                                               QJsonObject{ { "results",
                                                                              QJsonObject{ { "type", "object" },
                                                                                           { "additionalProperties",
                                                                                             QJsonObject{ { "$ref", "#/$defs/Value" } } } } } } },
                                                             { "required", QJsonArray{ "results" } },
                                                             { "$defs", QJsonObject{ { "Value", output } } } } },
                                              { "output_schema", output } } );
    definition.nodes.push_back( { .name = "gather", .taskId = "t.gather" } );
    definition.inputs.insert( "gather", QJsonObject() );
    return RimWorkflowDefinitionTools::normalize( definition );
}

QStringList taskIssues( const RimWorkflowDefinition& definition, const QString& task )
{
    std::vector<RimWorkflowIssue> issues;
    for ( const auto& issue : RimWorkflowDefinitionTools::validate( definition ) )
    {
        if ( issue.task == task ) issues.push_back( issue );
    }
    return issueMessages( issues );
}
} // namespace

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinitionEdit, collectIntoList )
{
    using Collect   = RimWorkflowDefinitionEdge::Collect;
    auto definition = collectDefinition();

    definition =
        edited( RimWorkflowDefinitionTools::connect( definition, { .from = "a", .output = "value", .to = "sum_list", .input = "values" } ) );
    definition =
        edited( RimWorkflowDefinitionTools::connect( definition, { .from = "b", .output = "value", .to = "sum_list", .input = "values" } ) );
    ASSERT_EQ( 2u, definition.incomingEdges( "sum_list" ).size() );
    EXPECT_EQ( Collect::List, definition.incomingEdges( "sum_list" )[0].collect );
    EXPECT_EQ( "a", definition.incomingEdges( "sum_list" )[0].from );
    EXPECT_TRUE( definition.findNode( "sum_list" )->configFields.isEmpty() );
    EXPECT_TRUE( taskIssues( definition, "sum_list" ).isEmpty() ) << taskIssues( definition, "sum_list" ).join( "\n" ).toStdString();

    // Collecting the same output twice is rejected
    EXPECT_FALSE( RimWorkflowDefinitionTools::connect( definition, { .from = "a", .output = "value", .to = "sum_list", .input = "values" } ) );

    // Reorder the members
    const RimWorkflowDefinitionEdge second{ .from = "b", .output = "value", .to = "sum_list", .input = "values" };
    definition = edited( RimWorkflowDefinitionTools::moveCollectMember( definition, second, -1 ) );
    EXPECT_EQ( "b", definition.incomingEdges( "sum_list" )[0].from );
    EXPECT_FALSE( RimWorkflowDefinitionTools::moveCollectMember( definition, second, -1 ) );

    // Index labels in the graph
    const QJsonArray edges = RimWorkflowDefinitionTools::graphFromDefinition( definition, true ).value( "edges" ).toArray();
    QStringList      labels;
    for ( const QJsonValue& edge : edges )
        labels.append( edge.toObject().value( "from" ).toString() + edge.toObject().value( "label" ).toString() );
    EXPECT_TRUE( labels.contains( "b[0]" ) );
    EXPECT_TRUE( labels.contains( "a[1]" ) );

    // Disconnecting ignores how the member is collected
    definition = edited(
        RimWorkflowDefinitionTools::disconnect( definition, { .from = "a", .output = "value", .to = "sum_list", .input = "values" } ) );
    EXPECT_EQ( 1u, definition.incomingEdges( "sum_list" ).size() );

    // JSON round trip keeps the collected edges
    const auto roundTrip = RimWorkflowDefinitionTools::fromJson( RimWorkflowDefinitionTools::toJson( definition ) );
    ASSERT_TRUE( roundTrip.has_value() );
    EXPECT_EQ( definition.edges, roundTrip->edges );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinitionEdit, collectIntoDict )
{
    using Collect   = RimWorkflowDefinitionEdge::Collect;
    auto definition = collectDefinition();

    definition =
        edited( RimWorkflowDefinitionTools::connect( definition, { .from = "a", .output = "value", .to = "sum_dict", .input = "values" } ) );
    definition =
        edited( RimWorkflowDefinitionTools::connect( definition, { .from = "b", .output = "value", .to = "sum_dict", .input = "values" } ) );
    const auto incoming = definition.incomingEdges( "sum_dict" );
    ASSERT_EQ( 2u, incoming.size() );
    EXPECT_EQ( Collect::Dict, incoming[0].collect );
    EXPECT_EQ( "a_value", incoming[0].key );
    EXPECT_EQ( "b_value", incoming[1].key );

    const RimWorkflowDefinitionEdge first{ .from = "a", .output = "value", .to = "sum_dict", .input = "values" };
    EXPECT_FALSE( RimWorkflowDefinitionTools::setCollectKey( definition, first, "b_value" ) );
    EXPECT_FALSE( RimWorkflowDefinitionTools::setCollectKey( definition, first, "" ) );
    EXPECT_FALSE( RimWorkflowDefinitionTools::setCollectKey( definition, first, " b_value " ) );
    definition = edited( RimWorkflowDefinitionTools::setCollectKey( definition, first, " low " ) );
    EXPECT_EQ( "low", definition.incomingEdges( "sum_dict" )[0].key );
    EXPECT_TRUE( taskIssues( definition, "sum_dict" ).isEmpty() ) << taskIssues( definition, "sum_dict" ).join( "\n" ).toStdString();

    const auto roundTrip = RimWorkflowDefinitionTools::fromJson( RimWorkflowDefinitionTools::toJson( definition ) );
    ASSERT_TRUE( roundTrip.has_value() );
    EXPECT_EQ( definition.edges, roundTrip->edges );

    // Duplicate keys are reported
    definition.edges[1].key = "low";
    EXPECT_FALSE( taskIssues( definition, "sum_dict" ).isEmpty() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDefinitionEdit, mapTask )
{
    auto definition = collectDefinition();

    // The key input must accept a string
    EXPECT_FALSE(
        RimWorkflowDefinitionTools::setTaskMap( definition, "a", RimWorkflowTaskMap{ .over = "items", .keyAs = "factor", .valueAs = "name" } ) );
    EXPECT_FALSE(
        RimWorkflowDefinitionTools::setTaskMap( definition, "a", RimWorkflowTaskMap{ .over = "name", .keyAs = "name", .valueAs = "factor" } ) );
    EXPECT_FALSE(
        RimWorkflowDefinitionTools::setTaskMap( definition, "a", RimWorkflowTaskMap{ .over = "items", .keyAs = "name", .valueAs = "name" } ) );

    const RimWorkflowTaskMap map{ .over = "items", .keyAs = "name", .valueAs = "factor" };
    definition = edited( RimWorkflowDefinitionTools::setTaskMap( definition, "a", map ) );
    ASSERT_TRUE( definition.findNode( "a" )->map.has_value() );
    EXPECT_TRUE( definition.findNode( "a" )->configFields.isEmpty() );

    // The mapped output is a dict[str, Value]: it feeds a dict[str, Value] input directly
    const QJsonObject mappedType = definition.taskTypeForNode( "a" );
    EXPECT_EQ( "dict[str, Value]", RimWorkflowPortCompatibility::outputPorts( mappedType ).front().typeName );
    EXPECT_FALSE( RimWorkflowPortCompatibility::canConnect( definition, "a", "", "sum_dict", "values" ) );
    EXPECT_FALSE( RimWorkflowPortCompatibility::canConnect( definition, "a", "", "sum_list", "values" ) );
    definition = edited( RimWorkflowDefinitionTools::connect( definition, { .from = "a", .to = "gather", .input = "results" } ) );
    EXPECT_FALSE( definition.incomingEdges( "gather" ).front().isCollected() );
    // A mapped task takes its inputs field by field
    EXPECT_FALSE( RimWorkflowPortCompatibility::canConnect( definition, "b", "", "a", "" ) );

    // The mapping is a config value shown as an input port
    const QJsonObject task = definitionTaskByName( RimWorkflowDefinitionTools::graphFromDefinition( definition, true ), "a" );
    EXPECT_EQ( "items", task.value( "map" ).toObject().value( "over" ).toString() );
    EXPECT_EQ( "dict[str, float]", task.value( "input_types" ).toObject().value( "items" ).toString() );
    bool hasOverField = false;
    for ( const QJsonValue& field : task.value( "config_fields" ).toArray() )
    {
        if ( field.toObject().value( "name" ).toString() == "items" )
        {
            hasOverField = field.toObject().value( "map_over" ).toBool();
            EXPECT_EQ( "number", field.toObject().value( "value_type" ).toString() );
        }
    }
    EXPECT_TRUE( hasOverField );

    // JSON round trip
    const auto roundTrip = RimWorkflowDefinitionTools::fromJson( RimWorkflowDefinitionTools::toJson( definition ) );
    ASSERT_TRUE( roundTrip.has_value() );
    EXPECT_EQ( definition, roundTrip.value() );

    // Editing the map keeps the items, also when they are renamed
    const QJsonObject items{ { "x", 1.0 } };
    definition.inputs.insert( "a", QJsonObject{ { "items", items } } );
    RimWorkflowTaskMap collectAll = map;
    collectAll.errorMode          = "collect_all";
    definition                    = edited( RimWorkflowDefinitionTools::setTaskMap( definition, "a", collectAll ) );
    EXPECT_EQ( items, definition.inputs.value( "a" ).toObject().value( "items" ).toObject() );
    RimWorkflowTaskMap renamed = collectAll;
    renamed.over               = "cases";
    definition                 = edited( RimWorkflowDefinitionTools::setTaskMap( definition, "a", renamed ) );
    EXPECT_FALSE( definition.inputs.value( "a" ).toObject().contains( "items" ) );
    EXPECT_EQ( items, definition.inputs.value( "a" ).toObject().value( "cases" ).toObject() );

    // Removing the map restores the inputs and drops the mapping value
    definition.inputs.insert( "a", QJsonObject{ { "cases", items } } );
    definition = edited( RimWorkflowDefinitionTools::setTaskMap( definition, "a", std::nullopt ) );
    EXPECT_FALSE( definition.findNode( "a" )->map.has_value() );
    EXPECT_EQ( QStringList( { "name", "factor" } ), definition.findNode( "a" )->configFields );
    EXPECT_FALSE( definition.inputs.value( "a" ).toObject().contains( "cases" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, fromJsonAndRoundTrip )
{
    using Op                 = RimWorkflowTaskGuard::Op;
    const auto definition    = definitionFixture( "conditional" );
    const auto guardOf       = [&definition]( const QString& name ) { return definition.findNode( name )->when; };
    const auto largeGuard    = guardOf( "double_large" );
    const auto smallGuard    = guardOf( "double_small" );
    const auto positiveGuard = guardOf( "double_positive" );
    const auto specialGuard  = guardOf( "special" );

    EXPECT_TRUE( definition.editable );
    EXPECT_TRUE( RimWorkflowDefinitionTools::structuralReadOnlyReasons( definition ).isEmpty() );
    ASSERT_TRUE( largeGuard && smallGuard && positiveGuard && specialGuard );
    EXPECT_EQ( ( RimWorkflowTaskGuard{ .source = "choose_size", .op = Op::Case, .values = { "large", "huge" }, .caseAsList = true } ),
               *largeGuard );
    EXPECT_EQ( ( RimWorkflowTaskGuard{ .source = "choose_size", .op = Op::Otherwise } ), *smallGuard );
    EXPECT_EQ( ( RimWorkflowTaskGuard{ .source = "is_positive", .op = Op::Case, .values = { true } } ), *positiveGuard );
    EXPECT_EQ( ( RimWorkflowTaskGuard{ .source = "start", .field = "value", .op = Op::In, .values = { 1.0, 2.0 } } ), *specialGuard );
    EXPECT_FALSE( guardOf( "report" ).has_value() );

    int firstOf = 0;
    for ( const auto& edge : definition.edges )
    {
        if ( edge.to == "report" ) EXPECT_TRUE( edge.isFirstOf() );
        if ( edge.isFirstOf() ) ++firstOf;
    }
    EXPECT_EQ( 4, firstOf );

    // The YAML forms of the guards and the order of the alternatives survive a round trip
    const QJsonObject json = RimWorkflowDefinitionTools::toJson( definition );
    // Written nodes also carry ResInsight's explicit config field marker
    QJsonArray writtenNodes;
    for ( const QJsonValue& node : json.value( "nodes" ).toArray() )
    {
        QJsonObject object = node.toObject();
        object.remove( "x_ri_explicit_config_fields" );
        writtenNodes.append( object );
    }
    EXPECT_EQ( fixture( "conditional" ).value( "nodes" ).toArray(), writtenNodes );
    EXPECT_EQ( fixture( "conditional" ).value( "edges" ), json.value( "edges" ) );
    const auto roundTrip = RimWorkflowDefinitionTools::fromJson( json );
    ASSERT_TRUE( roundTrip.has_value() );
    EXPECT_EQ( definition, roundTrip.value() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, validFixtureAndSinks )
{
    const auto definition = definitionFixture( "conditional" );
    EXPECT_TRUE( RimWorkflowDefinitionTools::validate( definition ).empty() )
        << issueMessages( RimWorkflowDefinitionTools::validate( definition ) ).join( "\n" ).toStdString();
    EXPECT_EQ( definition, RimWorkflowDefinitionTools::normalize( definition ) );

    // Switch tasks used only by run conditions are not end tasks
    QStringList sinks = RimWorkflowDefinitionTools::sinkTasks( definition );
    sinks.sort();
    EXPECT_EQ( QStringList( { "report", "special" } ), sinks );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, switchBranches )
{
    const auto definition = definitionFixture( "conditional" );
    const auto typeOf     = [&definition]( const QString& name ) { return definition.taskType( definition.findNode( name )->taskId ); };

    EXPECT_TRUE( RimWorkflowDefinitionTools::isSwitchTask( typeOf( "choose_size" ) ) );
    EXPECT_TRUE( RimWorkflowDefinitionTools::isSwitchTask( typeOf( "is_positive" ) ) );
    EXPECT_FALSE( RimWorkflowDefinitionTools::isSwitchTask( typeOf( "start" ) ) );
    EXPECT_EQ( QJsonArray( { "small", "large", "huge" } ), RimWorkflowDefinitionTools::switchBranches( typeOf( "choose_size" ) ) );
    EXPECT_EQ( QJsonArray( { true, false } ), RimWorkflowDefinitionTools::switchBranches( typeOf( "is_positive" ) ) );
    EXPECT_TRUE( RimWorkflowDefinitionTools::switchBranches( typeOf( "start" ) ).isEmpty() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, checkTaskGuard )
{
    using Op              = RimWorkflowTaskGuard::Op;
    const auto definition = definitionFixture( "conditional" );
    const auto check      = [&definition]( const QString& node, const RimWorkflowTaskGuard& guard )
    {
        const auto result = RimWorkflowDefinitionTools::checkTaskGuard( definition, node, guard );
        return result ? QString() : result.error();
    };

    EXPECT_EQ( "", check( "special", { .source = "choose_size", .op = Op::Case, .values = { "small" } } ) );
    EXPECT_EQ( "", check( "special", { .source = "start", .field = "value", .op = Op::Equals, .values = { 3 } } ) );
    EXPECT_EQ( "", check( "special", { .source = "choose_size", .op = Op::Otherwise } ) );

    EXPECT_TRUE( check( "special", { .source = "missing", .op = Op::Otherwise } ).contains( "unknown task" ) );
    EXPECT_TRUE( check( "special", { .source = "special", .field = "value", .op = Op::Truthy } ).contains( "its own output" ) );
    EXPECT_TRUE( check( "special", { .source = "choose_size", .op = Op::Case, .values = { "tiny" } } ).contains( "not a valid value" ) );
    EXPECT_TRUE( check( "special", { .source = "choose_size", .op = Op::Case } ).contains( "at least one branch" ) );
    EXPECT_TRUE( check( "special", { .source = "start", .op = Op::Case, .values = { 1 } } ).contains( "not a switch task" ) );
    EXPECT_TRUE( check( "special", { .source = "start", .field = "value", .op = Op::Otherwise } ).contains( "requires a switch" ) );
    EXPECT_TRUE( check( "special", { .source = "start", .field = "value", .op = Op::Truthy } ).contains( "bool or optional" ) );
    EXPECT_TRUE(
        check( "special", { .source = "start", .field = "value", .op = Op::Equals, .values = { "text" } } ).contains( "not a valid value" ) );
    EXPECT_TRUE( check( "special", { .source = "start", .field = "missing", .op = Op::Truthy } ).contains( "no output field" ) );
    EXPECT_TRUE( check( "choose_size", { .source = "report", .field = "sized", .op = Op::Truthy } ).contains( "cycle" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, validateSwitchCoverage )
{
    using Op              = RimWorkflowTaskGuard::Op;
    const auto definition = definitionFixture( "conditional" );

    // Without the otherwise branch, `small` is not handled by any task
    const auto uncovered = edited( RimWorkflowDefinitionTools::setTaskGuard( definition, "double_small", std::nullopt ) );
    EXPECT_TRUE( issueMessages( RimWorkflowDefinitionTools::validate( uncovered ) ).join( "\n" ).contains( "small" ) );

    // An otherwise branch that can never match
    const auto allLabels = edited( RimWorkflowDefinitionTools::setTaskGuard( definition,
                                                                             "double_large",
                                                                             RimWorkflowTaskGuard{ .source = "choose_size",
                                                                                                   .op     = Op::Case,
                                                                                                   .values = { "small", "large", "huge" } } ) );
    const auto issues    = RimWorkflowDefinitionTools::validate( allLabels );
    ASSERT_FALSE( issues.empty() );
    EXPECT_EQ( "choose_size", issues.front().task );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, guardsFollowRenamesAndRemovals )
{
    const auto definition = definitionFixture( "conditional" );

    const auto renamed = edited( RimWorkflowDefinitionTools::renameTask( definition, "choose_size", "picker" ) );
    EXPECT_EQ( "picker", renamed.findNode( "double_large" )->when->source );
    EXPECT_EQ( "picker", renamed.findNode( "double_small" )->when->source );

    const auto removed = edited( RimWorkflowDefinitionTools::removeTask( definition, "is_positive" ) );
    EXPECT_FALSE( removed.findNode( "double_positive" )->when.has_value() );
    EXPECT_FALSE( removed.findNode( "constant" )->when.has_value() );
    EXPECT_TRUE( removed.findNode( "double_large" )->when.has_value() );

    const auto cleared = edited( RimWorkflowDefinitionTools::setTaskGuard( definition, "special", std::nullopt ) );
    EXPECT_FALSE( cleared.findNode( "special" )->when.has_value() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, firstOfAlternatives )
{
    using Collect         = RimWorkflowDefinitionEdge::Collect;
    const auto definition = definitionFixture( "conditional" );

    // Removing one of two alternatives leaves a plain connection
    const auto single =
        edited( RimWorkflowDefinitionTools::disconnect( definition, { .from = "double_small", .to = "report", .input = "sized" } ) );
    EXPECT_TRUE( hasEdge( single, { .from = "double_large", .to = "report", .input = "sized" } ) );

    // ... and adding an alternative to it makes the connection the first alternative
    const auto again =
        edited( RimWorkflowDefinitionTools::addFirstOfMember( single, { .from = "double_small", .to = "report", .input = "sized" } ) );
    EXPECT_TRUE( hasEdge( again, { .from = "double_large", .to = "report", .input = "sized", .collect = Collect::FirstOf } ) );
    EXPECT_TRUE( hasEdge( again, { .from = "double_small", .to = "report", .input = "sized", .collect = Collect::FirstOf } ) );
    EXPECT_EQ( definition, again );

    const auto third =
        edited( RimWorkflowDefinitionTools::addFirstOfMember( definition, { .from = "double_positive", .to = "report", .input = "sized" } ) );
    EXPECT_TRUE( hasEdge( third, { .from = "double_positive", .to = "report", .input = "sized", .collect = Collect::FirstOf } ) );
    const auto moved = edited(
        RimWorkflowDefinitionTools::moveCollectMember( third,
                                                       { .from = "double_positive", .to = "report", .input = "sized", .collect = Collect::FirstOf },
                                                       -2 ) );
    for ( const auto& edge : moved.edges )
    {
        if ( edge.input != "sized" ) continue;
        EXPECT_EQ( "double_positive", edge.from );
        break;
    }

    EXPECT_FALSE(
        RimWorkflowDefinitionTools::addFirstOfMember( definition, { .from = "start", .to = "special", .input = "value" } ).has_value() );
    EXPECT_FALSE(
        RimWorkflowDefinitionTools::addFirstOfMember( definition, { .from = "double_small", .to = "report", .input = "sized" } ).has_value() );

    // A plain connection replaces all alternatives
    const auto replaced =
        edited( RimWorkflowDefinitionTools::connect( definition, { .from = "double_positive", .to = "report", .input = "sized" } ) );
    EXPECT_FALSE( hasEdge( replaced, { .from = "double_large", .to = "report", .input = "sized", .collect = Collect::FirstOf } ) );
    EXPECT_TRUE( hasEdge( replaced, { .from = "double_positive", .to = "report", .input = "sized" } ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, graphFromDefinition )
{
    const auto        definition = definitionFixture( "conditional" );
    const QJsonObject graph      = RimWorkflowDefinitionTools::graphFromDefinition( definition, true );

    EXPECT_TRUE( definitionTaskByName( graph, "choose_size" ).value( "switch" ).toBool() );
    EXPECT_EQ( QJsonArray( { "small", "large", "huge" } ), definitionTaskByName( graph, "choose_size" ).value( "branches" ).toArray() );
    EXPECT_FALSE( definitionTaskByName( graph, "start" ).value( "switch" ).toBool() );
    EXPECT_EQ( "large, huge", definitionTaskByName( graph, "double_large" ).value( "when_label" ).toString() );

    QMap<QString, QString> conditions;
    QMap<QString, QString> alternatives;
    for ( const QJsonValue& value : graph.value( "edges" ).toArray() )
    {
        const QJsonObject edge = value.toObject();
        if ( edge.value( "kind" ).toString() == "when" )
            conditions.insert( edge.value( "to" ).toString(), edge.value( "from" ).toString() + ": " + edge.value( "label" ).toString() );
        if ( edge.value( "collect" ).toString() == "first_of" )
            alternatives.insert( edge.value( "from" ).toString(), edge.value( "label" ).toString() );
    }
    const QMap<QString, QString> expectedConditions{ { "double_large", "choose_size: large, huge" },
                                                     { "double_small", "choose_size: otherwise" },
                                                     { "double_positive", "is_positive: true" },
                                                     { "constant", "is_positive: false" },
                                                     { "special", "start: value in {1, 2}" } };
    EXPECT_EQ( expectedConditions, conditions );
    const QMap<QString, QString> expectedAlternatives{ { "double_large", "1st" },
                                                       { "double_small", "2nd" },
                                                       { "double_positive", "1st" },
                                                       { "constant", "2nd" } };
    EXPECT_EQ( expectedAlternatives, alternatives );

    for ( const QJsonValue& port : definitionTaskByName( graph, "report" ).value( "input_ports" ).toArray() )
    {
        const QString name = port.toObject().value( "name" ).toString();
        if ( name == "sized" || name == "signed" ) EXPECT_TRUE( port.toObject().value( "first_of" ).toBool() ) << name.toStdString();
    }
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, graphFromDescribe )
{
    const QJsonObject graph = RimWorkflowDescribeTools::graphFromDescribe( fixture( "conditional" ).value( "describe" ).toObject() );

    EXPECT_TRUE( definitionTaskByName( graph, "choose_size" ).value( "switch" ).toBool() );
    EXPECT_FALSE( definitionTaskByName( graph, "report" ).value( "switch" ).toBool() );

    QStringList conditions;
    QStringList alternatives;
    for ( const QJsonValue& value : graph.value( "edges" ).toArray() )
    {
        const QJsonObject edge = value.toObject();
        if ( edge.value( "kind" ).toString() == "when" )
            conditions.append(
                QString( "%1->%2: %3" ).arg( edge.value( "from" ).toString(), edge.value( "to" ).toString(), edge.value( "label" ).toString() ) );
        if ( edge.value( "collect" ).toString() == "first_of" )
            alternatives.append( QString( "%1->%2.%3 %4" )
                                     .arg( edge.value( "from" ).toString(),
                                           edge.value( "to" ).toString(),
                                           edge.value( "input" ).toString(),
                                           edge.value( "label" ).toString() ) );
    }
    conditions.sort();
    alternatives.sort();
    EXPECT_EQ( QStringList( { "choose_size->double_large: large, huge",
                              "choose_size->double_small: otherwise",
                              "is_positive->constant: false",
                              "is_positive->double_positive: true",
                              "start->special: value in {1, 2}" } ),
               conditions );
    EXPECT_EQ( QStringList( { "constant->report.signed 2nd",
                              "double_large->report.sized 1st",
                              "double_positive->report.signed 1st",
                              "double_small->report.sized 2nd" } ),
               alternatives );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, guardValuesText )
{
    using RimWorkflowDefinitionTools::guardValuesFromText;
    using RimWorkflowDefinitionTools::guardValuesToText;
    const QJsonObject numberField{ { "type", "number" } };
    const QJsonObject stringField{ { "type", "string" } };

    EXPECT_EQ( QJsonArray( { 1, "a", true } ), guardValuesFromText( "1, a, true", numberField ) );
    EXPECT_EQ( QJsonArray( { "1", "a", "true" } ), guardValuesFromText( "1, a, true", stringField ) );

    // Quoted values are strings and may contain commas and escaped quotes
    EXPECT_EQ( QJsonArray( { "a, b", "1", "say \"hi\"" } ), guardValuesFromText( "\"a, b\", \"1\", \"say \\\"hi\\\"\"", numberField ) );
    EXPECT_EQ( QJsonArray( { "a, b", "c" } ), guardValuesFromText( "\"a, b\", c", stringField ) );

    // Values read back as written, with strings quoted only when needed
    const QJsonArray values{ "1", "a, b", "text", 2.5, false, QJsonValue::Null, "say \"hi\"" };
    EXPECT_EQ( values, guardValuesFromText( guardValuesToText( values, numberField ), numberField ) );
    const QJsonArray strings{ "1", "a, b", "text", "\"quoted\"", "say \"hi\"" };
    EXPECT_EQ( strings, guardValuesFromText( guardValuesToText( strings, stringField ), stringField ) );
    EXPECT_EQ( "\"1\", text", guardValuesToText( QJsonArray( { "1", "text" } ), numberField ) );
    EXPECT_EQ( "1, text", guardValuesToText( QJsonArray( { "1", "text" } ), stringField ) );

    // A string Literal has enum or const but no type
    EXPECT_EQ( QJsonArray( { "1", "true" } ), guardValuesFromText( "1, true", QJsonObject{ { "enum", QJsonArray( { "1", "true" } ) } } ) );
    EXPECT_EQ( QJsonArray( { "1" } ), guardValuesFromText( "1", QJsonObject{ { "const", "1" } } ) );
    EXPECT_EQ( QJsonArray( { 1 } ), guardValuesFromText( "1", QJsonObject{ { "enum", QJsonArray( { 1, "a" } ) } } ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, malformedFieldRef )
{
    QJsonObject json  = fixture( "conditional" );
    QJsonArray  nodes = json.value( "nodes" ).toArray();
    for ( qsizetype i = 0; i < nodes.size(); ++i )
    {
        QJsonObject node = nodes.at( i ).toObject();
        if ( node.value( "name" ) != "double_large" ) continue;
        node["when"] = QJsonObject{ { "ref", QJsonArray( { "choose_size" } ) }, { "truthy", true } };
        nodes.replace( i, node );
    }
    json["nodes"] = nodes;

    const auto definition = RimWorkflowDefinitionTools::fromJson( json );
    ASSERT_TRUE( definition.has_value() );
    const auto guard = definition->findNode( "double_large" )->when;
    ASSERT_TRUE( guard.has_value() );
    EXPECT_FALSE( guard->isSwitchBranch() );
    EXPECT_TRUE( issueMessages( RimWorkflowDefinitionTools::validate( *definition ) ).join( "\n" ).contains( "task and an output field" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowConditional, switchWithoutCase )
{
    QJsonObject json  = fixture( "conditional" );
    QJsonArray  nodes = json.value( "nodes" ).toArray();
    for ( qsizetype i = 0; i < nodes.size(); ++i )
    {
        QJsonObject node = nodes.at( i ).toObject();
        if ( node.value( "name" ) != "double_large" ) continue;
        node["when"] = QJsonObject{ { "switch", "choose_size" } };
        nodes.replace( i, node );
    }
    json["nodes"] = nodes;

    const auto definition = RimWorkflowDefinitionTools::fromJson( json );
    ASSERT_TRUE( definition.has_value() );
    const auto guard = definition->findNode( "double_large" )->when;
    ASSERT_TRUE( guard.has_value() );
    EXPECT_TRUE( guard->values.isEmpty() );
    EXPECT_TRUE( issueMessages( RimWorkflowDefinitionTools::validate( *definition ) ).join( "\n" ).contains( "at least one branch" ) );
}
