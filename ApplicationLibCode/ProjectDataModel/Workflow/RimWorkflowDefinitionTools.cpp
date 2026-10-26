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
#include <QJsonDocument>
#include <QMap>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>
#include <cmath>

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

QJsonArray asArray( const QJsonValue& value )
{
    return value.isArray() ? value.toArray() : QJsonArray{ value };
}

std::optional<RimWorkflowTaskGuard> guardFromJson( const QJsonValue& value )
{
    if ( !value.isObject() ) return std::nullopt;
    const QJsonObject json = value.toObject();

    using Op = RimWorkflowTaskGuard::Op;
    RimWorkflowTaskGuard guard;
    if ( json.contains( "switch" ) )
    {
        guard.source = json.value( "switch" ).toString();
        if ( json.value( "otherwise" ).toBool() )
        {
            guard.op = Op::Otherwise;
            return guard;
        }
        guard.op         = Op::Case;
        guard.caseAsList = json.value( "case" ).isArray();
        // Without a case there are no branches, which validation reports
        if ( json.contains( "case" ) ) guard.values = asArray( json.value( "case" ) );
        return guard;
    }

    const QJsonValue ref = json.value( "ref" );
    if ( ref.isArray() )
    {
        const QJsonArray items = ref.toArray();
        guard.source           = items.at( 0 ).toString();
        guard.field            = items.at( 1 ).toString();
        guard.malformedRef     = items.size() != 2;
    }
    else
    {
        guard.source = ref.toString();
    }
    if ( json.contains( "truthy" ) )
    {
        guard.op = json.value( "truthy" ).toBool() ? Op::Truthy : Op::Falsy;
    }
    else if ( json.contains( "in" ) )
    {
        guard.op     = Op::In;
        guard.values = json.value( "in" ).toArray();
    }
    else
    {
        guard.op     = Op::Equals;
        guard.values = QJsonArray{ json.value( "equals" ) };
    }
    return guard;
}

