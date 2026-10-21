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

#include "RimWorkflowSchemaTools.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QMap>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowSchemaTools::resolveRef( const QString& ref, const QJsonObject& rootSchema )
{
    const QString prefix = "#/$defs/";
    if ( !ref.startsWith( prefix ) ) return {};
    return rootSchema.value( "$defs" ).toObject().value( ref.mid( prefix.size() ) ).toObject();
}

//--------------------------------------------------------------------------------------------------
/// Follow $ref and unwrap single-item allOf, keeping the sibling keywords of a $ref (description, default)
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowSchemaTools::resolveReferences( QJsonObject schema, const QJsonObject& rootSchema )
{
    constexpr int maxDepth = 16;
    for ( int depth = 0; depth < maxDepth; ++depth )
    {
        if ( schema.contains( "$ref" ) )
        {
            QJsonObject target = resolveRef( schema.value( "$ref" ).toString(), rootSchema );
            schema.remove( "$ref" );
            for ( auto it = schema.begin(); it != schema.end(); ++it )
            {
                if ( !target.contains( it.key() ) ) target.insert( it.key(), it.value() );
            }
            schema = target;
            continue;
        }

        const QJsonArray allOf = schema.value( "allOf" ).toArray();
        if ( allOf.size() == 1 )
        {
            schema = allOf.first().toObject();
            continue;
        }
        break;
    }
    return schema;
}

//--------------------------------------------------------------------------------------------------
/// Follow $ref and unwrap single-item allOf and Optional (anyOf with null) to the underlying type
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowSchemaTools::underlyingSchema( QJsonObject schema, const QJsonObject& rootSchema )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowSchemaTools::isOpaque( const QJsonObject& schema, const QJsonObject& rootSchema )
{
    return underlyingSchema( schema, rootSchema ).value( "x-taskmaestro-opaque" ).toBool( false );
}

//--------------------------------------------------------------------------------------------------
/// Python type of a runtime-only object, either bare or wrapped in an ObjectModel `value` field
//--------------------------------------------------------------------------------------------------
QString RimWorkflowSchemaTools::objectPythonType( const QJsonObject& typeSchema, const QJsonObject& rootSchema )
{
    if ( typeSchema.contains( "x-taskmaestro-python-type" ) ) return typeSchema.value( "x-taskmaestro-python-type" ).toString();

    const QJsonObject valueSchema = typeSchema.value( "properties" ).toObject().value( "value" ).toObject();
    if ( valueSchema.isEmpty() ) return {};
    return underlyingSchema( valueSchema, rootSchema ).value( "x-taskmaestro-python-type" ).toString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowSchemaTools::jsonType( const QJsonObject& typeSchema )
{
    const QString type = typeSchema.value( "type" ).toString();
    if ( type == "boolean" || type == "integer" || type == "number" || type == "array" ) return type;
    return "string";
}

//--------------------------------------------------------------------------------------------------
/// taskmaestro_resinsight.models.Vec3 is a plain pydantic model (x, y, z numbers), so it has no
/// x-taskmaestro-python-type tag; recognize it structurally instead.
//--------------------------------------------------------------------------------------------------
bool RimWorkflowSchemaTools::isVec3Schema( const QJsonObject& typeSchema )
{
    if ( typeSchema.value( "type" ).toString() != "object" ) return false;

    const QJsonObject properties = typeSchema.value( "properties" ).toObject();
    if ( properties.size() != 3 ) return false;

    for ( const QString& component : QStringList{ "x", "y", "z" } )
    {
        if ( properties.value( component ).toObject().value( "type" ).toString() != "number" ) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// Map a Python class path such as `rips.generated.generated_classes.WellPath` to the ResInsight
/// object type used to select the field binding. Returns an empty string for unsupported types.
//--------------------------------------------------------------------------------------------------
QString RimWorkflowSchemaTools::resinsightTypeFromPythonType( const QString& pythonType )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowSchemaTools::requiredFields( const QJsonObject& modelSchema )
{
    QStringList fields;
    for ( const QJsonValue& value : modelSchema.value( "required" ).toArray() )
        fields.append( value.toString() );
    return fields;
}

//--------------------------------------------------------------------------------------------------
/// Property names in schema order
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowSchemaTools::propertyNames( const QJsonObject& modelSchema )
{
    // QJsonObject sorts its keys; the `required` list and the original document keep model order
    QStringList       names;
    const QJsonObject properties = modelSchema.value( "properties" ).toObject();
    for ( const QString& name : requiredFields( modelSchema ) )
    {
        if ( properties.contains( name ) && !names.contains( name ) ) names.append( name );
    }
    for ( const QString& name : properties.keys() )
    {
        if ( !names.contains( name ) ) names.append( name );
    }
    return names;
}

//--------------------------------------------------------------------------------------------------
/// Whether a value for the input property can be entered in ResInsight (a literal or an object reference)
//--------------------------------------------------------------------------------------------------
bool RimWorkflowSchemaTools::isConfigurable( const QJsonObject& property, const QJsonObject& rootSchema )
{
    const QJsonObject typeSchema = underlyingSchema( property, rootSchema );
    const QString     pythonType = objectPythonType( typeSchema, rootSchema );
    if ( !pythonType.isEmpty() )
    {
        // An object model with more required fields than `value` cannot be given as a single object
        QStringList required = requiredFields( typeSchema );
        required.removeAll( "value" );
        return required.isEmpty() && !resinsightTypeFromPythonType( pythonType ).isEmpty();
    }
    if ( typeSchema.value( "x-taskmaestro-opaque" ).toBool( false ) ) return false;
    if ( isVec3Schema( typeSchema ) ) return true;
    return typeSchema.value( "type" ).toString() != "object";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowSchemaTools::configFieldSchema( const QString&     fieldName,
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

    const QJsonObject property       = properties.value( fieldName ).toObject();
    const QJsonObject typeSchema     = underlyingSchema( property, inputSchema );
    QString           resinsightType = resinsightTypeFromPythonType( objectPythonType( typeSchema, inputSchema ) );
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
