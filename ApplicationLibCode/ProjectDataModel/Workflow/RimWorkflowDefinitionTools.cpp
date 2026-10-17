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

#include "RimWorkflowDefinitionTools.h"

#include "RimWorkflowPortCompatibility.h"
#include "RimWorkflowSchemaTools.h"
#include "RimWorkflowTaskCatalog.h"

#include <QJsonArray>
#include <QMap>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>

namespace
{
const QString explicitConfigKey = "x_ri_explicit_config_fields";

QStringList stringList( const QJsonValue& value )
{
    QStringList result;
    for ( const QJsonValue& item : value.toArray() )
    {
        if ( item.isString() ) result.append( item.toString() );
    }
    return result;
}

QJsonValue nullIfEmpty( const QString& text )
{
    return text.isEmpty() ? QJsonValue( QJsonValue::Null ) : QJsonValue( text );
}

RimWorkflowDefinitionNode nodeFromJson( const QJsonObject& json )
{
    RimWorkflowDefinitionNode node;
    node.name                 = json.value( "name" ).toString();
    node.taskId               = json.value( "task" ).toString();
    node.configFields         = stringList( json.value( "config_fields" ) );
    node.explicitConfigFields = json.contains( explicitConfigKey ) ? stringList( json.value( explicitConfigKey ) ) : node.configFields;
    if ( json.value( "map" ).isObject() )
    {
        const QJsonObject map = json.value( "map" ).toObject();
        node.map              = RimWorkflowTaskMap{ .over      = map.value( "over" ).toString(),
                                                    .keyAs     = map.value( "key_as" ).toString(),
                                                    .valueAs   = map.value( "value_as" ).toString(),
                                                    .errorMode = map.value( "error_mode" ).toString( "fail_fast" ) };
    }
    for ( auto it = json.begin(); it != json.end(); ++it )
    {
        if ( it.key() != "name" && it.key() != "task" && it.key() != "config_fields" && it.key() != explicitConfigKey && it.key() != "map" )
            node.extra.insert( it.key(), it.value() );
    }
    return node;
}

QJsonObject mapToJson( const RimWorkflowTaskMap& map )
{
    return QJsonObject{ { "over", map.over }, { "key_as", map.keyAs }, { "value_as", map.valueAs }, { "error_mode", map.errorMode } };
}

QString collectName( RimWorkflowDefinitionEdge::Collect collect )
{
    switch ( collect )
    {
        case RimWorkflowDefinitionEdge::Collect::List:
            return "list";
        case RimWorkflowDefinitionEdge::Collect::Dict:
            return "dict";
        default:
            return {};
    }
}

RimWorkflowDefinitionEdge edgeFromJson( const QJsonObject& json )
{
    RimWorkflowDefinitionEdge edge{ .from   = json.value( "from" ).toString(),
                                    .output = json.value( "output" ).toString(),
                                    .to     = json.value( "to" ).toString(),
                                    .input  = json.value( "input" ).toString() };
    const QString             collect = json.value( "collect" ).toString();
    if ( collect == "list" ) edge.collect = RimWorkflowDefinitionEdge::Collect::List;
    if ( collect == "dict" )
    {
        edge.collect = RimWorkflowDefinitionEdge::Collect::Dict;
        edge.key     = json.value( "key" ).toString();
    }
    return edge;
}

QJsonObject nodeToJson( const RimWorkflowDefinitionNode& node )
{
    QJsonObject json        = node.extra;
    json["name"]            = node.name;
    json["task"]            = node.taskId;
    json["config_fields"]   = QJsonArray::fromStringList( node.configFields );
    json[explicitConfigKey] = QJsonArray::fromStringList( node.explicitConfigFields );
    if ( node.map ) json["map"] = mapToJson( *node.map );
    return json;
}

QJsonObject edgeToJson( const RimWorkflowDefinitionEdge& edge )
{
    QJsonObject json{ { "from", edge.from }, { "output", nullIfEmpty( edge.output ) }, { "to", edge.to }, { "input", nullIfEmpty( edge.input ) } };
    if ( edge.isCollected() )
    {
        json["collect"] = collectName( edge.collect );
        json["key"] = edge.collect == RimWorkflowDefinitionEdge::Collect::Dict ? nullIfEmpty( edge.key ) : QJsonValue( QJsonValue::Null );
    }
    return json;
}

QJsonObject issueToJson( const RimWorkflowIssue& issue )
{
    return QJsonObject{ { "task", issue.task },
                        { "field", issue.field },
                        { "severity", issue.severity == RimWorkflowIssue::Severity::Error ? "error" : "warning" },
                        { "message", issue.message } };
}

void addIssue( std::vector<RimWorkflowIssue>& issues, const QString& task, const QString& field, const QString& message, bool isError = true )
{
    RimWorkflowIssue issue{ .task     = task,
                            .field    = field,
                            .severity = isError ? RimWorkflowIssue::Severity::Error : RimWorkflowIssue::Severity::Warning,
                            .message  = message };
    if ( std::find( issues.begin(), issues.end(), issue ) == issues.end() ) issues.push_back( issue );
}

bool hasCycle( const RimWorkflowDefinition& definition )
{
    QMap<QString, int> inDegree;
    for ( const auto& node : definition.nodes )
        inDegree[node.name] = 0;
    for ( const auto& edge : definition.edges )
    {
        if ( inDegree.contains( edge.to ) && inDegree.contains( edge.from ) ) inDegree[edge.to]++;
    }

    QStringList ready;
    for ( auto it = inDegree.begin(); it != inDegree.end(); ++it )
    {
        if ( it.value() == 0 ) ready.append( it.key() );
    }
    int visited = 0;
    while ( !ready.isEmpty() )
    {
        const QString current = ready.takeLast();
        ++visited;
        for ( const auto& edge : definition.edges )
        {
            if ( edge.from != current || !inDegree.contains( edge.to ) ) continue;
            if ( --inDegree[edge.to] == 0 ) ready.append( edge.to );
        }
    }
    return visited != inDegree.size();
}

void validateConnections( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionNode& node, std::vector<RimWorkflowIssue>& issues )
{
    const auto                 incoming   = definition.incomingEdges( node.name );
    int                        wholeCount = 0;
    QSet<QString>              fieldInputs;
    QMap<QString, QString>     collectKinds;
    QMap<QString, QStringList> collectKeys;
    for ( const auto& edge : incoming )
    {
        if ( edge.isWholeInput() )
        {
            ++wholeCount;
            continue;
        }
        if ( edge.isCollected() )
        {
            const QString kind = collectName( edge.collect );
            if ( fieldInputs.contains( edge.input ) && !collectKinds.contains( edge.input ) )
                addIssue( issues,
                          node.name,
                          edge.input,
                          QString( "Input '%1' has both a direct connection and collected members" ).arg( edge.input ) );
            if ( collectKinds.contains( edge.input ) && collectKinds.value( edge.input ) != kind )
                addIssue( issues, node.name, edge.input, QString( "Input '%1' mixes list and keyed members" ).arg( edge.input ) );
            collectKinds.insert( edge.input, kind );
            if ( edge.collect == RimWorkflowDefinitionEdge::Collect::Dict )
            {
                if ( edge.key.isEmpty() )
                    addIssue( issues, node.name, edge.input, QString( "A member of input '%1' has no key" ).arg( edge.input ) );
                else if ( collectKeys[edge.input].contains( edge.key ) )
                    addIssue( issues, node.name, edge.input, QString( "Duplicate key '%1' in input '%2'" ).arg( edge.key, edge.input ) );
                collectKeys[edge.input].append( edge.key );
            }
            fieldInputs.insert( edge.input );
            continue;
        }
        if ( collectKinds.contains( edge.input ) )
            addIssue( issues, node.name, edge.input, QString( "Input '%1' has both a direct connection and collected members" ).arg( edge.input ) );
        else if ( fieldInputs.contains( edge.input ) )
            addIssue( issues, node.name, edge.input, QString( "Input '%1' has more than one connection" ).arg( edge.input ) );
        fieldInputs.insert( edge.input );
    }
    if ( wholeCount > 1 ) addIssue( issues, node.name, {}, "The whole input has more than one connection" );
    if ( wholeCount > 0 && !fieldInputs.isEmpty() )
        addIssue( issues, node.name, {}, "The task takes both its whole input and individual input fields from other tasks" );
    if ( wholeCount > 1 || ( wholeCount > 0 && !fieldInputs.isEmpty() ) ) return;

    for ( const auto& edge : incoming )
    {
        if ( !definition.findNode( edge.from ) ) continue;
        if ( edge.isCollected() )
        {
            auto check = RimWorkflowPortCompatibility::checkCollectMember( definition, edge );
            if ( !check ) addIssue( issues, node.name, edge.input, check.error() );
            continue;
        }
        RimWorkflowDefinition others = definition;
        std::erase( others.edges, edge );
        auto check = RimWorkflowPortCompatibility::resolveConnection( others, edge.from, edge.output, edge.to, edge.input );
        if ( !check )
            addIssue( issues, node.name, edge.input, check.error() );
        else if ( check->isCollected() )
            addIssue( issues,
                      node.name,
                      edge.input,
                      QString( "%1 is a single member of input '%2'; it must be collected" ).arg( edge.from, edge.input ) );
    }
}

void validateMap( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionNode& node, std::vector<RimWorkflowIssue>& issues )
{
    if ( !node.map ) return;
    auto check = RimWorkflowDefinitionTools::checkTaskMap( definition, node.name, *node.map );
    if ( !check ) addIssue( issues, node.name, node.map->over, check.error() );
}

void validateInputs( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionNode& node, std::vector<RimWorkflowIssue>& issues )
{
    const QJsonObject taskType = definition.taskTypeForNode( node.name );
    const QStringList covered  = RimWorkflowDefinitionTools::coveredInputFields( definition, node.name );
    const auto        incoming = definition.incomingEdges( node.name );

    bool hasFieldIssue = false;
    for ( const auto& port : RimWorkflowPortCompatibility::inputPorts( taskType ) )
    {
        if ( port.isWhole() || !port.required || covered.contains( port.name ) || node.configFields.contains( port.name ) ) continue;
        const QString message = port.configurable ? QString( "Input '%1' is required; connect or configure it" ).arg( port.name )
                                                  : QString( "Input '%1' (%2) must be connected" ).arg( port.name, port.typeName );
        addIssue( issues, node.name, port.name, message );
        hasFieldIssue = true;
    }

    const QJsonObject inputSchema = taskType.value( "input_schema" ).toObject();
    const bool        hasFields   = !inputSchema.value( "properties" ).toObject().isEmpty();
    if ( incoming.empty() && hasFields && node.configFields.isEmpty() && !hasFieldIssue && !node.map )
    {
        const QString message =
            RimWorkflowPortCompatibility::inputPorts( taskType ).size() > 1
                ? QString( "No inputs are connected or configured; configure an optional input or connect the task" )
                : QString( "The input (%1) must be connected" ).arg( RimWorkflowPortCompatibility::typeName( inputSchema, inputSchema ) );
        addIssue( issues, node.name, {}, message );
    }

    const QJsonObject properties = inputSchema.value( "properties" ).toObject();
    for ( const QString& field : node.configFields )
    {
        if ( !properties.contains( field ) )
            addIssue( issues, node.name, field, QString( "Configured field '%1' is not an input of the task" ).arg( field ) );
    }
}

RimWorkflowDefinition withoutInvalidReferences( RimWorkflowDefinition definition )
{
    const QStringList names = definition.nodeNames();
    std::erase_if( definition.edges,
                   [&names]( const RimWorkflowDefinitionEdge& edge ) { return !names.contains( edge.from ) || !names.contains( edge.to ); } );
    if ( !definition.resultTask.isEmpty() && !names.contains( definition.resultTask ) ) definition.resultTask.clear();

    QJsonObject inputs;
    for ( const QString& name : names )
        inputs.insert( name, definition.inputs.value( name ).toObject() );
    definition.inputs = inputs;
    return definition;
}

QJsonArray inputPortsJson( const RimWorkflowDefinition&                  definition,
                           const RimWorkflowDefinitionNode&              node,
                           const std::vector<RimWorkflowPort>&           ports,
                           const std::vector<RimWorkflowDefinitionEdge>& incoming )
{
    const QStringList covered = RimWorkflowDefinitionTools::coveredInputFields( definition, node.name );
    QJsonArray        result;
    for ( const auto& port : ports )
    {
        bool wired = false;
        for ( const auto& edge : incoming )
            wired = wired || edge.input == port.name;
        QJsonObject json{ { "name", port.name },
                          { "type", port.typeName },
                          { "icon", port.iconResource },
                          { "description", port.description },
                          { "required", port.required },
                          { "configurable", port.configurable },
                          { "configured", !port.isWhole() && node.configFields.contains( port.name ) },
                          { "covered", !port.isWhole() && covered.contains( port.name ) },
                          { "wired", wired } };
        if ( const auto target = RimWorkflowPortCompatibility::collectTarget( port ) ) json["collect"] = collectName( target->first );
        result.append( json );
    }
    return result;
}

QJsonArray outputPortsJson( const std::vector<RimWorkflowPort>& ports, const std::vector<RimWorkflowDefinitionEdge>& outgoing )
{
    QJsonArray result;
    for ( const auto& port : ports )
    {
        bool wired = false;
        for ( const auto& edge : outgoing )
            wired = wired || edge.output == port.name;
        result.append( QJsonObject{ { "name", port.name },
                                    { "type", port.typeName },
                                    { "icon", port.iconResource },
                                    { "description", port.description },
                                    { "wired", wired } } );
    }
    return result;
}

QJsonObject graphTask( const RimWorkflowDefinition&         definition,
                       const RimWorkflowDefinitionNode&     node,
                       const QString&                       resultTask,
                       const std::vector<RimWorkflowIssue>& issues )
{
    const QJsonObject taskType     = definition.taskTypeForNode( node.name );
    const QJsonObject inputSchema  = taskType.value( "input_schema" ).toObject();
    const auto        inputPorts   = RimWorkflowPortCompatibility::inputPorts( taskType );
    const auto        outputPorts  = RimWorkflowPortCompatibility::outputPorts( taskType );
    const QStringList required     = RimWorkflowSchemaTools::requiredFields( inputSchema );
    const QJsonObject configValues = definition.inputs.value( node.name ).toObject();

    QJsonArray inputs;
    for ( const auto& port : inputPorts )
    {
        if ( !port.isWhole() ) inputs.append( port.name );
    }
    QJsonArray outputs;
    for ( const auto& port : outputPorts )
    {
        if ( !port.isWhole() ) outputs.append( port.name );
    }
    QJsonArray configFields;
    for ( const QString& field : node.configFields )
        configFields.append( RimWorkflowSchemaTools::configFieldSchema( field, inputSchema, required, configValues ) );

    // The mapping a mapped task runs over is a config value, shown as an input of the task
    QJsonArray  inputPortList = inputPortsJson( definition, node, inputPorts, definition.incomingEdges( node.name ) );
    QJsonObject inputTypes    = RimWorkflowPortCompatibility::portTypes( inputPorts );
    QJsonObject inputIcons    = RimWorkflowPortCompatibility::portIcons( inputPorts );
    QJsonObject mapJson;
    if ( node.map )
    {
        const QJsonObject overSchema = RimWorkflowDefinitionTools::mapOverSchema( definition, node.name );
        configFields.append( overSchema );
        mapJson                    = mapToJson( *node.map );
        mapJson["type"]            = overSchema.value( "type_name" ).toString();
        inputTypes[node.map->over] = overSchema.value( "type_name" ).toString();
        if ( overSchema.contains( "icon" ) ) inputIcons[node.map->over] = overSchema.value( "icon" ).toString();
        inputPortList.prepend( QJsonObject{ { "name", node.map->over },
                                            { "type", overSchema.value( "type_name" ).toString() },
                                            { "icon", overSchema.value( "icon" ).toString() },
                                            { "description", overSchema.value( "description" ).toString() },
                                            { "required", true },
                                            { "configurable", true },
                                            { "configured", true },
                                            { "covered", false },
                                            { "wired", false },
                                            { "map_over", true } } );
    }

    QJsonArray taskIssues;
    for ( const auto& issue : issues )
    {
        if ( issue.task == node.name ) taskIssues.append( issueToJson( issue ) );
    }

    return QJsonObject{ { "name", node.name },
                        { "task_id", node.taskId },
                        { "task_name", taskType.value( "name" ).toString() },
                        { "description", taskType.value( "description" ).toString() },
                        { "known_type", !taskType.isEmpty() },
                        { "inputs", inputs },
                        { "outputs", outputs },
                        { "config_fields", configFields },
                        { "accepts_input", !inputSchema.value( "properties" ).toObject().isEmpty() },
                        { "whole_input", inputPorts.size() == 1 && !inputSchema.value( "properties" ).toObject().isEmpty() },
                        { "input_ports", inputPortList },
                        { "output_ports", outputPortsJson( outputPorts, definition.outgoingEdges( node.name ) ) },
                        { "input_types", inputTypes },
                        { "output_types", RimWorkflowPortCompatibility::portTypes( outputPorts ) },
                        { "input_icons", inputIcons },
                        { "output_icons", RimWorkflowPortCompatibility::portIcons( outputPorts ) },
                        { "map", mapJson.isEmpty() ? QJsonValue( QJsonValue::Null ) : QJsonValue( mapJson ) },
                        { "is_result", node.name == resultTask },
                        { "issues", taskIssues } };
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RimWorkflowDefinition, QString> RimWorkflowDefinitionTools::fromJson( const QJsonObject& json )
{
    if ( json.value( "status" ).toString() == "invalid" )
    {
        const QString message = json.value( "error" ).toObject().value( "message" ).toString();
        return std::unexpected( message.isEmpty() ? QString( "Invalid workflow definition" ) : message );
    }
    if ( json.contains( "format" ) && json.value( "format" ).toInt() != 1 )
        return std::unexpected( QString( "Unsupported workflow definition format %1" ).arg( json.value( "format" ).toInt() ) );
    if ( !json.value( "nodes" ).isArray() ) return std::unexpected( "The workflow definition has no task list" );

    RimWorkflowDefinition definition;
    definition.name            = json.value( "name" ).toString();
    definition.resultTask      = json.value( "result_task" ).toString();
    definition.headerComment   = json.value( "header_comment" ).toString();
    definition.passthrough     = json.value( "passthrough" ).toObject();
    definition.source          = json.value( "source" ).toObject();
    definition.inputs          = json.value( "inputs" ).toObject();
    definition.taskTypes       = json.value( "task_types" ).toObject();
    definition.editable        = json.value( "editable" ).toBool( true );
    definition.readOnlyReasons = stringList( json.value( "readonly_reasons" ) );
    definition.warnings        = stringList( json.value( "warnings" ) );
    definition.describe        = json.value( "describe" ).toObject();
    definition.describeError   = json.value( "describe_error" ).toObject();

    for ( const QJsonValue& value : json.value( "nodes" ).toArray() )
    {
        auto node = nodeFromJson( value.toObject() );
        if ( node.name.isEmpty() || node.taskId.isEmpty() )
            return std::unexpected( "A task in the workflow definition has no name or type" );
        definition.nodes.push_back( node );
    }

    for ( const QJsonValue& value : json.value( "edges" ).toArray() )
        definition.edges.push_back( edgeFromJson( value.toObject() ) );
    return definition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowDefinitionTools::toJson( const RimWorkflowDefinition& definition )
{
    QJsonArray nodes;
    for ( const auto& node : definition.nodes )
        nodes.append( nodeToJson( node ) );
    QJsonArray edges;
    for ( const auto& edge : definition.edges )
        edges.append( edgeToJson( edge ) );

    return QJsonObject{ { "format", definition.format },
                        { "name", definition.name },
                        { "result_task", nullIfEmpty( definition.resultTask ) },
                        { "editable", definition.editable },
                        { "readonly_reasons", QJsonArray::fromStringList( definition.readOnlyReasons ) },
                        { "warnings", QJsonArray::fromStringList( definition.warnings ) },
                        { "source", definition.source },
                        { "header_comment", definition.headerComment },
                        { "passthrough", definition.passthrough },
                        { "nodes", nodes },
                        { "edges", edges },
                        { "inputs", definition.inputs },
                        { "task_types", definition.taskTypes },
                        { "describe", definition.describe.isEmpty() ? QJsonValue( QJsonValue::Null ) : QJsonValue( definition.describe ) },
                        { "describe_error",
                          definition.describeError.isEmpty() ? QJsonValue( QJsonValue::Null ) : QJsonValue( definition.describeError ) } };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinition RimWorkflowDefinitionTools::createEmpty( const QString& name )
{
    RimWorkflowDefinition definition;
    definition.name          = name;
    definition.headerComment = "# Created in ResInsight";
    return definition;
}

//--------------------------------------------------------------------------------------------------
/// A copy that is saved as a new workflow. The source folder is kept for workflow-local modules.
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinition RimWorkflowDefinitionTools::editableCopy( const RimWorkflowDefinition& definition, const QString& name )
{
    RimWorkflowDefinition copy = definition;
    copy.name                  = name;
    copy.describe              = {};
    copy.describeError         = {};
    copy.warnings.clear();
    if ( copy.headerComment.isEmpty() ) copy.headerComment = QString( "# Copy of %1, created in ResInsight" ).arg( definition.name );
    return copy;
}

//--------------------------------------------------------------------------------------------------
/// Parts of the workflow the editor does not model (nested workflows). A copy of such a
/// workflow stays read-only.
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowDefinitionTools::structuralReadOnlyReasons( const RimWorkflowDefinition& definition )
{
    QStringList reasons;
    for ( const auto& node : definition.nodes )
    {
        if ( !node.extra.isEmpty() )
            reasons << QString( "Task '%1' uses '%2', which the editor does not support" ).arg( node.name, node.extra.keys().join( "', '" ) );
    }
    return reasons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowDefinitionTools::isIdentifier( const QString& name )
{
    static const QRegularExpression pattern( "^[A-Za-z_][A-Za-z0-9_]*$" );
    return pattern.match( name ).hasMatch();
}

//--------------------------------------------------------------------------------------------------
/// Turn arbitrary text (a task id, a folder name) into a Python identifier
//--------------------------------------------------------------------------------------------------
QString RimWorkflowDefinitionTools::identifierFrom( const QString& text )
{
    QString result;
    for ( const QChar c : text )
    {
        const bool valid = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || c == '_';
        result += valid ? c : QChar( '_' );
    }
    if ( result.isEmpty() || result.front().isDigit() ) result.prepend( '_' );
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowDefinitionTools::uniqueInstanceName( const RimWorkflowDefinition& definition, const QString& baseName )
{
    const QString base = identifierFrom( baseName );
    if ( !definition.findNode( base ) ) return base;
    for ( int index = 2;; ++index )
    {
        const QString candidate = QString( "%1_%2" ).arg( base ).arg( index );
        if ( !definition.findNode( candidate ) ) return candidate;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString>
    RimWorkflowDefinitionTools::validateInstanceName( const RimWorkflowDefinition& definition, const QString& name, const QString& currentName )
{
    if ( !isIdentifier( name ) )
        return std::unexpected( QString( "'%1' is not a valid task name; use letters, digits and underscores" ).arg( name ) );
    if ( name != currentName && definition.findNode( name ) )
        return std::unexpected( QString( "A task named '%1' already exists" ).arg( name ) );
    for ( const auto& node : definition.nodes )
    {
        if ( node.taskId == name ) return std::unexpected( QString( "'%1' is the id of a task type" ).arg( name ) );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Tasks without downstream tasks, in definition order
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowDefinitionTools::sinkTasks( const RimWorkflowDefinition& definition )
{
    QStringList sinks;
    for ( const auto& node : definition.nodes )
    {
        if ( definition.outgoingEdges( node.name ).empty() ) sinks.append( node.name );
    }
    return sinks;
}

//--------------------------------------------------------------------------------------------------
/// The explicit result task, or the only sink. Empty when ambiguous.
//--------------------------------------------------------------------------------------------------
QString RimWorkflowDefinitionTools::effectiveResultTask( const RimWorkflowDefinition& definition )
{
    if ( !definition.resultTask.isEmpty() ) return definition.resultTask;
    const QStringList sinks = sinkTasks( definition );
    return sinks.size() == 1 ? sinks.front() : QString();
}

//--------------------------------------------------------------------------------------------------
/// Input fields that are supplied by connections
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowDefinitionTools::coveredInputFields( const RimWorkflowDefinition& definition, const QString& nodeName )
{
    const QJsonObject taskType = definition.taskTypeForNode( nodeName );
    QStringList       covered;
    for ( const auto& edge : definition.incomingEdges( nodeName ) )
    {
        if ( !edge.isWholeInput() )
        {
            covered.append( edge.input );
            continue;
        }

        // A routed output field becomes the whole input value
        if ( !edge.isWholeOutput() )
        {
            covered.append( RimWorkflowSchemaTools::propertyNames( taskType.value( "input_schema" ).toObject() ) );
            continue;
        }
        covered.append( RimWorkflowPortCompatibility::coveredByWholeOutput( definition.taskTypeForNode( edge.from ), taskType ) );
    }
    covered.removeDuplicates();
    return covered;
}

//--------------------------------------------------------------------------------------------------
/// Config fields are the inputs that are not connected: required ones that can be entered in
/// ResInsight, and those the user explicitly configured. A routed output field allows no config.
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowDefinitionTools::deriveConfigFields( const RimWorkflowDefinition& definition, const QString& nodeName )
{
    const auto* node = definition.findNode( nodeName );
    if ( !node ) return {};
    const QJsonObject taskType = definition.taskTypeForNode( nodeName );
    if ( taskType.isEmpty() ) return node->configFields;

    for ( const auto& edge : definition.incomingEdges( nodeName ) )
    {
        if ( edge.isWholeInput() && !edge.isWholeOutput() ) return {};
    }

    const QJsonObject inputSchema = taskType.value( "input_schema" ).toObject();
    const QJsonObject properties  = inputSchema.value( "properties" ).toObject();
    const QStringList required    = RimWorkflowSchemaTools::requiredFields( inputSchema );
    const QStringList covered     = coveredInputFields( definition, nodeName );

    QStringList derived;
    for ( const QString& field : RimWorkflowSchemaTools::propertyNames( inputSchema ) )
    {
        if ( covered.contains( field ) || ( node->map && field == node->map->over ) ) continue;
        const bool configurable = RimWorkflowSchemaTools::isConfigurable( properties.value( field ).toObject(), inputSchema );
        if ( node->explicitConfigFields.contains( field ) || ( required.contains( field ) && configurable ) ) derived.append( field );
    }

    // Keep the existing order and append new fields
    QStringList result;
    for ( const QString& field : node->configFields )
    {
        if ( derived.contains( field ) ) result.append( field );
    }
    for ( const QString& field : derived )
    {
        if ( !result.contains( field ) ) result.append( field );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/// Remove dangling references and derive the config fields of every task
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinition RimWorkflowDefinitionTools::normalize( RimWorkflowDefinition definition )
{
    if ( !definition.editable ) return definition;

    definition = withoutInvalidReferences( definition );

    std::vector<QStringList> configFields;
    for ( const auto& node : definition.nodes )
        configFields.push_back( deriveConfigFields( definition, node.name ) );
    for ( size_t i = 0; i < definition.nodes.size(); ++i )
        definition.nodes[i].configFields = configFields[i];
    return definition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult RimWorkflowDefinitionTools::addTask( const RimWorkflowDefinition&  definition,
                                                                            const RimWorkflowTaskCatalog& catalog,
                                                                            const QString&                taskId,
                                                                            QString*                      addedName )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );

    QJsonObject descriptor = catalog.taskType( taskId );
    if ( descriptor.isEmpty() ) descriptor = definition.taskType( taskId );
    if ( descriptor.isEmpty() ) return std::unexpected( QString( "Unknown task type '%1'" ).arg( taskId ) );

    QString baseName = descriptor.value( "name" ).toString();
    if ( baseName.isEmpty() ) baseName = taskId.mid( taskId.lastIndexOf( '.' ) + 1 );

    RimWorkflowDefinition result = definition;
    const QString         name   = uniqueInstanceName( result, baseName );
    result.nodes.push_back( { .name = name, .taskId = taskId } );
    result.taskTypes.insert( taskId, descriptor );
    result.inputs.insert( name, QJsonObject() );
    result = autoWireResInsightInputs( result, catalog, name );

    if ( addedName ) *addedName = name;
    return normalize( result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult RimWorkflowDefinitionTools::removeTask( const RimWorkflowDefinition& definition, const QString& nodeName )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );
    if ( !definition.findNode( nodeName ) ) return std::unexpected( QString( "Unknown task '%1'" ).arg( nodeName ) );

    RimWorkflowDefinition result = definition;
    std::erase_if( result.nodes, [&nodeName]( const RimWorkflowDefinitionNode& node ) { return node.name == nodeName; } );
    return normalize( result );
}

//--------------------------------------------------------------------------------------------------
/// Add a connection. An existing connection to the same input is replaced, except when the new
/// connection is another member of a collected (list or dict) input.
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult RimWorkflowDefinitionTools::connect( const RimWorkflowDefinition&     definition,
                                                                            const RimWorkflowDefinitionEdge& edge )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );

    auto resolved = RimWorkflowPortCompatibility::resolveConnection( definition, edge.from, edge.output, edge.to, edge.input );
    if ( !resolved ) return std::unexpected( resolved.error() );

    RimWorkflowDefinition result = definition;
    std::erase_if( result.edges,
                   [&resolved]( const RimWorkflowDefinitionEdge& existing )
                   {
                       return existing.to == resolved->to && existing.input == resolved->input &&
                              ( !resolved->isCollected() || existing.collect != resolved->collect );
                   } );
    result.edges.push_back( *resolved );
    return normalize( result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult RimWorkflowDefinitionTools::disconnect( const RimWorkflowDefinition&     definition,
                                                                               const RimWorkflowDefinitionEdge& edge )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );

    RimWorkflowDefinition result = definition;
    if ( std::erase_if( result.edges, [&edge]( const RimWorkflowDefinitionEdge& existing ) { return existing.sameConnection( edge ); } ) == 0 )
        return std::unexpected( "The connection does not exist" );
    return normalize( result );
}

//--------------------------------------------------------------------------------------------------
/// Rename the key of a member of a `dict[str, T]` input
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult RimWorkflowDefinitionTools::setCollectKey( const RimWorkflowDefinition&     definition,
                                                                                  const RimWorkflowDefinitionEdge& edge,
                                                                                  const QString&                   key )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );
    if ( key.trimmed().isEmpty() ) return std::unexpected( "The key cannot be empty" );

    RimWorkflowDefinition result = definition;
    auto                  it =
        std::find_if( result.edges.begin(), result.edges.end(), [&edge]( const auto& existing ) { return existing.sameConnection( edge ); } );
    if ( it == result.edges.end() ) return std::unexpected( "The connection does not exist" );
    if ( it->collect != RimWorkflowDefinitionEdge::Collect::Dict ) return std::unexpected( "The connection is not a keyed member" );

    for ( const auto& other : result.edges )
    {
        if ( &other != &*it && other.to == it->to && other.input == it->input && other.key == key )
            return std::unexpected( QString( "Input '%1' already has a member with key '%2'" ).arg( it->input, key ) );
    }
    it->key = key;
    return result;
}

//--------------------------------------------------------------------------------------------------
/// Move a member of a `list[T]` input `delta` positions within the list
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult
    RimWorkflowDefinitionTools::moveCollectMember( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionEdge& edge, int delta )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );

    RimWorkflowDefinition result = definition;
    std::vector<size_t>   members;
    size_t                position = 0;
    bool                  found    = false;
    for ( size_t i = 0; i < result.edges.size(); ++i )
    {
        const auto& existing = result.edges[i];
        if ( existing.to != edge.to || existing.input != edge.input || !existing.isCollected() ) continue;
        if ( existing.sameConnection( edge ) )
        {
            position = members.size();
            found    = true;
        }
        members.push_back( i );
    }
    if ( !found ) return std::unexpected( "The connection is not a collected member" );

    const int target = static_cast<int>( position ) + delta;
    if ( target < 0 || target >= static_cast<int>( members.size() ) ) return std::unexpected( "The member cannot be moved further" );
    std::swap( result.edges[members[position]], result.edges[members[static_cast<size_t>( target )]] );
    return result;
}

//--------------------------------------------------------------------------------------------------
/// Checks that a map fits the task: the key and value fields are inputs of the task, and `over`
/// is a new config field name
//--------------------------------------------------------------------------------------------------
std::expected<void, QString>
    RimWorkflowDefinitionTools::checkTaskMap( const RimWorkflowDefinition& definition, const QString& nodeName, const RimWorkflowTaskMap& map )
{
    const auto* node = definition.findNode( nodeName );
    if ( !node ) return std::unexpected( QString( "Unknown task '%1'" ).arg( nodeName ) );

    if ( !isIdentifier( map.over ) ) return std::unexpected( QString( "'%1' is not a valid name for the mapping" ).arg( map.over ) );
    if ( map.keyAs.isEmpty() || map.valueAs.isEmpty() ) return std::unexpected( "Choose the inputs that receive the key and the value" );
    if ( map.keyAs == map.valueAs ) return std::unexpected( "The same input cannot receive both the key and the value" );
    if ( map.errorMode != "fail_fast" && map.errorMode != "collect_all" )
        return std::unexpected( QString( "Unknown error mode '%1'" ).arg( map.errorMode ) );

    const QJsonObject taskType = definition.taskType( node->taskId );
    if ( taskType.isEmpty() ) return {};

    const QJsonObject inputSchema = taskType.value( "input_schema" ).toObject();
    const QJsonObject properties  = inputSchema.value( "properties" ).toObject();
    for ( const QString& field : { map.keyAs, map.valueAs } )
    {
        if ( !properties.contains( field ) ) return std::unexpected( QString( "'%1' has no input '%2'" ).arg( nodeName, field ) );
    }
    if ( properties.contains( map.over ) )
        return std::unexpected( QString( "'%1' is an input of '%2'; choose another name for the mapping" ).arg( map.over, nodeName ) );

    const QJsonObject keySchema = properties.value( map.keyAs ).toObject();
    const QJsonObject strSchema{ { "type", "string" } };
    if ( !RimWorkflowPortCompatibility::isCompatible( strSchema, strSchema, keySchema, inputSchema ) )
        return std::unexpected( QString( "Input '%1' receives the key and must accept a string" ).arg( map.keyAs ) );
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Config field schema of the mapping a mapped task runs over: a `dict[str, Value]`, where the
/// value type is the type of the `value_as` input
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowDefinitionTools::mapOverSchema( const RimWorkflowDefinition& definition, const QString& nodeName )
{
    const auto* node = definition.findNode( nodeName );
    if ( !node || !node->map ) return {};

    const QJsonObject inputSchema  = definition.taskType( node->taskId ).value( "input_schema" ).toObject();
    const QJsonObject configValues = definition.inputs.value( nodeName ).toObject();

    QJsonObject entry{ { "name", node->map->over }, { "type", "object" }, { "required", true }, { "map_over", true } };
    if ( configValues.contains( node->map->over ) ) entry["default"] = configValues.value( node->map->over );

    QString valueType = "Any";
    if ( !node->map->valueAs.isEmpty() )
    {
        const QJsonObject value = RimWorkflowSchemaTools::configFieldSchema( node->map->valueAs, inputSchema, {}, {} );
        entry["value_type"]     = value.value( "type" );
        if ( value.contains( "resinsight_type" ) ) entry["value_resinsight_type"] = value.value( "resinsight_type" );
        if ( value.contains( "format" ) ) entry["value_format"] = value.value( "format" );

        const QJsonObject property = inputSchema.value( "properties" ).toObject().value( node->map->valueAs ).toObject();
        valueType                  = RimWorkflowPortCompatibility::typeName( property, inputSchema );
        const QString icon         = RimWorkflowPortCompatibility::iconResource( property, inputSchema );
        if ( !icon.isEmpty() ) entry["icon"] = icon;
    }
    entry["type_name"] = QString( "dict[str, %1]" ).arg( valueType );
    entry["description"] =
        QString( "The items '%1' runs for; each key is passed as '%2' and each value as '%3'" ).arg( nodeName, node->map->keyAs, node->map->valueAs );
    return entry;
}

//--------------------------------------------------------------------------------------------------
/// Run a task once per item of a mapping, or remove the map with std::nullopt
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult RimWorkflowDefinitionTools::setTaskMap( const RimWorkflowDefinition&             definition,
                                                                               const QString&                           nodeName,
                                                                               const std::optional<RimWorkflowTaskMap>& map )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );
    const auto* node = definition.findNode( nodeName );
    if ( !node ) return std::unexpected( QString( "Unknown task '%1'" ).arg( nodeName ) );

    RimWorkflowDefinition result = definition;
    auto*                 target = result.findNode( nodeName );
    QJsonObject           values = result.inputs.value( nodeName ).toObject();
    if ( node->map ) values.remove( node->map->over );

    if ( map )
    {
        if ( auto check = checkTaskMap( definition, nodeName, *map ); !check ) return std::unexpected( check.error() );
        for ( const auto& edge : definition.incomingEdges( nodeName ) )
        {
            if ( edge.isWholeInput() )
                return std::unexpected(
                    QString( "'%1' takes its whole input from '%2'; disconnect it before mapping" ).arg( nodeName, edge.from ) );
        }

        // The key and value inputs are filled by the map
        std::erase_if( result.edges,
                       [&]( const RimWorkflowDefinitionEdge& edge )
                       { return edge.to == nodeName && ( edge.input == map->keyAs || edge.input == map->valueAs ); } );
        target->explicitConfigFields.removeAll( map->keyAs );
        target->explicitConfigFields.removeAll( map->valueAs );
        values.remove( map->keyAs );
        values.remove( map->valueAs );
        if ( node->map && node->map->over != map->over && definition.inputs.value( nodeName ).toObject().contains( node->map->over ) )
            values[map->over] = definition.inputs.value( nodeName ).toObject().value( node->map->over );
    }
    target->map = map;
    result.inputs.insert( nodeName, values );

    // Downstream connections may no longer fit the changed output type
    result = normalize( result );
    return result;
}

//--------------------------------------------------------------------------------------------------
/// Rename a task instance, updating connections, the result task and the input values
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult
    RimWorkflowDefinitionTools::renameTask( const RimWorkflowDefinition& definition, const QString& oldName, const QString& newName )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );
    if ( !definition.findNode( oldName ) ) return std::unexpected( QString( "Unknown task '%1'" ).arg( oldName ) );
    if ( oldName == newName ) return definition;
    if ( auto valid = validateInstanceName( definition, newName, oldName ); !valid ) return std::unexpected( valid.error() );

    RimWorkflowDefinition result     = definition;
    result.findNode( oldName )->name = newName;
    for ( auto& edge : result.edges )
    {
        if ( edge.from == oldName ) edge.from = newName;
        if ( edge.to == oldName ) edge.to = newName;
    }
    if ( result.resultTask == oldName ) result.resultTask = newName;

    const QJsonValue values = result.inputs.take( oldName );
    result.inputs.insert( newName, values.isObject() ? values : QJsonObject() );
    return normalize( result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult RimWorkflowDefinitionTools::renameWorkflow( const RimWorkflowDefinition& definition,
                                                                                   const QString&               newName )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );
    if ( !isIdentifier( newName ) )
        return std::unexpected( QString( "'%1' is not a valid workflow name; use letters, digits and underscores" ).arg( newName ) );

    RimWorkflowDefinition result = definition;
    result.name                  = newName;
    return result;
}

//--------------------------------------------------------------------------------------------------
/// Set the explicit result task. An empty name clears it (the only sink is used).
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult RimWorkflowDefinitionTools::setResultTask( const RimWorkflowDefinition& definition,
                                                                                  const QString&               nodeName )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );
    if ( !nodeName.isEmpty() && !definition.findNode( nodeName ) ) return std::unexpected( QString( "Unknown task '%1'" ).arg( nodeName ) );

