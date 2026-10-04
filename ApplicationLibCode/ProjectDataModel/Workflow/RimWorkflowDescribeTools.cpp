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

#include "RimWorkflowDescribeTools.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QMap>
#include <QStringList>

namespace
{
QJsonObject resolveRef( const QString& ref, const QJsonObject& rootSchema )
{
    const QString prefix = "#/$defs/";
    if ( !ref.startsWith( prefix ) ) return {};
    return rootSchema.value( "$defs" ).toObject().value( ref.mid( prefix.size() ) ).toObject();
}

//--------------------------------------------------------------------------------------------------
/// Follow $ref and unwrap single-item allOf and Optional (anyOf with null) to the underlying type
//--------------------------------------------------------------------------------------------------
QJsonObject underlyingSchema( QJsonObject schema, const QJsonObject& rootSchema )
{
    constexpr int maxDepth = 16;
    for ( int depth = 0; depth < maxDepth; ++depth )
    {
        if ( schema.contains( "$ref" ) )
        {
            schema = resolveRef( schema.value( "$ref" ).toString(), rootSchema );
            continue;
        }

        const QJsonArray allOf = schema.value( "allOf" ).toArray();
        if ( allOf.size() == 1 )
        {
            schema = allOf.first().toObject();
            continue;
        }

        if ( schema.value( "anyOf" ).isArray() )
        {
            QJsonArray nonNull;
            for ( const QJsonValue& arm : schema.value( "anyOf" ).toArray() )
            {
                if ( arm.toObject().value( "type" ).toString() != "null" ) nonNull.append( arm );
            }
            if ( nonNull.size() == 1 )
            {
                schema = nonNull.first().toObject();
                continue;
            }
        }
        break;
    }
    return schema;
}

bool isOpaque( const QJsonObject& schema, const QJsonObject& rootSchema )
{
    return underlyingSchema( schema, rootSchema ).value( "x-taskmaestro-opaque" ).toBool( false );
}

//--------------------------------------------------------------------------------------------------
/// Python type of a runtime-only object, either bare or wrapped in an ObjectModel `value` field
//--------------------------------------------------------------------------------------------------
QString objectPythonType( const QJsonObject& typeSchema, const QJsonObject& rootSchema )
{
    if ( typeSchema.contains( "x-taskmaestro-python-type" ) ) return typeSchema.value( "x-taskmaestro-python-type" ).toString();

    const QJsonObject valueSchema = typeSchema.value( "properties" ).toObject().value( "value" ).toObject();
    if ( valueSchema.isEmpty() ) return {};
    return underlyingSchema( valueSchema, rootSchema ).value( "x-taskmaestro-python-type" ).toString();
}

QString jsonType( const QJsonObject& typeSchema )
{
    const QString type = typeSchema.value( "type" ).toString();
    if ( type == "boolean" || type == "integer" || type == "number" || type == "array" ) return type;
    return "string";
}

//--------------------------------------------------------------------------------------------------
/// taskmaestro_resinsight.models.Vec3 is a plain pydantic model (x, y, z numbers), so it has no
/// x-taskmaestro-python-type tag; recognize it structurally instead.
//--------------------------------------------------------------------------------------------------
bool isVec3Schema( const QJsonObject& typeSchema )
{
    if ( typeSchema.value( "type" ).toString() != "object" ) return false;

    const QJsonObject properties = typeSchema.value( "properties" ).toObject();
    if ( properties.size() != 3 ) return false;

    for ( const QString& component : { "x", "y", "z" } )
    {
        if ( properties.value( component ).toObject().value( "type" ).toString() != "number" ) return false;
    }
    return true;
}

QJsonObject configFieldSchema( const QString&     fieldName,
                               const QJsonObject& inputSchema,
                               const QStringList& requiredFields,
                               const QJsonObject& configValues )
{
    QJsonObject entry{ { "name", fieldName } };

    const QJsonObject properties = inputSchema.value( "properties" ).toObject();
    if ( !properties.contains( fieldName ) )
    {
        entry["type"]     = "string";
        entry["required"] = true;
        entry["error"]    = "field not found in input model";
        return entry;
    }

    const QJsonObject property   = properties.value( fieldName ).toObject();
    const QJsonObject typeSchema = underlyingSchema( property, inputSchema );
    QString resinsightType       = RimWorkflowDescribeTools::resinsightTypeFromPythonType( objectPythonType( typeSchema, inputSchema ) );
    if ( resinsightType.isEmpty() && isVec3Schema( typeSchema ) ) resinsightType = "Vec3";

    entry["type"]     = resinsightType.isEmpty() ? jsonType( typeSchema ) : "object";
    entry["required"] = requiredFields.contains( fieldName );

    const QString description = property.value( "description" ).toString();
    if ( !description.isEmpty() ) entry["description"] = description;

    // Prefer the value already in input.yaml; fall back to the model default
    if ( configValues.contains( fieldName ) )
        entry["default"] = configValues.value( fieldName );
    else if ( property.contains( "default" ) )
        entry["default"] = property.value( "default" );

    const QString format = typeSchema.value( "format" ).toString();
    if ( !format.isEmpty() ) entry["format"] = format;

    if ( !resinsightType.isEmpty() ) entry["resinsight_type"] = resinsightType;

    return entry;
}

void appendEdge( QJsonArray& edges, const QJsonObject& ref, const QString& to, const QString& input )
{
    const QString from = ref.value( "task" ).toString();
    if ( from.isEmpty() ) return;

    QJsonObject edge{ { "from", from }, { "to", to } };
    if ( !input.isEmpty() ) edge["input"] = input;
    const QString output = ref.value( "field" ).toString();
    if ( !output.isEmpty() ) edge["output"] = output;
    edges.append( edge );
}

void appendDependencyEdges( QJsonArray& edges, const QString& taskName, const QJsonValue& dependsOn )
{
    const QJsonObject deps = dependsOn.toObject();
    if ( deps.isEmpty() ) return;

    // Single upstream task: {"task": ..., "field": ...}
    if ( deps.value( "task" ).isString() )
    {
        appendEdge( edges, deps, taskName, {} );
        return;
    }

    for ( auto it = deps.begin(); it != deps.end(); ++it )
    {
        const QJsonObject ref = it.value().toObject();
        if ( !ref.contains( "collect" ) )
        {
            appendEdge( edges, ref, taskName, it.key() );
            continue;
        }

        const QJsonValue members = ref.value( "collect" ).toObject().value( "members" );
        if ( members.isArray() )
        {
            for ( const QJsonValue& member : members.toArray() )
                appendEdge( edges, member.toObject(), taskName, it.key() );
        }
        else
        {
            for ( const QJsonValue& member : members.toObject() )
                appendEdge( edges, member.toObject(), taskName, it.key() );
        }
    }
}

QJsonObject taskFromDescribe( const QJsonObject& task )
{
    const QJsonObject inputSchema  = task.value( "input_schema" ).toObject();
    const QJsonObject outputSchema = task.value( "output_schema" ).toObject();

    QJsonArray inputs;
    for ( const QString& key : inputSchema.value( "properties" ).toObject().keys() )
        inputs.append( key );

    // ObjectModel wraps its object in an opaque `value` field, which is not a routable output
    QJsonArray        outputs;
    const QJsonObject outputProperties = outputSchema.value( "properties" ).toObject();
    for ( auto it = outputProperties.begin(); it != outputProperties.end(); ++it )
    {
        if ( it.key() == "value" && isOpaque( it.value().toObject(), outputSchema ) ) continue;
        outputs.append( it.key() );
    }

    QStringList requiredFields;
    for ( const QJsonValue& value : inputSchema.value( "required" ).toArray() )
        requiredFields.append( value.toString() );

    const QJsonObject configValues = task.value( "config_values" ).toObject();

    QJsonArray configFields;
    for ( const QJsonValue& value : task.value( "config_fields" ).toArray() )
    {
        const QString fieldName = value.toString();
        if ( fieldName.isEmpty() ) continue;
        configFields.append( configFieldSchema( fieldName, inputSchema, requiredFields, configValues ) );
    }

    return QJsonObject{ { "name", task.value( "name" ).toString() },
                        { "inputs", inputs },
                        { "outputs", outputs },
                        { "config_fields", configFields } };
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowDescribeTools::graphFromDescribe( const QJsonObject& describe )
{
    QJsonArray tasks;
    QJsonArray edges;
    for ( const QJsonValue& value : describe.value( "tasks" ).toArray() )
    {
        const QJsonObject task = value.toObject();
        tasks.append( taskFromDescribe( task ) );
        appendDependencyEdges( edges, task.value( "name" ).toString(), task.value( "depends_on" ) );
    }

    return QJsonObject{ { "name", describe.value( "workflow" ).toString() }, { "description", "" }, { "tasks", tasks }, { "edges", edges } };
}

//--------------------------------------------------------------------------------------------------
/// Error message from a `{"status": "invalid", "error": {...}}` document, empty if there is none
//--------------------------------------------------------------------------------------------------
QString RimWorkflowDescribeTools::errorFromDescribe( const QJsonObject& describe )
{
    if ( describe.value( "status" ).toString() != "invalid" ) return {};

    const QJsonObject error   = describe.value( "error" ).toObject();
    QString           message = error.value( "message" ).toString( "Workflow configuration is invalid" );

    QStringList   details;
    const QString task = error.value( "task" ).toString();
    if ( !task.isEmpty() ) details.append( QString( "task '%1'" ).arg( task ) );
    const QString field = error.value( "field" ).toString();
    if ( !field.isEmpty() ) details.append( QString( "field '%1'" ).arg( field ) );
    if ( !details.isEmpty() ) message += " (" + details.join( ", " ) + ")";

    return message;
}

//--------------------------------------------------------------------------------------------------
/// Map a Python class path such as `rips.generated.generated_classes.WellPath` to the ResInsight
/// object type used to select the field binding. Returns an empty string for unsupported types.
//--------------------------------------------------------------------------------------------------
QString RimWorkflowDescribeTools::resinsightTypeFromPythonType( const QString& pythonType )
{
    const int separator = pythonType.lastIndexOf( '.' );
    if ( separator < 0 ) return {};

    const QString module = pythonType.left( separator );
    if ( module != "rips" && !module.startsWith( "rips." ) ) return {};

    static const QMap<QString, QString> typeMap = { { "Case", "EclipseCase" },
                                                    { "Reservoir", "EclipseCase" },
                                                    { "EclipseCase", "EclipseCase" },
                                                    { "WellPath", "WellPath" },
                                                    { "View", "View" },
                                                    { "EclipseView", "View" } };
    return typeMap.value( pythonType.mid( separator + 1 ) );
}
