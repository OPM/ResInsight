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
#include <QJsonDocument>
#include <QJsonValue>
#include <QMap>

#include <algorithm>

namespace
{
//==================================================================================================
/// Records the key order of every object named `properties` in a JSON text, by the path of the
/// object. The text must be valid JSON.
//==================================================================================================
class PropertyOrderScanner
{
public:
    explicit PropertyOrderScanner( const QByteArray& json )
        : m_json( json )
    {
    }

    QMap<QString, QStringList> scan()
    {
        m_pos = 0;
        m_orders.clear();
        scanValue( QString() );
        return m_orders;
    }

    static QString childPath( const QString& path, const QString& key )
    {
        QString segment = key;
        segment.replace( "~", "~0" ).replace( "/", "~1" );
        return path + "/" + segment;
    }

private:
    void skipWhitespace()
    {
        while ( m_pos < m_json.size() && QByteArrayView( " \t\r\n" ).contains( m_json[m_pos] ) )
            ++m_pos;
    }

    QString scanString()
    {
        const qsizetype start      = m_pos++;
        bool            hasEscapes = false;
        while ( m_pos < m_json.size() && m_json[m_pos] != '"' )
        {
            if ( m_json[m_pos] == '\\' )
            {
                hasEscapes = true;
                ++m_pos;
            }
            ++m_pos;
        }
        ++m_pos;

        const QByteArray quoted = m_json.mid( start, m_pos - start );
        if ( !hasEscapes ) return QString::fromUtf8( quoted.mid( 1, quoted.size() - 2 ) );
        return QJsonDocument::fromJson( "[" + quoted + "]" ).array().first().toString();
    }

    void scanValue( const QString& path )
    {
        skipWhitespace();
        if ( m_pos >= m_json.size() ) return;

        const char c = m_json[m_pos];
        if ( c == '{' )
        {
            ++m_pos;
            QStringList keys;
            while ( true )
            {
                skipWhitespace();
                if ( m_pos >= m_json.size() || m_json[m_pos] == '}' ) break;
                const QString key = scanString();
                skipWhitespace();
                ++m_pos; // ':'
                scanValue( childPath( path, key ) );
                keys.append( key );
                skipWhitespace();
                if ( m_pos < m_json.size() && m_json[m_pos] == ',' ) ++m_pos;
            }
            ++m_pos;
            if ( path.endsWith( "/properties" ) ) m_orders.insert( path, keys );
        }
        else if ( c == '[' )
        {
            ++m_pos;
            for ( int index = 0;; ++index )
            {
                skipWhitespace();
                if ( m_pos >= m_json.size() || m_json[m_pos] == ']' ) break;
                scanValue( path + "/" + QString::number( index ) );
                skipWhitespace();
                if ( m_pos < m_json.size() && m_json[m_pos] == ',' ) ++m_pos;
            }
            ++m_pos;
        }
        else if ( c == '"' )
        {
            scanString();
        }
        else
        {
            while ( m_pos < m_json.size() && !QByteArrayView( ",}] \t\r\n" ).contains( m_json[m_pos] ) )
                ++m_pos;
        }
    }

    const QByteArray&          m_json;
    qsizetype                  m_pos = 0;
    QMap<QString, QStringList> m_orders;
};

//--------------------------------------------------------------------------------------------------
/// Where a JSON value is: in data around the schemas, in a schema, or in the `properties` map of a schema
//--------------------------------------------------------------------------------------------------
enum class SchemaLocation
{
    Data,
    Schema,
    PropertiesMap
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SchemaLocation childLocation( SchemaLocation location, const QString& key )
{
    if ( location == SchemaLocation::PropertiesMap ) return SchemaLocation::Schema;
    if ( location == SchemaLocation::Schema ) return key == "properties" ? SchemaLocation::PropertiesMap : SchemaLocation::Schema;
    return ( key == "input_schema" || key == "output_schema" ) ? SchemaLocation::Schema : SchemaLocation::Data;
}

//--------------------------------------------------------------------------------------------------
/// Add the property order to the object schemas under `input_schema` and `output_schema`. Literal
/// values in the schemas (defaults, examples) and the data around the schemas are left as they are.
//--------------------------------------------------------------------------------------------------
QJsonValue withPropertyOrder( const QJsonValue& value, const QString& path, SchemaLocation location, const QMap<QString, QStringList>& orders )
{
    if ( value.isArray() )
    {
        QJsonArray array = value.toArray();
        for ( qsizetype i = 0; i < array.size(); ++i )
            array[i] = withPropertyOrder( array[i], path + "/" + QString::number( i ), location, orders );
        return array;
    }
    if ( !value.isObject() ) return value;

    static const QStringList literalKeywords = { "default", "examples", "const", "enum" };

    QJsonObject object = value.toObject();
    for ( auto it = object.begin(); it != object.end(); ++it )
    {
        if ( location == SchemaLocation::Schema && literalKeywords.contains( it.key() ) ) continue;
        it.value() =
            withPropertyOrder( it.value(), PropertyOrderScanner::childPath( path, it.key() ), childLocation( location, it.key() ), orders );
    }

    const QString propertiesPath = PropertyOrderScanner::childPath( path, "properties" );
    if ( location == SchemaLocation::Schema && object.value( "type" ).toString() == "object" && object.value( "properties" ).isObject() &&
         orders.contains( propertiesPath ) )
    {
        object.insert( RimWorkflowSchemaTools::propertyOrderKey(), QJsonArray::fromStringList( orders.value( propertiesPath ) ) );
    }
    return object;
}

//--------------------------------------------------------------------------------------------------
/// Whether the schema allows null, e.g. Optional[int] (anyOf with null)
//--------------------------------------------------------------------------------------------------
bool isNullable( const QJsonObject& schema, const QJsonObject& rootSchema, int depth = 0 )
{
    constexpr int maxDepth = 16;
    if ( depth > maxDepth ) return false;
    if ( schema.contains( "$ref" ) )
        return isNullable( RimWorkflowSchemaTools::resolveRef( schema.value( "$ref" ).toString(), rootSchema ), rootSchema, depth + 1 );

    const QJsonValue type = schema.value( "type" );
    if ( type.toString() == "null" || type.toArray().contains( QJsonValue( "null" ) ) ) return true;

    for ( const QString& keyword : { "anyOf", "oneOf" } )
    {
        for ( const QJsonValue& arm : schema.value( keyword ).toArray() )
        {
            if ( isNullable( arm.toObject(), rootSchema, depth + 1 ) ) return true;
        }
    }
    const QJsonArray allOf = schema.value( "allOf" ).toArray();
    return allOf.size() == 1 && isNullable( allOf.first().toObject(), rootSchema, depth + 1 );
}
} // namespace
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