    RimWorkflowDefinition result = definition;
    result.resultTask            = nodeName;
    return result;
}

//--------------------------------------------------------------------------------------------------
/// Choose whether an optional input that is not connected is a config field
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult RimWorkflowDefinitionTools::setOptionalInputConfigured( const RimWorkflowDefinition& definition,
                                                                                               const QString&               nodeName,
                                                                                               const QString&               field,
                                                                                               bool                         configured )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );
    const auto* node = definition.findNode( nodeName );
    if ( !node ) return std::unexpected( QString( "Unknown task '%1'" ).arg( nodeName ) );

    const auto port = RimWorkflowPortCompatibility::inputPort( definition.taskTypeForNode( nodeName ), field );
    if ( !port || port->isWhole() ) return std::unexpected( QString( "'%1' has no input '%2'" ).arg( nodeName, field ) );
    if ( port->required ) return std::unexpected( QString( "Input '%1' is required and always configured unless connected" ).arg( field ) );

    RimWorkflowDefinition result = definition;
    QStringList&          fields = result.findNode( nodeName )->explicitConfigFields;
    fields.removeAll( field );
    if ( configured ) fields.append( field );
    return normalize( result );
}

//--------------------------------------------------------------------------------------------------
/// Connect inputs that take a ResInsight connection to a `resinsight.connect` task, which is
/// created if the workflow has none
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinition RimWorkflowDefinitionTools::autoWireResInsightInputs( const RimWorkflowDefinition&  definition,
                                                                            const RimWorkflowTaskCatalog& catalog,
                                                                            const QString&                nodeName )
{
    const auto* node = definition.findNode( nodeName );
    if ( !node || node->taskId == connectTaskId ) return definition;

    QJsonObject connectType = catalog.taskType( connectTaskId );
    if ( connectType.isEmpty() ) connectType = definition.taskType( connectTaskId );
    if ( connectType.isEmpty() ) return definition;

    const QJsonObject connectOutput = connectType.value( "output_schema" ).toObject();
    if ( !connectOutput.contains( "x-ri-python-type" ) ) return definition;
    const RimWorkflowPort produced{ .schema = connectOutput, .rootSchema = connectOutput };

    const auto  incoming = definition.incomingEdges( nodeName );
    QStringList wiredInputs;
    for ( const auto& edge : incoming )
        wiredInputs.append( edge.input );

    // Only inputs declared with the connection type; untyped (Any) inputs are left alone
    auto takesConnection = [&produced]( const RimWorkflowPort& port )
    {
        const QJsonObject resolved = RimWorkflowSchemaTools::resolveReferences( port.schema, port.rootSchema );
        return port.required && resolved.contains( "x-ri-python-type" ) && RimWorkflowPortCompatibility::isCompatible( produced, port );
    };

    QStringList inputsToWire;
    const auto  ports = RimWorkflowPortCompatibility::inputPorts( definition.taskTypeForNode( nodeName ) );
    for ( const auto& port : ports )
    {
        if ( port.isWhole() || wiredInputs.contains( port.name ) || !takesConnection( port ) ) continue;
        inputsToWire.append( port.name );
    }
    const bool wireWhole = inputsToWire.isEmpty() && incoming.empty() && ports.size() == 1 && takesConnection( ports.front() );
    if ( inputsToWire.isEmpty() && !wireWhole ) return definition;

    RimWorkflowDefinition result = definition;
    QString               connectName;
    for ( const auto& candidate : result.nodes )
    {
        if ( candidate.taskId == connectTaskId )
        {
            connectName = candidate.name;
            break;
        }
    }
    if ( connectName.isEmpty() )
    {
        connectName = uniqueInstanceName( result, connectType.value( "name" ).toString( "connect_to_resinsight" ) );
        result.nodes.insert( result.nodes.begin(), { .name = connectName, .taskId = connectTaskId } );
        result.taskTypes.insert( connectTaskId, connectType );
        result.inputs.insert( connectName, QJsonObject() );
    }

    if ( wireWhole ) result.edges.push_back( { .from = connectName, .to = nodeName } );
    for ( const QString& input : inputsToWire )
        result.edges.push_back( { .from = connectName, .to = nodeName, .input = input } );
    return result;
}

