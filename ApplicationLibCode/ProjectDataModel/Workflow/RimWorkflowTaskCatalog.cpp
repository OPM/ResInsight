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

#include "RimWorkflowTaskCatalog.h"

#include "RimWorkflowHelperProcess.h"

#include "RiaLogging.h"

#include <optional>

namespace
{
constexpr int catalogTimeoutMs = 120000;

std::optional<RimWorkflowTaskCatalog>& cachedCatalog()
{
    static std::optional<RimWorkflowTaskCatalog> catalog;
    return catalog;
}

RimWorkflowTaskCatalog loadCatalog()
{
    auto result = RimWorkflowHelperProcess::runSync( { "catalog" }, {}, catalogTimeoutMs );
    if ( !result )
    {
        RiaLogging::warning( QString( "Could not load the workflow task catalog: %1" ).arg( result.error() ).toStdString() );
        return RimWorkflowTaskCatalog::failed( result.error() );
    }

    auto catalog = RimWorkflowTaskCatalog::fromJson( result.value() );
    for ( const QString& error : catalog.pluginErrors() )
        RiaLogging::warning( QString( "Workflow plugin: %1" ).arg( error ).toStdString() );
    return catalog;
}

QString pluginErrorText( const QJsonValue& error )
{
    if ( error.isString() ) return error.toString();
    const QJsonObject object = error.toObject();
    const QString     source = object.value( "id" ).toString( object.value( "entry_point" ).toString() );
    const QString     text   = object.value( "message" ).toString();
    return source.isEmpty() ? text : source + ": " + text;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowTaskCatalog RimWorkflowTaskCatalog::fromJson( const QJsonObject& catalog )
{
    RimWorkflowTaskCatalog result;
    result.m_valid              = true;
    result.m_taskmaestroVersion = catalog.value( "taskmaestro_version" ).toString();
    result.m_workflows          = catalog.value( "workflows" ).toArray();
    for ( const QJsonValue& error : catalog.value( "errors" ).toArray() )
        result.m_pluginErrors.append( pluginErrorText( error ) );

    for ( const QJsonValue& value : catalog.value( "tasks" ).toArray() )
    {
        const QJsonObject task = value.toObject();
        const QString     id   = task.value( "id" ).toString();
        if ( id.isEmpty() || result.m_taskTypes.contains( id ) ) continue;
        result.m_taskTypes.insert( id, task );
        result.m_taskOrder.append( id );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowTaskCatalog RimWorkflowTaskCatalog::failed( const QString& errorMessage )
{
    RimWorkflowTaskCatalog result;
    result.m_loadError = errorMessage;
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWorkflowTaskCatalog& RimWorkflowTaskCatalog::instance()
{
    auto& catalog = cachedCatalog();
    if ( !catalog ) catalog = loadCatalog();
    return *catalog;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowTaskCatalog::invalidate()
{
    cachedCatalog().reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowTaskCatalog::setInstance( const RimWorkflowTaskCatalog& catalog )
{
    cachedCatalog() = catalog;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowTaskCatalog::isValid() const
{
    return m_valid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowTaskCatalog::loadError() const
{
    return m_loadError;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowTaskCatalog::pluginErrors() const
{
    return m_pluginErrors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowTaskCatalog::taskmaestroVersion() const
{
    return m_taskmaestroVersion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowTaskCatalog::taskType( const QString& taskId ) const
{
    return m_taskTypes.value( taskId ).toObject();
}

//--------------------------------------------------------------------------------------------------
/// Task descriptors in catalog order
//--------------------------------------------------------------------------------------------------
std::vector<QJsonObject> RimWorkflowTaskCatalog::tasks() const
{
    std::vector<QJsonObject> result;
    result.reserve( m_taskOrder.size() );
    for ( const QString& id : m_taskOrder )
        result.push_back( m_taskTypes.value( id ).toObject() );
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonArray RimWorkflowTaskCatalog::registeredWorkflows() const
{
    return m_workflows;
}

//--------------------------------------------------------------------------------------------------
/// Add task types (from a workflow definition) that are not in the catalog
//--------------------------------------------------------------------------------------------------
void RimWorkflowTaskCatalog::merge( const QJsonObject& taskTypes )
{
    for ( auto it = taskTypes.begin(); it != taskTypes.end(); ++it )
    {
        if ( m_taskTypes.contains( it.key() ) || !it.value().isObject() ) continue;
        m_taskTypes.insert( it.key(), it.value() );
        m_taskOrder.append( it.key() );
    }
}
