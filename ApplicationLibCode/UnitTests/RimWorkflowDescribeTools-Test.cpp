#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"
#include "RimWorkflowDescribeTools.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Output of `taskmaestro workflow describe workflow.yaml --input input.yaml --json` for a
/// synthetic workflow: start -> greet_a, greet_b -> merge (collect), merge <- start.count
//--------------------------------------------------------------------------------------------------
QJsonObject describeFixture()
{
    QFile file( QString( "%1/RimWorkflowDescribeTools/describe.json" ).arg( TEST_DATA_DIR ) );
    if ( !file.open( QIODevice::ReadOnly ) ) return {};
    return QJsonDocument::fromJson( file.readAll() ).object();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonObject taskByName( const QJsonObject& graph, const QString& name )
{
    for ( const QJsonValue& task : graph.value( "tasks" ).toArray() )
    {
        if ( task.toObject().value( "name" ).toString() == name ) return task.toObject();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonObject fieldByName( const QJsonObject& task, const QString& name )
{
    for ( const QJsonValue& field : task.value( "config_fields" ).toArray() )
    {
        if ( field.toObject().value( "name" ).toString() == name ) return field.toObject();
    }
    return {};
}
} // namespace

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDescribeTools, tasksInputsAndOutputs )
{
    const QJsonObject describe = describeFixture();
    ASSERT_FALSE( describe.isEmpty() );

    const QJsonObject graph = RimWorkflowDescribeTools::graphFromDescribe( describe );
    EXPECT_EQ( "synthetic", graph.value( "name" ).toString() );

    QStringList names;
    for ( const QJsonValue& task : graph.value( "tasks" ).toArray() )
        names.append( task.toObject().value( "name" ).toString() );
    EXPECT_EQ( QStringList( { "start", "greet_a", "greet_b", "merge" } ), names );

    // The opaque ObjectModel `value` is not an output port
    EXPECT_EQ( QJsonArray( { "count" } ), taskByName( graph, "start" ).value( "outputs" ).toArray() );
    EXPECT_EQ( QJsonArray( { "message" } ), taskByName( graph, "greet_a" ).value( "outputs" ).toArray() );
    EXPECT_EQ( QJsonArray( { "count", "messages" } ), taskByName( graph, "merge" ).value( "inputs" ).toArray() );
    EXPECT_TRUE( taskByName( graph, "start" ).value( "config_fields" ).toArray().isEmpty() );
    EXPECT_EQ( 9, taskByName( graph, "greet_a" ).value( "config_fields" ).toArray().size() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDescribeTools, configFieldTypesAndValues )
{
    const QJsonObject graph  = RimWorkflowDescribeTools::graphFromDescribe( describeFixture() );
    const QJsonObject greetA = taskByName( graph, "greet_a" );
    const QJsonObject greetB = taskByName( graph, "greet_b" );

    // input.yaml value wins over the model default
    const QJsonObject name = fieldByName( greetA, "name" );
    EXPECT_EQ( "string", name.value( "type" ).toString() );
    EXPECT_EQ( "alice", name.value( "default" ).toString() );
    EXPECT_EQ( "Person to greet", name.value( "description" ).toString() );
    EXPECT_FALSE( name.value( "required" ).toBool() );
    EXPECT_EQ( "world", fieldByName( greetB, "name" ).value( "default" ).toString() );
    EXPECT_EQ( "str", greetA.value( "input_types" ).toObject().value( "name" ).toString() );
    EXPECT_EQ( "int", greetA.value( "input_types" ).toObject().value( "times" ).toString() );

    const QJsonObject times = fieldByName( greetA, "times" );
    EXPECT_EQ( "integer", times.value( "type" ).toString() );
    EXPECT_TRUE( times.value( "required" ).toBool() );
    EXPECT_EQ( 3, times.value( "default" ).toInt() );
    EXPECT_FALSE( fieldByName( greetB, "times" ).contains( "default" ) );

    // Optional[int] is peeled; an explicit null in input.yaml is preserved
    const QJsonObject optionalCount = fieldByName( greetA, "optional_count" );
    EXPECT_EQ( "integer", optionalCount.value( "type" ).toString() );
    EXPECT_TRUE( optionalCount.contains( "default" ) );
    EXPECT_TRUE( optionalCount.value( "default" ).isNull() );

    EXPECT_EQ( "array", fieldByName( greetA, "values" ).value( "type" ).toString() );

    const QJsonObject when = fieldByName( greetA, "when" );
    EXPECT_EQ( "date", when.value( "format" ).toString() );
    EXPECT_EQ( "2024-01-01", when.value( "default" ).toString() );

    EXPECT_EQ( "path", fieldByName( greetA, "out_file" ).value( "format" ).toString() );
    EXPECT_EQ( "/tmp/hi.txt", fieldByName( greetA, "out_file" ).value( "default" ).toString() );
    EXPECT_EQ( "directory-path", fieldByName( greetA, "out_dir" ).value( "format" ).toString() );
    EXPECT_EQ( "/tmp", fieldByName( greetA, "out_dir" ).value( "default" ).toString() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDescribeTools, resinsightObjectFields )
{
    const QJsonObject graph  = RimWorkflowDescribeTools::graphFromDescribe( describeFixture() );
    const QJsonObject greetA = taskByName( graph, "greet_a" );

    const QJsonObject wellPath = fieldByName( greetA, "well_path" );
    EXPECT_EQ( "object", wellPath.value( "type" ).toString() );
    EXPECT_EQ( "WellPath", wellPath.value( "resinsight_type" ).toString() );
    EXPECT_EQ( "Well path to use", wellPath.value( "description" ).toString() );
    EXPECT_EQ( "WellPath", wellPath.value( "default" ).toObject().value( "__resinsight_ref__" ).toString() );
    EXPECT_EQ( "B-2H", wellPath.value( "default" ).toObject().value( "well_path_name" ).toString() );

    const QJsonObject gridCase = fieldByName( greetA, "case" );
    EXPECT_EQ( "EclipseCase", gridCase.value( "resinsight_type" ).toString() );
    EXPECT_TRUE( gridCase.value( "required" ).toBool() );

    EXPECT_FALSE( fieldByName( greetA, "name" ).contains( "resinsight_type" ) );
}

//--------------------------------------------------------------------------------------------------
/// taskmaestro_resinsight.models.Vec3 is a plain pydantic model (x, y, z numbers) with no
/// x-taskmaestro-python-type tag, so it must be recognized structurally.
//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDescribeTools, vec3FieldIsRecognizedStructurally )
{
    const QJsonObject vec3Schema{ { "title", "Vec3" },
                                  { "type", "object" },
                                  { "properties",
                                    QJsonObject{ { "x", QJsonObject{ { "title", "X" }, { "type", "number" } } },
                                                 { "y", QJsonObject{ { "title", "Y" }, { "type", "number" } } },
                                                 { "z", QJsonObject{ { "title", "Z" }, { "type", "number" } } } } },
                                  { "required", QJsonArray{ "x", "y", "z" } } };

    const QJsonObject describe{ { "workflow", "well_path" },
                                { "tasks",
                                  QJsonArray{ QJsonObject{ { "name", "create_well_path" },
                                                           { "config_fields", QJsonArray{ "reference_point" } },
                                                           { "input_schema",
                                                             QJsonObject{ { "properties", QJsonObject{ { "reference_point", vec3Schema } } },
                                                                          { "required", QJsonArray{ "reference_point" } } } } } } } };

    const QJsonObject graph          = RimWorkflowDescribeTools::graphFromDescribe( describe );
    const QJsonObject referencePoint = fieldByName( taskByName( graph, "create_well_path" ), "reference_point" );
    EXPECT_EQ( "object", referencePoint.value( "type" ).toString() );
    EXPECT_EQ( "Vec3", referencePoint.value( "resinsight_type" ).toString() );
    EXPECT_TRUE( referencePoint.value( "required" ).toBool() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDescribeTools, dependencyEdges )
{
    const QJsonObject graph = RimWorkflowDescribeTools::graphFromDescribe( describeFixture() );

    const QJsonArray expected = {
        QJsonObject{ { "from", "start" }, { "to", "greet_a" } },
        QJsonObject{ { "from", "start" }, { "to", "greet_b" } },
        QJsonObject{ { "from", "start" }, { "to", "merge" }, { "input", "count" }, { "output", "count" } },
        QJsonObject{ { "from", "greet_a" }, { "to", "merge" }, { "input", "messages" }, { "output", "message" } },
        QJsonObject{ { "from", "greet_b" }, { "to", "merge" }, { "input", "messages" }, { "output", "message" } },
    };
    EXPECT_EQ( expected, graph.value( "edges" ).toArray() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDescribeTools, singleFieldAndKeyedCollectionEdges )
{
    const QJsonObject keyedMembers{ { "a", QJsonObject{ { "task", "first" }, { "field", QJsonValue() } } },
                                    { "b", QJsonObject{ { "task", "second" }, { "field", "value" } } } };
    const QJsonObject describe{
        { "workflow", "routing" },
        { "tasks",
          QJsonArray{ QJsonObject{ { "name", "next" }, { "depends_on", QJsonObject{ { "task", "producer" }, { "field", "value" } } } },
                      QJsonObject{ { "name", "merge" },
                                   { "depends_on",
                                     QJsonObject{ { "items",
                                                    QJsonObject{ { "collect",
                                                                   QJsonObject{ { "kind", "keyed" }, { "members", keyedMembers } } } } } } } } } },
    };

    const QJsonArray expected = {
        QJsonObject{ { "from", "producer" }, { "to", "next" }, { "output", "value" } },
        QJsonObject{ { "from", "first" }, { "to", "merge" }, { "input", "items" } },
        QJsonObject{ { "from", "second" }, { "to", "merge" }, { "input", "items" }, { "output", "value" } },
    };
    EXPECT_EQ( expected, RimWorkflowDescribeTools::graphFromDescribe( describe ).value( "edges" ).toArray() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDescribeTools, missingConfigFieldIsReported )
{
    const QJsonObject describe{ { "workflow", "broken" },
                                { "tasks",
                                  QJsonArray{ QJsonObject{ { "name", "task" },
                                                           { "config_fields", QJsonArray{ "ghost" } },
                                                           { "input_schema", QJsonObject{ { "properties", QJsonObject{} } } } } } } };

    const QJsonObject field = fieldByName( taskByName( RimWorkflowDescribeTools::graphFromDescribe( describe ), "task" ), "ghost" );
    EXPECT_EQ( "string", field.value( "type" ).toString() );
    EXPECT_TRUE( field.value( "required" ).toBool() );
    EXPECT_FALSE( field.value( "error" ).toString().isEmpty() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDescribeTools, errorFromDescribe )
{
    EXPECT_TRUE( RimWorkflowDescribeTools::errorFromDescribe( describeFixture() ).isEmpty() );

    const QJsonObject invalid{ { "status", "invalid" },
                               { "error",
                                 QJsonObject{ { "message", "Workflow configuration is invalid" }, { "task", "greet" }, { "field", "times" } } } };
    EXPECT_EQ( "Workflow configuration is invalid (task 'greet', field 'times')", RimWorkflowDescribeTools::errorFromDescribe( invalid ) );

    const QJsonObject invalidWithoutDetails{ { "status", "invalid" },
                                             { "error", QJsonObject{ { "message", "Workflow configuration is invalid" } } } };
    EXPECT_EQ( "Workflow configuration is invalid", RimWorkflowDescribeTools::errorFromDescribe( invalidWithoutDetails ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowDescribeTools, resinsightTypeFromPythonType )
{
    EXPECT_EQ( "EclipseCase", RimWorkflowDescribeTools::resinsightTypeFromPythonType( "rips.generated.generated_classes.Reservoir" ) );
    EXPECT_EQ( "EclipseCase", RimWorkflowDescribeTools::resinsightTypeFromPythonType( "rips.case.Case" ) );
    EXPECT_EQ( "WellPath", RimWorkflowDescribeTools::resinsightTypeFromPythonType( "rips.generated.generated_classes.WellPath" ) );
    EXPECT_EQ( "View", RimWorkflowDescribeTools::resinsightTypeFromPythonType( "rips.generated.generated_classes.EclipseView" ) );
    EXPECT_TRUE( RimWorkflowDescribeTools::resinsightTypeFromPythonType( "rips.instance.Instance" ).isEmpty() );
    EXPECT_TRUE( RimWorkflowDescribeTools::resinsightTypeFromPythonType( "mypackage.WellPath" ).isEmpty() );
    EXPECT_TRUE( RimWorkflowDescribeTools::resinsightTypeFromPythonType( "WellPath" ).isEmpty() );
}