//--------------------------------------------------------------------------------------------------
/// Static checks that do not need Python. `taskmaestro workflow describe` has the final say.
//--------------------------------------------------------------------------------------------------
std::vector<RimWorkflowIssue> RimWorkflowDefinitionTools::validate( const RimWorkflowDefinition& definition )
{
    std::vector<RimWorkflowIssue> issues;
    if ( !isIdentifier( definition.name ) )
        addIssue( issues, {}, {}, QString( "'%1' is not a valid workflow name" ).arg( definition.name ) );
    if ( definition.nodes.empty() )
    {
        addIssue( issues, {}, {}, "The workflow has no tasks" );
        return issues;
    }

    QSet<QString> names;
    for ( const auto& node : definition.nodes )
    {
        if ( !isIdentifier( node.name ) ) addIssue( issues, node.name, {}, QString( "'%1' is not a valid task name" ).arg( node.name ) );
        if ( names.contains( node.name ) ) addIssue( issues, node.name, {}, QString( "Duplicate task name '%1'" ).arg( node.name ) );
        names.insert( node.name );
    }

    for ( const auto& edge : definition.edges )
    {
        if ( !names.contains( edge.from ) || !names.contains( edge.to ) )
            addIssue( issues, edge.to, edge.input, QString( "Connection from unknown task '%1'" ).arg( edge.from ) );
    }
    if ( hasCycle( definition ) ) addIssue( issues, {}, {}, "The workflow contains a cycle" );

    for ( const auto& node : definition.nodes )
    {
        if ( definition.taskType( node.taskId ).isEmpty() )
        {
            addIssue( issues, node.name, {}, QString( "Task type '%1' is not installed" ).arg( node.taskId ) );
            continue;
        }
        validateConnections( definition, node, issues );
        validateInputs( definition, node, issues );
        validateMap( definition, node, issues );
    }

    if ( !definition.resultTask.isEmpty() && !names.contains( definition.resultTask ) )
    {
        addIssue( issues, {}, {}, QString( "The result task '%1' does not exist" ).arg( definition.resultTask ) );
    }
    else if ( definition.resultTask.isEmpty() )
    {
        const QStringList sinks = sinkTasks( definition );
        if ( sinks.size() > 1 )
            addIssue( issues,
                      {},
                      {},
                      QString( "The workflow has %1 end tasks (%2); set a result task" ).arg( sinks.size() ).arg( sinks.join( ", " ) ) );
    }
    return issues;
}

