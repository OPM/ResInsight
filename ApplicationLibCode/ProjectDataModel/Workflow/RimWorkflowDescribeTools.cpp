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

#include "RimWorkflowPortCompatibility.h"
#include "RimWorkflowSchemaTools.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QStringList>

namespace
{
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
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
        if ( it.key() == "value" && RimWorkflowSchemaTools::isOpaque( it.value().toObject(), outputSchema ) ) continue;
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
        configFields.append( RimWorkflowSchemaTools::configFieldSchema( fieldName, inputSchema, requiredFields, configValues ) );
    }

    return QJsonObject{ { "name", task.value( "name" ).toString() },
                        { "inputs", inputs },
                        { "outputs", outputs },
                        { "config_fields", configFields },
                        { "input_types", RimWorkflowPortCompatibility::portTypes( RimWorkflowPortCompatibility::inputPorts( task ) ) },
                        { "output_types", RimWorkflowPortCompatibility::portTypes( RimWorkflowPortCompatibility::outputPorts( task ) ) } };
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
    return RimWorkflowSchemaTools::resinsightTypeFromPythonType( pythonType );
}