    // int | float
    const QJsonArray anyOf = typeSchema.value( "anyOf" ).toArray();
    if ( !anyOf.isEmpty() && std::all_of( anyOf.begin(),
                                          anyOf.end(),
                                          []( const QJsonValue& arm )
                                          {
                                              const QString armType = arm.toObject().value( "type" ).toString();
                                              return armType == "integer" || armType == "number";
                                          } ) )
        return "number";

    // Any: the value is typed as a literal
    for ( const char* key : { "type", "anyOf", "oneOf", "allOf", "enum", "const", "$ref", "properties", "items" } )
    {
        if ( typeSchema.contains( key ) ) return "string";
    }
    return "any";
}

//--------------------------------------------------------------------------------------------------
/// A value typed for an untyped (Any) field, read like a YAML scalar: true/false, numbers,
/// null/None/~ and JSON arrays, objects and quoted strings. Other text is a string; empty text is null.
//--------------------------------------------------------------------------------------------------
QJsonValue RimWorkflowSchemaTools::literalFromText( const QString& text )
{
    const QString token = text.trimmed();
    const QString lower = token.toLower();
    if ( token.isEmpty() || lower == "null" || lower == "none" || token == "~" ) return QJsonValue( QJsonValue::Null );
    if ( lower == "true" ) return true;
    if ( lower == "false" ) return false;

    const QJsonDocument document = QJsonDocument::fromJson( ( "[" + token + "]" ).toUtf8() );
    if ( document.isArray() && document.array().size() == 1 ) return document.array().first();
    return token;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowSchemaTools::explicitNullKey()
{
    return "explicit_null";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowSchemaTools::propertyOrderKey()
{
    return "x-ri-property-order";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonDocument RimWorkflowSchemaTools::parseWithPropertyOrder( const QByteArray& json, QJsonParseError* error )
{
    QJsonParseError     parseError{};
    const QJsonDocument document = QJsonDocument::fromJson( json, &parseError );
    if ( error ) *error = parseError;
    if ( parseError.error != QJsonParseError::NoError || document.isNull() ) return document;

    const auto orders = PropertyOrderScanner( json ).scan();
    if ( orders.isEmpty() ) return document;

    const QJsonValue root      = document.isObject() ? QJsonValue( document.object() ) : QJsonValue( document.array() );
    const QJsonValue annotated = withPropertyOrder( root, QString(), SchemaLocation::Data, orders );
    return annotated.isObject() ? QJsonDocument( annotated.toObject() ) : QJsonDocument( annotated.toArray() );
}

//--------------------------------------------------------------------------------------------------
/// The text that `literalFromText` reads back as `value`; null is written as `null`
//--------------------------------------------------------------------------------------------------
QString RimWorkflowSchemaTools::textFromLiteral( const QJsonValue& value )
{
    if ( value.isString() && literalFromText( value.toString() ) == value ) return value.toString();
    if ( value.isUndefined() ) return {};

    const QByteArray json = QJsonDocument( QJsonArray{ value } ).toJson( QJsonDocument::Compact );
    return QString::fromUtf8( json.mid( 1, json.size() - 2 ) );
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
/// Property names in declaration order when the schema has it, else required fields first
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowSchemaTools::propertyNames( const QJsonObject& modelSchema )
{
    // QJsonObject sorts its keys; the `required` list keeps model order of the required fields
    QStringList       names;
    const QJsonObject properties = modelSchema.value( "properties" ).toObject();
    for ( const QJsonValue& value : modelSchema.value( propertyOrderKey() ).toArray() )
    {
        const QString name = value.toString();
        if ( properties.contains( name ) && !names.contains( name ) ) names.append( name );
    }
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
    {
        entry["default"] = configValues.value( fieldName );
        // A null in input.yaml is an entered None, unlike a model default of None
        if ( entry["default"].isNull() ) entry[explicitNullKey()] = true;
    }
    else if ( property.contains( "default" ) )
        entry["default"] = property.value( "default" );

    const QString format = typeSchema.value( "format" ).toString();
    if ( !format.isEmpty() ) entry["format"] = format;

    if ( !resinsightType.isEmpty() ) entry["resinsight_type"] = resinsightType;

    return entry;
}