//--------------------------------------------------------------------------------------------------
/// The graph shown by RiuWorkflowGraphView, in the shape produced by graphFromDescribe() plus the
/// ports, the result task and the issues of every task
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowDefinitionTools::graphFromDefinition( const RimWorkflowDefinition&         definition,
                                                             bool                                 editMode,
                                                             const std::vector<RimWorkflowIssue>& extraIssues )
{
    std::vector<RimWorkflowIssue> issues = validate( definition );
    for ( const auto& issue : extraIssues )
    {
        if ( std::find( issues.begin(), issues.end(), issue ) == issues.end() ) issues.push_back( issue );
    }

    const QString resultTask = effectiveResultTask( definition );
    QJsonArray    tasks;
    for ( const auto& node : definition.nodes )
        tasks.append( graphTask( definition, node, resultTask, issues ) );

    // Position of each list member within its input
    QMap<const RimWorkflowDefinitionEdge*, int> listIndex;
    QMap<QString, int>                          listCount;
    for ( const auto& edge : definition.edges )
    {
        if ( edge.collect == RimWorkflowDefinitionEdge::Collect::List ) listIndex[&edge] = listCount[edge.to + "." + edge.input]++;
    }

    QJsonArray edges;
    for ( const auto& edge : definition.edges )
    {
        QJsonObject json{ { "from", edge.from }, { "to", edge.to } };
        if ( !edge.input.isEmpty() ) json["input"] = edge.input;
        if ( !edge.output.isEmpty() ) json["output"] = edge.output;
        if ( edge.isCollected() )
        {
            json["collect"] = collectName( edge.collect );
            json["label"] = edge.collect == RimWorkflowDefinitionEdge::Collect::Dict ? edge.key : QString( "[%1]" ).arg( listIndex[&edge] );
        }
        edges.append( json );
    }

    const QStringList nodeNames = definition.nodeNames();
    QJsonArray        workflowIssues;
    for ( const auto& issue : issues )
    {
        if ( !nodeNames.contains( issue.task ) ) workflowIssues.append( issueToJson( issue ) );
    }

    return QJsonObject{ { "name", definition.name },
                        { "description", definition.headerComment },
                        { "editable", editMode && definition.editable },
                        { "result_task", resultTask },
                        { "tasks", tasks },
                        { "edges", edges },
                        { "issues", workflowIssues } };
}
