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

#include "RimWorkflowValidationTools.h"

#include <QJsonArray>

namespace
{
RimWorkflowIssue errorIssue( const QString& task, const QString& field, const QString& message )
{
    return { .task = task, .field = field, .severity = RimWorkflowIssue::Severity::Error, .message = message };
}

void appendErrorIssues( const QJsonObject& error, const QStringList& nodeNames, std::vector<RimWorkflowIssue>& issues )
{
    QString message = error.value( "message" ).toString();
    if ( message.isEmpty() ) message = "The workflow is not valid";

    QString task = error.value( "task" ).toString();
    if ( !nodeNames.contains( task ) ) task = RimWorkflowValidationTools::taskFromMessage( message, nodeNames );

    const QJsonArray fieldIssues = error.value( "issues" ).toArray();
    if ( fieldIssues.isEmpty() || task.isEmpty() )
    {
        issues.push_back( errorIssue( task, error.value( "field" ).toString(), message ) );
        return;
    }

    for ( const QJsonValue& value : fieldIssues )
    {
        const QJsonObject fieldIssue = value.toObject();
        const QString     code       = fieldIssue.value( "code" ).toString();
        issues.push_back(
            errorIssue( task, fieldIssue.value( "field" ).toString(), code.isEmpty() ? message : QString( "%1 (%2)" ).arg( message, code ) ) );
    }
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowValidationTools::Result RimWorkflowValidationTools::issuesFromHelperResult( const QJsonObject& helperResult,
                                                                                       const QStringList& nodeNames )
{
    if ( helperResult.value( "status" ).toString() == "invalid" )
    {
        Result result;
        result.describeSucceeded = false;
        appendErrorIssues( helperResult.value( "error" ).toObject(), nodeNames, result.issues );
        return result;
    }
    return issuesFromDescribe( helperResult.value( "describe" ).toObject(), helperResult.value( "describe_error" ).toObject(), nodeNames );
}

//--------------------------------------------------------------------------------------------------
/// Errors from describe, and warnings for config fields without a value
//--------------------------------------------------------------------------------------------------
RimWorkflowValidationTools::Result RimWorkflowValidationTools::issuesFromDescribe( const QJsonObject& describe,
                                                                                   const QJsonObject& describeError,
                                                                                   const QStringList& nodeNames )
{
    Result result;
    if ( !describeError.isEmpty() )
    {
        result.describeSucceeded = false;
        appendErrorIssues( describeError, nodeNames, result.issues );
        return result;
    }

    for ( const QJsonValue& value : describe.value( "tasks" ).toArray() )
    {
        const QJsonObject task = value.toObject();
        for ( const QJsonValue& field : task.value( "missing_config_fields" ).toArray() )
        {
            result.issues.push_back( { .task     = task.value( "name" ).toString(),
                                       .field    = field.toString(),
                                       .severity = RimWorkflowIssue::Severity::Warning,
                                       .message  = QString( "Input '%1' has no value" ).arg( field.toString() ) } );
        }
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/// The node whose name is quoted in the message, preferring the longest name
//--------------------------------------------------------------------------------------------------
QString RimWorkflowValidationTools::taskFromMessage( const QString& message, const QStringList& nodeNames )
{
    QString found;
    for ( const QString& name : nodeNames )
    {
        const bool quoted = message.contains( QString( "'%1'" ).arg( name ) ) || message.contains( QString( "\"%1\"" ).arg( name ) );
        if ( quoted && name.size() > found.size() ) found = name;
    }
    return found;
}
