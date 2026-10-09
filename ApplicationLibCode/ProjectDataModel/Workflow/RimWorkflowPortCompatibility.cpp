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

#include "RimWorkflowPortCompatibility.h"

#include "RimWorkflowDefinition.h"
#include "RimWorkflowSchemaTools.h"

#include <QJsonArray>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>

namespace
{
QString shortName( const QString& pythonType )
{
    return pythonType.mid( pythonType.lastIndexOf( '.' ) + 1 );
}

QStringList pythonTypes( const QJsonObject& schema )
{
    QStringList types;
    for ( const char* key : { "x-taskmaestro-python-type", "x-ri-python-type" } )
    {
        const QString type = schema.value( key ).toString();
        if ( !type.isEmpty() ) types.append( type );
    }
    for ( const QJsonValue& base : schema.value( "x-ri-python-bases" ).toArray() )
        types.append( base.toString() );
    return types;
}

QJsonArray enumValues( const QJsonObject& schema )
{
    if ( schema.contains( "const" ) ) return QJsonArray{ schema.value( "const" ) };
    return schema.value( "enum" ).toArray();
}

bool isHiddenObjectValue( const QString& name, const QJsonObject& property, const QJsonObject& rootSchema )
{
    // ObjectModel wraps its runtime object in an opaque `value` field that is not routable on its own
    return name == "value" && RimWorkflowSchemaTools::isOpaque( property, rootSchema );
}

bool isModelCompatible( const QJsonObject& produced, const QJsonObject& expected )
{
    const QString expectedType = expected.value( "x-ri-python-type" ).toString();
    if ( !expectedType.isEmpty() && produced.contains( "x-ri-python-type" ) ) return pythonTypes( produced ).contains( expectedType );

    if ( expected.contains( "title" ) && produced.contains( "title" ) && expected.contains( "properties" ) )
        return expected.value( "title" ) == produced.value( "title" );
    return true;
}

bool isScalarCompatible( const QJsonObject& produced, const QJsonObject& expected )
{
    const QString producedType = produced.value( "type" ).toString();
    const QString expectedType = expected.value( "type" ).toString();

    // bool is a subclass of int in Python, but int is not a subclass of float
    if ( producedType != expectedType && !( producedType == "boolean" && expectedType == "integer" ) ) return false;
    if ( produced.value( "format" ).toString() != expected.value( "format" ).toString() ) return false;

    const QJsonArray expectedEnum = enumValues( expected );
    if ( expectedEnum.isEmpty() ) return true;
    const QJsonArray producedEnum = enumValues( produced );
    if ( producedEnum.isEmpty() ) return false;
    for ( const QJsonValue& value : producedEnum )
    {
        if ( !expectedEnum.contains( value ) ) return false;
    }
    return true;
}

std::optional<RimWorkflowPort> findPort( const std::vector<RimWorkflowPort>& ports, const QString& name )
{
    for ( const auto& port : ports )
    {
        if ( port.name == name ) return port;
    }
    return std::nullopt;
}

bool hasRequiredConfigurableField( const QJsonObject& inputSchema, const QStringList& covered )
{
    const QJsonObject properties = inputSchema.value( "properties" ).toObject();
    for ( const QString& field : RimWorkflowSchemaTools::requiredFields( inputSchema ) )
    {
        if ( covered.contains( field ) ) continue;
        if ( RimWorkflowSchemaTools::isConfigurable( properties.value( field ).toObject(), inputSchema ) ) return true;
    }
    return false;
}

std::expected<void, QString>
    checkWholeToWhole( const QJsonObject& upstreamType, const QJsonObject& downstreamType, const QString& from, const QString& to )
{
    const QJsonObject outputSchema = upstreamType.value( "output_schema" ).toObject();
    const QJsonObject inputSchema  = downstreamType.value( "input_schema" ).toObject();
    if ( RimWorkflowPortCompatibility::isCompatible( outputSchema, outputSchema, inputSchema, inputSchema ) ) return {};

    const QString mismatch = QString( "'%1' outputs %2, but '%3' expects %4" )
                                 .arg( from, RimWorkflowPortCompatibility::typeName( outputSchema, outputSchema ) )
                                 .arg( to, RimWorkflowPortCompatibility::typeName( inputSchema, inputSchema ) );

    // taskmaestro merges the upstream fields with configured values when the task has config fields
    const QStringList shared = RimWorkflowPortCompatibility::coveredByWholeOutput( upstreamType, downstreamType );
    if ( shared.isEmpty() || !hasRequiredConfigurableField( inputSchema, shared ) ) return std::unexpected( mismatch );

    const QJsonObject outputProperties = outputSchema.value( "properties" ).toObject();
    const QJsonObject inputProperties  = inputSchema.value( "properties" ).toObject();
    for ( const QString& field : shared )
    {
        if ( !RimWorkflowPortCompatibility::isCompatible( outputProperties.value( field ).toObject(),
                                                          outputSchema,
                                                          inputProperties.value( field ).toObject(),
                                                          inputSchema ) )
        {
            return std::unexpected( QString( "Type mismatch for field '%1': %2" ).arg( field, mismatch ) );
        }
    }
    return {};
}
//--------------------------------------------------------------------------------------------------
/// Icon of the ResInsight object a schema holds, or an empty string. The rips class is found in the
/// Python class bases: directly for opaque rips objects, or as `ObjectModel[Class]` for wrappers.
/// The icons are the ones the matching ResInsight project classes use.
//--------------------------------------------------------------------------------------------------
QString objectIconResource( const QJsonObject& schema, const QJsonObject& rootSchema )
{
    const QJsonObject resolved = RimWorkflowSchemaTools::resolveReferences( schema, rootSchema );
    for ( const char* unionKey : { "anyOf", "oneOf" } )
    {
        for ( const QJsonValue& option : resolved.value( unionKey ).toArray() )
        {
            const QString icon = objectIconResource( option.toObject(), rootSchema );
            if ( !icon.isEmpty() ) return icon;
        }
    }
    if ( resolved.value( "type" ).toString() == "array" ) return objectIconResource( resolved.value( "items" ).toObject(), rootSchema );
    if ( resolved.value( "type" ).toString() == "object" && resolved.value( "additionalProperties" ).isObject() &&
         !resolved.contains( "properties" ) )
        return objectIconResource( resolved.value( "additionalProperties" ).toObject(), rootSchema );

    static const QMap<QString, QString> icons = { { "Instance", ":/AppLogo48x48.png" },
                                                  { "Case", ":/Case48x48.png" },
                                                  { "Reservoir", ":/Case48x48.png" },
                                                  { "EclipseCase", ":/Case48x48.png" },
                                                  { "View", ":/3DView16x16.png" },
                                                  { "EclipseView", ":/3DView16x16.png" },
                                                  { "WellPath", ":/Well.svg" },
                                                  { "Surface", ":/ReservoirSurface16x16.png" } };

    QJsonArray bases = resolved.value( "x-ri-python-bases" ).toArray();
    if ( bases.isEmpty() && resolved.contains( "x-ri-python-type" ) ) bases.append( resolved.value( "x-ri-python-type" ) );

    static const QRegularExpression objectModel( R"(ObjectModel\[(\w+)\]$)" );
    for ( const QJsonValue& value : bases )
    {
        const QString base = value.toString();
        QString       className;
        if ( const auto match = objectModel.match( base ); match.hasMatch() )
            className = match.captured( 1 );
        else if ( base.startsWith( "rips." ) )
            className = base.section( '.', -1 );
        if ( icons.contains( className ) ) return icons.value( className );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Icon of a plain value (string, number, bool, date, path), or an empty string. The icons are Codicons,
/// see Resources/codicons/README.md.
//--------------------------------------------------------------------------------------------------
QString valueIconResource( const QJsonObject& schema, const QJsonObject& rootSchema )
{
    const QJsonObject resolved = RimWorkflowSchemaTools::resolveReferences( schema, rootSchema );
    for ( const char* unionKey : { "anyOf", "oneOf" } )
    {
        for ( const QJsonValue& option : resolved.value( unionKey ).toArray() )
        {
            const QString icon = valueIconResource( option.toObject(), rootSchema );
            if ( !icon.isEmpty() ) return icon;
        }
    }

    const QString type = resolved.value( "type" ).toString();
    if ( type == "array" ) return valueIconResource( resolved.value( "items" ).toObject(), rootSchema );
    if ( type == "object" && resolved.value( "additionalProperties" ).isObject() && !resolved.contains( "properties" ) )
        return valueIconResource( resolved.value( "additionalProperties" ).toObject(), rootSchema );

    if ( type == "boolean" ) return ":/codicons/check.svg";
    if ( type == "integer" || type == "number" ) return ":/codicons/symbol-numeric.svg";
    if ( type != "string" ) return {};

    const QString format = resolved.value( "format" ).toString();
    if ( format == "date" || format == "date-time" ) return ":/codicons/calendar.svg";
    if ( format == "directory-path" ) return ":/codicons/folder.svg";
    if ( format == "path" || format == "file-path" ) return ":/codicons/file.svg";
    return ":/codicons/symbol-string.svg";
}

} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowPort::isWhole() const
{
    return name.isEmpty();
}

//--------------------------------------------------------------------------------------------------
/// The whole input model first, then its fields
//--------------------------------------------------------------------------------------------------
std::vector<RimWorkflowPort> RimWorkflowPortCompatibility::inputPorts( const QJsonObject& taskType )
{
    const QJsonObject inputSchema = taskType.value( "input_schema" ).toObject();

    std::vector<RimWorkflowPort> ports;
    ports.push_back( { .name         = "",
                       .schema       = inputSchema,
                       .rootSchema   = inputSchema,
                       .description  = inputSchema.value( "description" ).toString(),
                       .typeName     = typeName( inputSchema, inputSchema ),
                       .iconResource = iconResource( inputSchema, inputSchema ),
                       .required     = true } );

    const QStringList required   = RimWorkflowSchemaTools::requiredFields( inputSchema );
    const QJsonObject properties = inputSchema.value( "properties" ).toObject();
    for ( const QString& name : RimWorkflowSchemaTools::propertyNames( inputSchema ) )
    {
        const QJsonObject property = properties.value( name ).toObject();
        if ( isHiddenObjectValue( name, property, inputSchema ) ) continue;
        ports.push_back( { .name         = name,
                           .schema       = property,
                           .rootSchema   = inputSchema,
                           .description  = property.value( "description" ).toString(),
                           .typeName     = typeName( property, inputSchema ),
                           .iconResource = iconResource( property, inputSchema ),
                           .required     = required.contains( name ),
                           .configurable = RimWorkflowSchemaTools::isConfigurable( property, inputSchema ) } );
    }
    return ports;
}

//--------------------------------------------------------------------------------------------------
/// The whole output model first, then its fields
//--------------------------------------------------------------------------------------------------
std::vector<RimWorkflowPort> RimWorkflowPortCompatibility::outputPorts( const QJsonObject& taskType )
{
    const QJsonObject outputSchema = taskType.value( "output_schema" ).toObject();

    std::vector<RimWorkflowPort> ports;
    ports.push_back( { .name         = "",
                       .schema       = outputSchema,
                       .rootSchema   = outputSchema,
                       .description  = outputSchema.value( "description" ).toString(),
                       .typeName     = typeName( outputSchema, outputSchema ),
                       .iconResource = iconResource( outputSchema, outputSchema ) } );

    const QJsonObject properties = outputSchema.value( "properties" ).toObject();
    for ( const QString& name : RimWorkflowSchemaTools::propertyNames( outputSchema ) )
    {
        const QJsonObject property = properties.value( name ).toObject();
        if ( isHiddenObjectValue( name, property, outputSchema ) ) continue;
        ports.push_back( { .name         = name,
                           .schema       = property,
                           .rootSchema   = outputSchema,
                           .description  = property.value( "description" ).toString(),
                           .typeName     = typeName( property, outputSchema ),
                           .iconResource = iconResource( property, outputSchema ) } );
    }
    return ports;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RimWorkflowPort> RimWorkflowPortCompatibility::inputPort( const QJsonObject& taskType, const QString& name )
{
    return findPort( inputPorts( taskType ), name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RimWorkflowPort> RimWorkflowPortCompatibility::outputPort( const QJsonObject& taskType, const QString& name )
{
    return findPort( outputPorts( taskType ), name );
}

//--------------------------------------------------------------------------------------------------
/// A schema without any type information accepts and produces anything
//--------------------------------------------------------------------------------------------------
bool RimWorkflowPortCompatibility::isAny( const QJsonObject& schema, const QJsonObject& rootSchema )
{
    const QJsonObject resolved = RimWorkflowSchemaTools::resolveReferences( schema, rootSchema );
    for ( const char* key : { "type", "anyOf", "oneOf", "properties", "enum", "const", "x-taskmaestro-opaque", "$ref" } )
    {
        if ( resolved.contains( key ) ) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// Whether every value of the produced schema is accepted by the expected schema. Models and
/// runtime objects compare their Python class and its bases, like `issubclass`.
//--------------------------------------------------------------------------------------------------
bool RimWorkflowPortCompatibility::isCompatible( const QJsonObject& produced,
                                                 const QJsonObject& producedRoot,
                                                 const QJsonObject& expected,
                                                 const QJsonObject& expectedRoot )
{
    if ( isAny( produced, producedRoot ) || isAny( expected, expectedRoot ) ) return true;

    const QJsonObject p = RimWorkflowSchemaTools::resolveReferences( produced, producedRoot );
    const QJsonObject e = RimWorkflowSchemaTools::resolveReferences( expected, expectedRoot );

    // Every produced option must be accepted; an expected union needs one accepting option
    for ( const char* unionKey : { "anyOf", "oneOf" } )
    {
        if ( !p.value( unionKey ).isArray() ) continue;
        for ( const QJsonValue& option : p.value( unionKey ).toArray() )
        {
            if ( !isCompatible( option.toObject(), producedRoot, e, expectedRoot ) ) return false;
        }
        return true;
    }
    for ( const char* unionKey : { "anyOf", "oneOf" } )
    {
        if ( !e.value( unionKey ).isArray() ) continue;
        for ( const QJsonValue& option : e.value( unionKey ).toArray() )
        {
            if ( isCompatible( p, producedRoot, option.toObject(), expectedRoot ) ) return true;
        }
        return false;
    }

    const bool producedOpaque = p.value( "x-taskmaestro-opaque" ).toBool( false );
    if ( e.value( "x-taskmaestro-opaque" ).toBool( false ) )
    {
        return producedOpaque && pythonTypes( p ).contains( e.value( "x-taskmaestro-python-type" ).toString() );
    }
    if ( producedOpaque ) return false;

    const QString producedType = p.value( "type" ).toString();
    const QString expectedType = e.value( "type" ).toString();
    if ( producedType == "object" || expectedType == "object" )
    {
        if ( producedType != expectedType ) return false;
        if ( e.value( "additionalProperties" ).isObject() && !e.contains( "properties" ) )
        {
            return isCompatible( p.value( "additionalProperties" ).toObject(),
                                 producedRoot,
                                 e.value( "additionalProperties" ).toObject(),
                                 expectedRoot );
        }
        return isModelCompatible( p, e );
    }

    if ( producedType == "array" || expectedType == "array" )
    {
        if ( producedType != expectedType ) return false;
        return isCompatible( p.value( "items" ).toObject(), producedRoot, e.value( "items" ).toObject(), expectedRoot );
    }

    return isScalarCompatible( p, e );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowPortCompatibility::isCompatible( const RimWorkflowPort& produced, const RimWorkflowPort& expected )
{
    return isCompatible( produced.schema, produced.rootSchema, expected.schema, expected.rootSchema );
}

//--------------------------------------------------------------------------------------------------
/// Short readable type name used in tooltips and error messages
//--------------------------------------------------------------------------------------------------
QString RimWorkflowPortCompatibility::typeName( const QJsonObject& schema, const QJsonObject& rootSchema )
{
    if ( isAny( schema, rootSchema ) ) return "Any";

    const QJsonObject resolved = RimWorkflowSchemaTools::resolveReferences( schema, rootSchema );
    for ( const char* unionKey : { "anyOf", "oneOf" } )
    {
        if ( !resolved.value( unionKey ).isArray() ) continue;
        QStringList names;
        for ( const QJsonValue& option : resolved.value( unionKey ).toArray() )
            names.append( typeName( option.toObject(), rootSchema ) );
        return names.join( " | " );
    }

    if ( resolved.value( "x-taskmaestro-opaque" ).toBool( false ) )
        return shortName( resolved.value( "x-taskmaestro-python-type" ).toString( "object" ) );
    if ( resolved.contains( "x-ri-python-type" ) ) return shortName( resolved.value( "x-ri-python-type" ).toString() );

    const QString type = resolved.value( "type" ).toString();
    if ( type == "object" && resolved.value( "additionalProperties" ).isObject() && !resolved.contains( "properties" ) )
        return QString( "dict[str, %1]" ).arg( typeName( resolved.value( "additionalProperties" ).toObject(), rootSchema ) );
    if ( type == "object" ) return resolved.value( "title" ).toString( "object" );
    if ( type == "array" ) return QString( "list[%1]" ).arg( typeName( resolved.value( "items" ).toObject(), rootSchema ) );

    static const QMap<QString, QString> pythonNames = { { "string", "str" },
                                                        { "integer", "int" },
                                                        { "number", "float" },
                                                        { "boolean", "bool" },
                                                        { "null", "None" } };
    const QString                       format      = resolved.value( "format" ).toString();
    if ( format == "date" ) return "date";
    if ( format == "date-time" ) return "datetime";
    if ( format == "path" || format == "file-path" || format == "directory-path" ) return "Path";
    return pythonNames.value( type, type );
}

//--------------------------------------------------------------------------------------------------
/// Icon of a port: the ResInsight object it holds, otherwise the kind of value
//--------------------------------------------------------------------------------------------------
QString RimWorkflowPortCompatibility::iconResource( const QJsonObject& schema, const QJsonObject& rootSchema )
{
    const QString objectIcon = objectIconResource( schema, rootSchema );
    return objectIcon.isEmpty() ? valueIconResource( schema, rootSchema ) : objectIcon;
}

//--------------------------------------------------------------------------------------------------
/// Port name -> icon resource, for the ports that have an icon
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowPortCompatibility::portIcons( const std::vector<RimWorkflowPort>& ports )
{
    QJsonObject icons;
    for ( const auto& port : ports )
    {
        if ( !port.iconResource.isEmpty() ) icons[port.name] = port.iconResource;
    }
    return icons;
}

//--------------------------------------------------------------------------------------------------
/// Port name -> type name, with an empty name for the whole model. Used to label ports in the graph.
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowPortCompatibility::portTypes( const std::vector<RimWorkflowPort>& ports )
{
    QJsonObject types;
    for ( const auto& port : ports )
        types[port.name] = port.typeName;
    return types;
}

//--------------------------------------------------------------------------------------------------
/// Input fields of the downstream task supplied when it takes the whole upstream output
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowPortCompatibility::coveredByWholeOutput( const QJsonObject& upstreamTaskType, const QJsonObject& downstreamTaskType )
{
    const QJsonObject outputSchema = upstreamTaskType.value( "output_schema" ).toObject();
    const QJsonObject inputSchema  = downstreamTaskType.value( "input_schema" ).toObject();
    const QStringList inputFields  = RimWorkflowSchemaTools::propertyNames( inputSchema );
    if ( isCompatible( outputSchema, outputSchema, inputSchema, inputSchema ) ) return inputFields;

    const QStringList outputFields = RimWorkflowSchemaTools::propertyNames( outputSchema );
    QStringList       covered;
    for ( const QString& field : inputFields )
    {
        if ( outputFields.contains( field ) ) covered.append( field );
    }
    return covered;
}

//--------------------------------------------------------------------------------------------------
/// Whether adding the connection from -> to closes a cycle
//--------------------------------------------------------------------------------------------------
bool RimWorkflowPortCompatibility::wouldCreateCycle( const RimWorkflowDefinition& definition, const QString& from, const QString& to )
{
    if ( from == to ) return true;

    QStringList   pending{ to };
    QSet<QString> visited;
    while ( !pending.isEmpty() )
    {
        const QString current = pending.takeLast();
        if ( current == from ) return true;
        if ( visited.contains( current ) ) continue;
        visited.insert( current );
        for ( const auto& edge : definition.edges )
        {
            if ( edge.from == current ) pending.append( edge.to );
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// A mapped task runs once per item: the key and value fields are filled by the map and are not
/// inputs, and the output is `dict[str, Output]`
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowPortCompatibility::mappedTaskType( const QJsonObject& taskType, const RimWorkflowTaskMap& map )
{
    QJsonObject inputSchema = taskType.value( "input_schema" ).toObject();
    QJsonObject properties  = inputSchema.value( "properties" ).toObject();
    properties.remove( map.keyAs );
    properties.remove( map.valueAs );
    inputSchema["properties"] = properties;

    QJsonArray required;
    for ( const QJsonValue& field : inputSchema.value( "required" ).toArray() )
    {
        if ( field.toString() != map.keyAs && field.toString() != map.valueAs ) required.append( field );
    }
    inputSchema["required"] = required;

    QJsonObject result      = taskType;
    result["input_schema"]  = inputSchema;
    result["output_schema"] = mappedOutputSchema( taskType.value( "output_schema" ).toObject() );
    return result;
}

//--------------------------------------------------------------------------------------------------
/// `MappedOutput[Output]`, a `dict[str, Output]`. The output model moves into `$defs`, so its own
/// references still resolve against the new root.
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowPortCompatibility::mappedOutputSchema( const QJsonObject& outputSchema )
{
    QJsonObject definitions = outputSchema.value( "$defs" ).toObject();
    QJsonObject item        = outputSchema;
    item.remove( "$defs" );

    QString itemKey = "MappedItem";
    while ( definitions.contains( itemKey ) )
        itemKey += "_";
    definitions[itemKey] = item;

    return QJsonObject{ { "type", "object" },
                        { "title", QString( "dict[str, %1]" ).arg( typeName( outputSchema, outputSchema ) ) },
                        { "additionalProperties", QJsonObject{ { "$ref", "#/$defs/" + itemKey } } },
                        { "$defs", definitions },
                        { "x-ri-mapped-output", true } };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<RimWorkflowDefinitionEdge::Collect, QJsonObject>>
    RimWorkflowPortCompatibility::collectTarget( const RimWorkflowPort& expected )
{
    if ( expected.isWhole() ) return std::nullopt;

    const QJsonObject resolved = RimWorkflowSchemaTools::resolveReferences( expected.schema, expected.rootSchema );
    QJsonArray        options;
    for ( const char* unionKey : { "anyOf", "oneOf" } )
    {
        for ( const QJsonValue& option : resolved.value( unionKey ).toArray() )
            options.append( option );
    }
    if ( options.isEmpty() ) options.append( resolved );

    for ( const QJsonValue& value : options )
    {
        const QJsonObject option = RimWorkflowSchemaTools::resolveReferences( value.toObject(), expected.rootSchema );
        const QString     type   = option.value( "type" ).toString();
        if ( type == "array" && option.value( "items" ).isObject() )
            return std::make_pair( RimWorkflowDefinitionEdge::Collect::List, option.value( "items" ).toObject() );
        if ( type == "object" && option.value( "additionalProperties" ).isObject() && !option.contains( "properties" ) )
            return std::make_pair( RimWorkflowDefinitionEdge::Collect::Dict, option.value( "additionalProperties" ).toObject() );
    }
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
/// Checks a new connection and returns the edge to add
//--------------------------------------------------------------------------------------------------
std::expected<RimWorkflowDefinitionEdge, QString> RimWorkflowPortCompatibility::resolveConnection( const RimWorkflowDefinition& definition,
                                                                                                   const QString&               from,
                                                                                                   const QString&               output,
                                                                                                   const QString&               to,
                                                                                                   const QString&               input )
{
    using Collect = RimWorkflowDefinitionEdge::Collect;

    if ( from == to ) return std::unexpected( "A task cannot be connected to itself" );
    if ( !definition.findNode( from ) ) return std::unexpected( QString( "Unknown task '%1'" ).arg( from ) );
    const auto* downstream = definition.findNode( to );
    if ( !downstream ) return std::unexpected( QString( "Unknown task '%1'" ).arg( to ) );

    const QJsonObject upstreamType   = definition.taskTypeForNode( from );
    const QJsonObject downstreamType = definition.taskTypeForNode( to );
    if ( upstreamType.isEmpty() ) return std::unexpected( QString( "The task type of '%1' is not installed" ).arg( from ) );
    if ( downstreamType.isEmpty() ) return std::unexpected( QString( "The task type of '%1' is not installed" ).arg( to ) );

    const auto producedPort = outputPort( upstreamType, output );
    if ( !producedPort ) return std::unexpected( QString( "'%1' has no output '%2'" ).arg( from, output ) );
    const auto expectedPort = inputPort( downstreamType, input );
    if ( !expectedPort ) return std::unexpected( QString( "'%1' has no input '%2'" ).arg( to, input ) );
    if ( downstream->map && input.isEmpty() )
        return std::unexpected( QString( "'%1' is mapped and takes its inputs field by field" ).arg( to ) );

    RimWorkflowDefinitionEdge edge{ .from = from, .output = output, .to = to, .input = input };
    if ( !input.isEmpty() && !isCompatible( *producedPort, *expectedPort ) )
    {
        const auto target = collectTarget( *expectedPort );
        if ( target && isCompatible( producedPort->schema, producedPort->rootSchema, target->second, expectedPort->rootSchema ) )
        {
            edge.collect = target->first;
        }
        else
        {
            const QString source = output.isEmpty() ? from : from + "." + output;
            return std::unexpected(
                QString( "%1 is %2, but %3.%4 expects %5" ).arg( source, producedPort->typeName, to, input, expectedPort->typeName ) );
        }
    }

    // Edges into the same input are replaced by the new connection, except other collected members
    RimWorkflowDefinition remaining = definition;
    std::erase_if( remaining.edges,
                   [&]( const RimWorkflowDefinitionEdge& existing )
                   { return existing.to == to && existing.input == input && ( !edge.isCollected() || existing.collect != edge.collect ); } );
    if ( wouldCreateCycle( remaining, from, to ) ) return std::unexpected( "The connection would create a cycle" );

    QStringList keys;
    for ( const auto& existing : remaining.incomingEdges( to ) )
    {
        if ( input.isEmpty() && !existing.isWholeInput() )
            return std::unexpected( QString( "'%1' already has individual inputs connected and cannot also take a whole input" ).arg( to ) );
        if ( !input.isEmpty() && existing.isWholeInput() )
            return std::unexpected( QString( "'%1' takes its whole input from '%2'; disconnect it first" ).arg( to, existing.from ) );
        if ( existing.input != input ) continue;
        if ( existing.sameConnection( edge ) )
            return std::unexpected( QString( "%1 is already collected into %2.%3" ).arg( from, to, input ) );
        keys.append( existing.key );
    }

    if ( input.isEmpty() && output.isEmpty() )
    {
        if ( auto check = checkWholeToWhole( upstreamType, downstreamType, from, to ); !check ) return std::unexpected( check.error() );
        return edge;
    }
    if ( input.isEmpty() && !isCompatible( *producedPort, *expectedPort ) )
    {
        return std::unexpected(
            QString( "%1.%2 is %3, but %4 expects %5" ).arg( from, output, producedPort->typeName, to, expectedPort->typeName ) );
    }

    if ( edge.collect == Collect::Dict )
    {
        // The upstream task name is the default key
        edge.key = output.isEmpty() ? from : from + "_" + output;
        for ( int index = 2; keys.contains( edge.key ); ++index )
            edge.key = QString( "%1_%2" ).arg( output.isEmpty() ? from : from + "_" + output ).arg( index );
    }
    return edge;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimWorkflowPortCompatibility::canConnect( const RimWorkflowDefinition& definition,
                                                                       const QString&               from,
                                                                       const QString&               output,
                                                                       const QString&               to,
                                                                       const QString&               input )
{
    auto edge = resolveConnection( definition, from, output, to, input );
    if ( !edge ) return std::unexpected( edge.error() );
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Type check of one member of a collected input
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimWorkflowPortCompatibility::checkCollectMember( const RimWorkflowDefinition&     definition,
                                                                               const RimWorkflowDefinitionEdge& edge )
{
    const QJsonObject upstreamType   = definition.taskTypeForNode( edge.from );
    const QJsonObject downstreamType = definition.taskTypeForNode( edge.to );
    if ( upstreamType.isEmpty() || downstreamType.isEmpty() ) return {};

    const auto producedPort = outputPort( upstreamType, edge.output );
    if ( !producedPort ) return std::unexpected( QString( "'%1' has no output '%2'" ).arg( edge.from, edge.output ) );
    const auto expectedPort = inputPort( downstreamType, edge.input );
    if ( !expectedPort || expectedPort->isWhole() )
        return std::unexpected( QString( "'%1' has no input '%2'" ).arg( edge.to, edge.input ) );

    const auto target = collectTarget( *expectedPort );
    const bool isList = edge.collect == RimWorkflowDefinitionEdge::Collect::List;
    if ( !target || target->first != edge.collect )
    {
        return std::unexpected( QString( "Input '%1' is %2 and cannot collect %3" )
                                    .arg( edge.input, expectedPort->typeName, isList ? "a list of members" : "keyed members" ) );
    }
    if ( !isCompatible( producedPort->schema, producedPort->rootSchema, target->second, expectedPort->rootSchema ) )
    {
        const QString source = edge.output.isEmpty() ? edge.from : edge.from + "." + edge.output;
        return std::unexpected(
            QString( "%1 is %2, but the members of %3.%4 must be %5" )
                .arg( source, producedPort->typeName, edge.to, edge.input, typeName( target->second, expectedPort->rootSchema ) ) );
    }
    return {};
}