QJsonObject guardToJson( const RimWorkflowTaskGuard& guard )
{
    using Op = RimWorkflowTaskGuard::Op;
    switch ( guard.op )
    {
        case Op::Otherwise:
            return QJsonObject{ { "switch", guard.source }, { "otherwise", true } };
        case Op::Case:
            return QJsonObject{ { "switch", guard.source },
                                { "case", guard.caseAsList || guard.values.size() != 1 ? QJsonValue( guard.values ) : guard.values.at( 0 ) } };
        default:
            break;
    }

    QJsonObject json{ { "ref", guard.isSwitchBranch() ? QJsonValue( guard.source ) : QJsonValue( QJsonArray{ guard.source, guard.field } ) } };
    if ( guard.op == Op::Truthy || guard.op == Op::Falsy )
        json["truthy"] = guard.op == Op::Truthy;
    else if ( guard.op == Op::In )
        json["in"] = guard.values;
    else
        json["equals"] = guard.values.isEmpty() ? QJsonValue( QJsonValue::Null ) : guard.values.at( 0 );
    return json;
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
    node.when = guardFromJson( json.value( "when" ) );
    for ( auto it = json.begin(); it != json.end(); ++it )
    {
        static const QStringList modeled{ "name", "task", "config_fields", explicitConfigKey, "map", "when" };
        if ( !modeled.contains( it.key() ) ) node.extra.insert( it.key(), it.value() );
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
        case RimWorkflowDefinitionEdge::Collect::FirstOf:
            return "first_of";
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
    if ( collect == "first_of" ) edge.collect = RimWorkflowDefinitionEdge::Collect::FirstOf;
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
    if ( node.when ) json["when"] = guardToJson( *node.when );
    return json;
}

QJsonObject edgeToJson( const RimWorkflowDefinitionEdge& edge )
{
    QJsonObject json{ { "from", edge.from }, { "output", nullIfEmpty( edge.output ) }, { "to", edge.to }, { "input", nullIfEmpty( edge.input ) } };
    if ( edge.collect != RimWorkflowDefinitionEdge::Collect::None )
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

// Connections and run conditions, as (upstream, downstream) pairs
std::vector<std::pair<QString, QString>> dependencyLinks( const RimWorkflowDefinition& definition )
{
    std::vector<std::pair<QString, QString>> links;
    for ( const auto& edge : definition.edges )
        links.emplace_back( edge.from, edge.to );
    for ( const auto& node : definition.nodes )
    {
        if ( node.when ) links.emplace_back( node.when->source, node.name );
    }
    return links;
}

bool hasCycle( const RimWorkflowDefinition& definition )
{
    const auto         links = dependencyLinks( definition );
    QMap<QString, int> inDegree;
    for ( const auto& node : definition.nodes )
        inDegree[node.name] = 0;
    for ( const auto& [from, to] : links )
    {
        if ( inDegree.contains( to ) && inDegree.contains( from ) ) inDegree[to]++;
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
        for ( const auto& [from, to] : links )
        {
            if ( from != current || !inDegree.contains( to ) ) continue;
            if ( --inDegree[to] == 0 ) ready.append( to );
        }
    }
    return visited != inDegree.size();
}

QString directAndMembersMessage( const QString& input, bool firstOf )
{
    return firstOf ? QString( "Input '%1' has both a direct connection and alternatives (first of)" ).arg( input )
                   : QString( "Input '%1' has both a direct connection and collected members" ).arg( input );
}

void validateConnections( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionNode& node, std::vector<RimWorkflowIssue>& issues )
{
    const auto                 incoming   = definition.incomingEdges( node.name );
    int                        wholeCount = 0;
    QSet<QString>              fieldInputs;
    QMap<QString, QString>     collectKinds;
    QMap<QString, QStringList> collectKeys;
    QMap<QString, int>         firstOfCounts;
    for ( const auto& edge : incoming )
    {
        if ( edge.isWholeInput() )
        {
            ++wholeCount;
            continue;
        }
        if ( edge.isCollected() || edge.isFirstOf() )
        {
            const QString kind = collectName( edge.collect );
            if ( fieldInputs.contains( edge.input ) && !collectKinds.contains( edge.input ) )
                addIssue( issues, node.name, edge.input, directAndMembersMessage( edge.input, edge.isFirstOf() ) );
            if ( collectKinds.contains( edge.input ) && collectKinds.value( edge.input ) != kind )
            {
                const bool firstOf = edge.isFirstOf() ||
                                     collectKinds.value( edge.input ) == collectName( RimWorkflowDefinitionEdge::Collect::FirstOf );
                const QString message = firstOf
                                            ? QString( "Input '%1' mixes alternatives (first of) with collected members" ).arg( edge.input )
                                            : QString( "Input '%1' mixes list and keyed members" ).arg( edge.input );
                addIssue( issues, node.name, edge.input, message );
            }
            collectKinds.insert( edge.input, kind );
            if ( edge.isFirstOf() ) firstOfCounts[edge.input]++;
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
            addIssue( issues,
                      node.name,
                      edge.input,
                      directAndMembersMessage( edge.input,
                                               collectKinds.value( edge.input ) == collectName( RimWorkflowDefinitionEdge::Collect::FirstOf ) ) );
        else if ( fieldInputs.contains( edge.input ) )
            addIssue( issues, node.name, edge.input, QString( "Input '%1' has more than one connection" ).arg( edge.input ) );
        fieldInputs.insert( edge.input );
    }
    for ( auto it = firstOfCounts.begin(); it != firstOfCounts.end(); ++it )
    {
        if ( it.value() < 2 )
            addIssue( issues,
                      node.name,
                      it.key(),
                      QString( "Input '%1' has a single alternative; first of needs at least two connections" ).arg( it.key() ) );
    }
    if ( wholeCount > 1 ) addIssue( issues, node.name, {}, "The whole input has more than one connection" );
    if ( wholeCount > 0 && !fieldInputs.isEmpty() )
        addIssue( issues, node.name, {}, "The task takes both its whole input and individual input fields from other tasks" );
    if ( wholeCount > 1 || ( wholeCount > 0 && !fieldInputs.isEmpty() ) ) return;

    for ( const auto& edge : incoming )
    {
        if ( !definition.findNode( edge.from ) ) continue;
        if ( edge.isCollected() || edge.isFirstOf() )
        {
            auto check = edge.isFirstOf() ? RimWorkflowPortCompatibility::checkFirstOfMember( definition, edge )
                                          : RimWorkflowPortCompatibility::checkCollectMember( definition, edge );
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

const QString branchField = "branch";

bool sameValue( const QJsonValue& a, const QJsonValue& b )
{
    if ( a.isDouble() && b.isDouble() ) return a.toDouble() == b.toDouble();
    return a == b;
}

bool containsValue( const QJsonArray& values, const QJsonValue& value )
{
    return std::any_of( values.begin(), values.end(), [&value]( const QJsonValue& item ) { return sameValue( item, value ); } );
}

bool labelIsTruthy( const QJsonValue& label )
{
    if ( label.isBool() ) return label.toBool();
    if ( label.isDouble() ) return label.toDouble() != 0.0;
    if ( label.isString() ) return !label.toString().isEmpty();
    return !label.isNull();
}

QJsonArray schemaOptions( const QJsonObject& schema, const QJsonObject& rootSchema )
{
    const QJsonObject resolved = RimWorkflowSchemaTools::resolveReferences( schema, rootSchema );
    QJsonArray        options;
    for ( const char* unionKey : { "anyOf", "oneOf" } )
    {
        for ( const QJsonValue& option : resolved.value( unionKey ).toArray() )
            options.append( RimWorkflowSchemaTools::resolveReferences( option.toObject(), rootSchema ) );
    }
    if ( options.isEmpty() ) options.append( resolved );
    return options;
}

// Mirrors taskmaestro: only bool, boolean literals and optional fields can be truthy or falsy
bool acceptsTruthiness( const QJsonObject& schema, const QJsonObject& rootSchema )
{
    const QJsonArray options = schemaOptions( schema, rootSchema );
    if ( options.size() > 1 )
        return std::any_of( options.begin(),
                            options.end(),
                            []( const QJsonValue& option ) { return option.toObject().value( "type" ) == "null"; } );

    const QJsonObject option = options.at( 0 ).toObject();
    if ( option.contains( "enum" ) )
    {
        const QJsonArray values = option.value( "enum" ).toArray();
        return std::all_of( values.begin(), values.end(), []( const QJsonValue& value ) { return value.isBool(); } );
    }
    if ( option.value( "type" ).isArray() ) return option.value( "type" ).toArray().contains( QJsonValue( "null" ) );
    return !option.contains( "type" ) || option.value( "type" ) == "boolean" || option.value( "type" ) == "null";
}

bool valueMatchesSchema( const QJsonValue& value, const QJsonObject& schema, const QJsonObject& rootSchema )
{
    for ( const QJsonValue& item : schemaOptions( schema, rootSchema ) )
    {
        const QJsonObject option = item.toObject();
        if ( option.contains( "enum" ) )
        {
            if ( containsValue( option.value( "enum" ).toArray(), value ) ) return true;
            continue;
        }
        if ( option.contains( "const" ) )
        {
            if ( sameValue( option.value( "const" ), value ) ) return true;
            continue;
        }

        QStringList types;
        if ( option.value( "type" ).isArray() )
            types = stringList( option.value( "type" ) );
        else if ( option.contains( "type" ) )
            types.append( option.value( "type" ).toString() );
        if ( types.isEmpty() ) return true;
        for ( const QString& type : types )
        {
            const bool matches = ( type == "string" && value.isString() ) || ( type == "boolean" && value.isBool() ) ||
                                 ( type == "number" && value.isDouble() ) ||
                                 ( type == "integer" && value.isDouble() && value.toDouble() == std::floor( value.toDouble() ) ) ||
                                 ( type == "null" && value.isNull() ) || type == "object" || type == "array";
            if ( matches ) return true;
        }
    }
    return false;
}

// Every branch of a switch needs a task, and `otherwise` must be able to match
void validateSwitchCoverage( const RimWorkflowDefinition&     definition,
                             const RimWorkflowDefinitionNode& switchNode,
                             std::vector<RimWorkflowIssue>&   issues )
{
    using Op                = RimWorkflowTaskGuard::Op;
    const QJsonArray labels = RimWorkflowDefinitionTools::switchBranches( definition.taskType( switchNode.taskId ) );
    if ( labels.isEmpty() || switchNode.map ) return;

    bool       guarded      = false;
    bool       hasOtherwise = false;
    QJsonArray claimed;
    for ( const auto& node : definition.nodes )
    {
        if ( !node.when || node.when->source != switchNode.name ) continue;
        if ( !node.when->isSwitchBranch() && node.when->field != branchField ) continue;
        guarded = true;
        switch ( node.when->op )
        {
            case Op::Otherwise:
                hasOtherwise = true;
                break;
            case Op::Truthy:
            case Op::Falsy:
                for ( const QJsonValue& label : labels )
                {
                    if ( labelIsTruthy( label ) == ( node.when->op == Op::Truthy ) ) claimed.append( label );
                }
                break;
            default:
                for ( const QJsonValue& value : node.when->values )
                    claimed.append( value );
        }
    }
    if ( !guarded ) return;

    QStringList unclaimed;
    for ( const QJsonValue& label : labels )
    {
        if ( !containsValue( claimed, label ) ) unclaimed.append( RimWorkflowDefinitionTools::guardValueText( label ) );
    }
    if ( !hasOtherwise && !unclaimed.isEmpty() )
    {
        addIssue( issues,
                  switchNode.name,
                  {},
                  QString( "Switch '%1' has no task for branch %2; add a run condition with this case, or otherwise" )
                      .arg( switchNode.name, unclaimed.join( ", " ) ) );
    }
    if ( hasOtherwise && unclaimed.isEmpty() )
    {
        addIssue( issues,
                  switchNode.name,
                  {},
                  QString( "Otherwise on switch '%1' can never match: every branch already has a case" ).arg( switchNode.name ) );
    }
}

void validateGuard( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionNode& node, std::vector<RimWorkflowIssue>& issues )
{
    if ( !node.when ) return;
    RimWorkflowDefinition others = definition;
    others.findNode( node.name )->when.reset();
    auto check = RimWorkflowDefinitionTools::checkTaskGuard( others, node.name, *node.when );
    if ( !check ) addIssue( issues, node.name, {}, check.error() );
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
    for ( auto& node : definition.nodes )
    {
        if ( node.when && !names.contains( node.when->source ) ) node.when.reset();
    }

    // A single remaining alternative becomes a plain connection
    QMap<QString, int> firstOfCounts;
    for ( const auto& edge : definition.edges )
    {
        if ( edge.isFirstOf() ) firstOfCounts[edge.to + "." + edge.input]++;
    }
    for ( auto& edge : definition.edges )
    {
        if ( edge.isFirstOf() && firstOfCounts.value( edge.to + "." + edge.input ) == 1 )
            edge.collect = RimWorkflowDefinitionEdge::Collect::None;
    }

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
        bool wired   = false;
        bool firstOf = false;
        for ( const auto& edge : incoming )
        {
            wired   = wired || edge.input == port.name;
            firstOf = firstOf || ( edge.input == port.name && edge.isFirstOf() );
        }
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
        if ( firstOf ) json["first_of"] = true;
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
                        { "switch", RimWorkflowDefinitionTools::isSwitchTask( definition.taskType( node.taskId ) ) },
                        { "branches", RimWorkflowDefinitionTools::switchBranches( definition.taskType( node.taskId ) ) },
                        { "when", node.when ? QJsonValue( node.when->source ) : QJsonValue( QJsonValue::Null ) },
                        { "when_label",
                          node.when ? QJsonValue( RimWorkflowDefinitionTools::guardLabel( *node.when ) ) : QJsonValue( QJsonValue::Null ) },
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
/// Tasks without downstream tasks, in definition order. A task used by a run condition is not a sink.
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowDefinitionTools::sinkTasks( const RimWorkflowDefinition& definition )
{
    QStringList sinks;
    for ( const auto& node : definition.nodes )
    {
        if ( !definition.outgoingEdges( node.name ).empty() ) continue;
        const bool guardsOthers = std::any_of( definition.nodes.begin(),
                                               definition.nodes.end(),
                                               [&node]( const auto& other ) { return other.when && other.when->source == node.name; } );
        if ( !guardsOthers ) sinks.append( node.name );
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
    const QString trimmedKey = key.trimmed();
    if ( trimmedKey.isEmpty() ) return std::unexpected( "The key cannot be empty" );

    RimWorkflowDefinition result = definition;
    auto                  it =
        std::find_if( result.edges.begin(), result.edges.end(), [&edge]( const auto& existing ) { return existing.sameConnection( edge ); } );
    if ( it == result.edges.end() ) return std::unexpected( "The connection does not exist" );
    if ( it->collect != RimWorkflowDefinitionEdge::Collect::Dict ) return std::unexpected( "The connection is not a keyed member" );

    for ( const auto& other : result.edges )
    {
        if ( &other != &*it && other.collect == RimWorkflowDefinitionEdge::Collect::Dict && other.to == it->to &&
             other.input == it->input && other.key == trimmedKey )
            return std::unexpected( QString( "Input '%1' already has a member with key '%2'" ).arg( it->input, trimmedKey ) );
    }
    it->key = trimmedKey;
    return result;
}

//--------------------------------------------------------------------------------------------------
/// Move a member of a `list[T]` input, or an alternative of a `first_of` input, `delta` positions
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult
    RimWorkflowDefinitionTools::moveCollectMember( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionEdge& edge, int delta )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );

    const auto current = std::find_if( definition.edges.begin(),
                                       definition.edges.end(),
                                       [&edge]( const auto& existing ) { return existing.sameConnection( edge ); } );
    if ( current == definition.edges.end() || current->collect == RimWorkflowDefinitionEdge::Collect::None )
        return std::unexpected( "The connection is not a collected member" );
    const auto kind = current->collect;

    RimWorkflowDefinition result = definition;
    std::vector<size_t>   members;
    size_t                position = 0;
    bool                  found    = false;
    for ( size_t i = 0; i < result.edges.size(); ++i )
    {
        const auto& existing = result.edges[i];
        if ( existing.to != edge.to || existing.input != edge.input || existing.collect != kind ) continue;
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
        // An edited map keeps the items it runs over, also when `over` is renamed
        const QJsonObject oldValues = definition.inputs.value( nodeName ).toObject();
        if ( node->map && oldValues.contains( node->map->over ) ) values[map->over] = oldValues.value( node->map->over );
    }
    target->map = map;
    result.inputs.insert( nodeName, values );

    // The output type changes to or from dict[str, Output]. Downstream connections are kept, and
    // validate() reports those that no longer fit; normalize() derives the config fields.
    result = normalize( result );
    return result;
}

//--------------------------------------------------------------------------------------------------
/// Rename a task instance, updating connections, run conditions, the result task and the input values
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
    for ( auto& node : result.nodes )
    {
        if ( node.when && node.when->source == oldName ) node.when->source = newName;
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
/// A switch or condition task: its output is a `taskmaestro.conditions.Branch`
//--------------------------------------------------------------------------------------------------
bool RimWorkflowDefinitionTools::isSwitchTask( const QJsonObject& taskType )
{
    const QJsonObject outputSchema = taskType.value( "output_schema" ).toObject();
    return outputSchema.value( "x-ri-python-bases" ).toArray().contains( QJsonValue( "taskmaestro.conditions.Branch" ) ) &&
           outputSchema.value( "properties" ).toObject().contains( branchField );
}

//--------------------------------------------------------------------------------------------------
/// The branch labels of a switch task, from the type of its `branch` output field: the values of a
/// Literal or an enum, or true and false for a condition task. Empty for other tasks.
//--------------------------------------------------------------------------------------------------
QJsonArray RimWorkflowDefinitionTools::switchBranches( const QJsonObject& taskType )
{
    if ( !isSwitchTask( taskType ) ) return {};

    const QJsonObject outputSchema = taskType.value( "output_schema" ).toObject();
    const QJsonObject branch =
        RimWorkflowSchemaTools::resolveReferences( outputSchema.value( "properties" ).toObject().value( branchField ).toObject(), outputSchema );
    if ( branch.contains( "enum" ) ) return branch.value( "enum" ).toArray();
    if ( branch.contains( "const" ) ) return QJsonArray{ branch.value( "const" ) };
    if ( branch.value( "type" ) == "boolean" ) return QJsonArray{ true, false };
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Output fields of a task that a run condition can test
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowDefinitionTools::guardFields( const RimWorkflowDefinition& definition, const QString& sourceName )
{
    const auto* node = definition.findNode( sourceName );
    if ( !node ) return {};
    return RimWorkflowSchemaTools::propertyNames( definition.taskType( node->taskId ).value( "output_schema" ).toObject() );
}

//--------------------------------------------------------------------------------------------------
/// Schema of the output field a run condition tests, with the `$defs` of the output schema. An
/// empty field is the branch of a switch task.
//--------------------------------------------------------------------------------------------------
QJsonObject
    RimWorkflowDefinitionTools::guardFieldSchema( const RimWorkflowDefinition& definition, const QString& sourceName, const QString& field )
{
    const auto* node = definition.findNode( sourceName );
    if ( !node ) return {};
    const QJsonObject outputSchema = definition.taskType( node->taskId ).value( "output_schema" ).toObject();
    const QJsonObject property = outputSchema.value( "properties" ).toObject().value( field.isEmpty() ? branchField : field ).toObject();
    if ( property.isEmpty() ) return {};

    QJsonObject result = RimWorkflowSchemaTools::resolveReferences( property, outputSchema );
    if ( outputSchema.contains( "$defs" ) ) result["$defs"] = outputSchema.value( "$defs" );
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowDefinitionTools::guardValueText( const QJsonValue& value )
{
    if ( value.isBool() ) return value.toBool() ? "true" : "false";
    if ( value.isDouble() ) return QString::number( value.toDouble() );
    if ( value.isNull() || value.isUndefined() ) return "None";
    return value.toString();
}

//--------------------------------------------------------------------------------------------------
/// Values typed as `a, "b, c", 2`: JSON scalars where the field is not a string, strings otherwise.
/// Commas inside double quotes do not separate values, and a quoted value is always a string.
//--------------------------------------------------------------------------------------------------
QJsonArray RimWorkflowDefinitionTools::guardValuesFromText( const QString& text, const QJsonObject& fieldSchema )
{
    QStringList tokens;
    QString     token;
    bool        inQuotes = false;
    for ( qsizetype i = 0; i < text.size(); ++i )
    {
        const QChar c = text.at( i );
        if ( inQuotes && c == '\\' && i + 1 < text.size() )
        {
            token += c;
            token += text.at( ++i );
            continue;
        }
        if ( c == '"' ) inQuotes = !inQuotes;
        if ( c == ',' && !inQuotes )
        {
            tokens.append( token );
            token.clear();
            continue;
        }
        token += c;
    }
    tokens.append( token );

    bool stringField = fieldSchema.value( "type" ).toString() == "string";
    if ( !fieldSchema.contains( "type" ) )
    {
        // A string Literal has only enum/const; every value must be a string
        QJsonArray allowed = fieldSchema.value( "enum" ).toArray();
        if ( fieldSchema.contains( "const" ) ) allowed.append( fieldSchema.value( "const" ) );
        stringField = !allowed.isEmpty() && std::ranges::all_of( allowed, []( const QJsonValue& v ) { return v.isString(); } );
    }
    QJsonArray values;
    for ( const QString& part : tokens )
    {
        const QString trimmed = part.trimmed();
        if ( trimmed.isEmpty() ) continue;

        const QJsonDocument document = QJsonDocument::fromJson( ( "[" + trimmed + "]" ).toUtf8() );
        const bool          parsed   = document.isArray() && document.array().size() == 1;
        const bool          quoted   = trimmed.size() > 1 && trimmed.startsWith( '"' ) && trimmed.endsWith( '"' );
        if ( quoted )
            values.append( parsed ? document.array().first() : QJsonValue( trimmed.mid( 1, trimmed.size() - 2 ) ) );
        else if ( !stringField && parsed )
            values.append( document.array().first() );
        else
            values.append( trimmed );
    }
    return values;
}

//--------------------------------------------------------------------------------------------------
/// The text that `guardValuesFromText` reads back as `values`. Strings are quoted only when needed.
//--------------------------------------------------------------------------------------------------
QString RimWorkflowDefinitionTools::guardValuesToText( const QJsonArray& values, const QJsonObject& fieldSchema )
{
    QStringList texts;
    for ( const QJsonValue& value : values )
    {
        if ( value.isString() && guardValuesFromText( value.toString(), fieldSchema ) == QJsonArray{ value } )
        {
            texts.append( value.toString() );
            continue;
        }
        const QByteArray json = QJsonDocument( QJsonArray{ value } ).toJson( QJsonDocument::Compact );
        texts.append( QString::fromUtf8( json.mid( 1, json.size() - 2 ) ) );
    }
    return texts.join( ", " );
}

//--------------------------------------------------------------------------------------------------
/// Short text shown on the control edge of a run condition
//--------------------------------------------------------------------------------------------------
QString RimWorkflowDefinitionTools::guardLabel( const RimWorkflowTaskGuard& guard )
{
    using Op = RimWorkflowTaskGuard::Op;
    QStringList values;
    for ( const QJsonValue& value : guard.values )
        values.append( guardValueText( value ) );

    const QString field = guard.isSwitchBranch() ? branchField : guard.field;
    switch ( guard.op )
    {
        case Op::Case:
            return values.join( ", " );
        case Op::Otherwise:
            return "otherwise";
        case Op::Equals:
            return QString( "%1 = %2" ).arg( field, values.value( 0 ) );
        case Op::In:
            return QString( "%1 in {%2}" ).arg( field, values.join( ", " ) );
        case Op::Truthy:
            return field;
        case Op::Falsy:
            return "not " + field;
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Label of the alternative at a 1-based position of a `first_of` input: 1st, 2nd, ...
//--------------------------------------------------------------------------------------------------
QString RimWorkflowDefinitionTools::firstOfLabel( int position )
{
    // Two-letter ordinal suffixes for 0 (and 4-9, 11-13), 1, 2 and 3
    const QString suffixes = "thstndrd";

    const int lastTwo = position % 100;
    const int index   = ( lastTwo >= 11 && lastTwo <= 13 ) || position % 10 > 3 ? 0 : position % 10;
    return QString::number( position ) + suffixes.mid( 2 * index, 2 );
}

//--------------------------------------------------------------------------------------------------
/// Checks a run condition of a task, mirroring the checks of taskmaestro
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimWorkflowDefinitionTools::checkTaskGuard( const RimWorkflowDefinition& definition,
                                                                         const QString&               nodeName,
                                                                         const RimWorkflowTaskGuard&  guard )
{
    using Op = RimWorkflowTaskGuard::Op;

    if ( !definition.findNode( nodeName ) ) return std::unexpected( QString( "Unknown task '%1'" ).arg( nodeName ) );
    const auto* source = definition.findNode( guard.source );
    if ( !source ) return std::unexpected( QString( "The run condition refers to unknown task '%1'" ).arg( guard.source ) );
    if ( guard.malformedRef ) return std::unexpected( "A field condition must refer to a task and an output field" );
    if ( guard.source == nodeName ) return std::unexpected( "A task cannot depend on its own output" );
    if ( source->map ) return std::unexpected( QString( "The run condition cannot refer to the mapped task '%1'" ).arg( guard.source ) );

    RimWorkflowDefinition others = definition;
    others.findNode( nodeName )->when.reset();
    if ( RimWorkflowPortCompatibility::wouldCreateCycle( others, guard.source, nodeName ) )
        return std::unexpected( "The run condition would create a cycle" );

    const QJsonObject sourceType = definition.taskType( source->taskId );
    if ( sourceType.isEmpty() ) return {};

    const bool isSwitch = isSwitchTask( sourceType );
    const bool isBranch = isSwitch && ( guard.isSwitchBranch() || guard.field == branchField );
    if ( guard.isSwitchBranch() && !isSwitch )
        return std::unexpected( QString( "'%1' is not a switch task; choose an output field to test" ).arg( guard.source ) );
    if ( guard.op == Op::Otherwise && !isBranch )
        return std::unexpected( QString( "Otherwise requires a switch task, but '%1' is not one" ).arg( guard.source ) );
    if ( guard.op == Op::Case && !guard.isSwitchBranch() ) return std::unexpected( "A case refers to the branch of a switch task" );

    const QJsonObject outputSchema = sourceType.value( "output_schema" ).toObject();
    const QString     field        = guard.isSwitchBranch() ? branchField : guard.field;
    if ( !outputSchema.value( "properties" ).toObject().contains( field ) )
        return std::unexpected( QString( "'%1' has no output field '%2'" ).arg( guard.source, field ) );
    const QJsonObject property = outputSchema.value( "properties" ).toObject().value( field ).toObject();
    const QString     target   = guard.isSwitchBranch() ? guard.source : guard.source + "." + field;

    switch ( guard.op )
    {
        case Op::Otherwise:
            return {};
        case Op::Truthy:
        case Op::Falsy:
            if ( !acceptsTruthiness( property, outputSchema ) )
            {
                return std::unexpected( QString( "%1 is %2; true/false conditions need a bool or optional field" )
                                            .arg( target, RimWorkflowPortCompatibility::typeName( property, outputSchema ) ) );
            }
            return {};
        case Op::Equals:
            if ( guard.values.size() != 1 ) return std::unexpected( "Choose the value to compare with" );
            break;
        default:
            if ( guard.values.isEmpty() ) return std::unexpected( isBranch ? "Choose at least one branch" : "Choose at least one value" );
    }

    const QJsonArray labels = isBranch ? switchBranches( sourceType ) : QJsonArray();
    for ( const QJsonValue& value : guard.values )
    {
        const bool valid = labels.isEmpty() ? valueMatchesSchema( value, property, outputSchema ) : containsValue( labels, value );
        if ( !valid )
        {
            return std::unexpected(
                QString( "%1 is not a valid value of %2 (%3)" )
                    .arg( guardValueText( value ), target, RimWorkflowPortCompatibility::typeName( property, outputSchema ) ) );
        }
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Run a task only when a condition holds, or always with std::nullopt
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult RimWorkflowDefinitionTools::setTaskGuard( const RimWorkflowDefinition&               definition,
                                                                                 const QString&                             nodeName,
                                                                                 const std::optional<RimWorkflowTaskGuard>& guard )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );
    if ( !definition.findNode( nodeName ) ) return std::unexpected( QString( "Unknown task '%1'" ).arg( nodeName ) );
    if ( guard )
    {
        if ( auto check = checkTaskGuard( definition, nodeName, *guard ); !check ) return std::unexpected( check.error() );
    }

    RimWorkflowDefinition result      = definition;
    result.findNode( nodeName )->when = guard;
    return normalize( result );
}

//--------------------------------------------------------------------------------------------------
/// Add an alternative to a connected input (`first_of` in YAML): the input takes the output of the
/// first alternative whose task ran. An existing plain connection becomes the first alternative.
//--------------------------------------------------------------------------------------------------
RimWorkflowDefinitionTools::EditResult RimWorkflowDefinitionTools::addFirstOfMember( const RimWorkflowDefinition&     definition,
                                                                                     const RimWorkflowDefinitionEdge& edge )
{
    if ( !definition.editable ) return std::unexpected( "The workflow is read-only" );
    if ( edge.input.isEmpty() ) return std::unexpected( "Alternatives (first of) need a named input" );
    if ( edge.from == edge.to ) return std::unexpected( "A task cannot be connected to itself" );
    for ( const QString& name : { edge.from, edge.to } )
    {
        if ( !definition.findNode( name ) ) return std::unexpected( QString( "Unknown task '%1'" ).arg( name ) );
    }

    bool hasExisting = false;
    for ( const auto& existing : definition.incomingEdges( edge.to ) )
    {
        if ( existing.input != edge.input ) continue;
        if ( existing.isCollected() )
            return std::unexpected( QString( "Input '%1' collects members and cannot take alternatives" ).arg( edge.input ) );
        if ( existing.sameConnection( edge ) )
            return std::unexpected( QString( "%1 is already connected to %2.%3" ).arg( edge.from, edge.to, edge.input ) );
        hasExisting = true;
    }
    if ( !hasExisting ) return std::unexpected( QString( "Input '%1' has no connection to add an alternative to" ).arg( edge.input ) );

    RimWorkflowDefinitionEdge member{ .from = edge.from, .output = edge.output, .to = edge.to, .input = edge.input };
    member.collect = RimWorkflowDefinitionEdge::Collect::FirstOf;
    if ( auto check = RimWorkflowPortCompatibility::checkFirstOfMember( definition, member ); !check )
        return std::unexpected( check.error() );
    if ( RimWorkflowPortCompatibility::wouldCreateCycle( definition, edge.from, edge.to ) )
        return std::unexpected( "The connection would create a cycle" );

    RimWorkflowDefinition result   = definition;
    auto                  insertAt = result.edges.end();
    for ( auto it = result.edges.begin(); it != result.edges.end(); ++it )
    {
        if ( it->to == edge.to && it->input == edge.input )
        {
            it->collect = RimWorkflowDefinitionEdge::Collect::FirstOf;
            insertAt    = std::next( it );
        }
    }
    // Keep the alternatives of an input together, after the existing ones
    result.edges.insert( insertAt, member );
    return normalize( result );
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
    for ( const auto& node : definition.nodes )
    {
        if ( node.when && !names.contains( node.when->source ) )
            addIssue( issues, node.name, {}, QString( "The run condition refers to unknown task '%1'" ).arg( node.when->source ) );
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
        validateGuard( definition, node, issues );
        validateSwitchCoverage( definition, node, issues );
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
        if ( edge.collect == RimWorkflowDefinitionEdge::Collect::List || edge.isFirstOf() )
            listIndex[&edge] = listCount[edge.to + "." + edge.input]++;
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
        else if ( edge.isFirstOf() )
        {
            json["collect"] = collectName( edge.collect );
            json["label"]   = firstOfLabel( listIndex[&edge] + 1 );
        }
        edges.append( json );
    }

    // Run conditions are drawn as control edges from the source task
    for ( const auto& node : definition.nodes )
    {
        if ( !node.when ) continue;
        edges.append(
            QJsonObject{ { "from", node.when->source }, { "to", node.name }, { "kind", "when" }, { "label", guardLabel( *node.when ) } } );
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
