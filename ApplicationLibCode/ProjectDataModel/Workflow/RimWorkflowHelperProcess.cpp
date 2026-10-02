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

#include "RimWorkflowHelperProcess.h"

#include "RimWorkflow.h"

#include <QDir>
#include <QJsonDocument>
#include <QProcess>

namespace
{
constexpr int startTimeoutMs = 10000;
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<QJsonObject, QString>
    RimWorkflowHelperProcess::runSync( const QStringList& args, const QByteArray& stdinData, int timeoutMs, const QString& extraPythonPath )
{
    const QString python = RimWorkflow::findPythonExecutable();
    if ( python.isEmpty() ) return std::unexpected( "No usable Python interpreter found" );

    QProcess process;
    process.setProcessEnvironment( environment( extraPythonPath ) );
    process.start( python, helperArguments( args ) );
    if ( !process.waitForStarted( startTimeoutMs ) ) return std::unexpected( QString( "Could not launch '%1'" ).arg( python ) );

    if ( !stdinData.isEmpty() ) process.write( stdinData );
    process.closeWriteChannel();

    if ( !process.waitForFinished( timeoutMs ) )
    {
        process.kill();
        process.waitForFinished( startTimeoutMs );
        return std::unexpected( QString( "The workflow helper timed out (%1)" ).arg( args.value( 0 ) ) );
    }

    const int exitCode = process.exitStatus() == QProcess::NormalExit ? process.exitCode() : -1;
    return parseOutput( exitCode, process.readAllStandardOutput(), process.readAllStandardError() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflowHelperProcess::helperArguments( const QStringList& args )
{
    return QStringList{ "-m", "rips.taskmaestro_helper" } + args;
}

//--------------------------------------------------------------------------------------------------
/// The process environment, with an extra folder in front of PYTHONPATH (to import workflow-local modules)
//--------------------------------------------------------------------------------------------------
QProcessEnvironment RimWorkflowHelperProcess::environment( const QString& extraPythonPath )
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if ( extraPythonPath.isEmpty() ) return env;

    const QString existing = env.value( "PYTHONPATH" );
    const QString path     = QDir::toNativeSeparators( extraPythonPath );
    env.insert( "PYTHONPATH", existing.isEmpty() ? path : path + QDir::listSeparator() + existing );
    return env;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<QJsonObject, QString>
    RimWorkflowHelperProcess::parseOutput( int exitCode, const QByteArray& standardOutput, const QByteArray& standardError )
{
    QJsonParseError     parseError{};
    const QJsonDocument document = QJsonDocument::fromJson( standardOutput.trimmed(), &parseError );
    if ( parseError.error == QJsonParseError::NoError && document.isObject() )
    {
        const QJsonObject result = document.object();
        if ( result.value( "status" ).toString() == "invalid" )
            return std::unexpected( errorMessage( result.value( "error" ).toObject() ) );
        if ( exitCode == 0 ) return result;
    }

    QString message = QString::fromUtf8( standardError ).trimmed();
    if ( message.isEmpty() ) message = QString::fromUtf8( standardOutput ).trimmed();
    // The last lines of a traceback hold the exception
    const QStringList lines = message.split( '\n', Qt::SkipEmptyParts );
    if ( lines.size() > 6 ) message = lines.mid( lines.size() - 6 ).join( '\n' );
    if ( message.isEmpty() ) message = QString( "The workflow helper failed with exit code %1" ).arg( exitCode );
    return std::unexpected( message );
}

//--------------------------------------------------------------------------------------------------
/// Message for an error object { message, task, field }
//--------------------------------------------------------------------------------------------------
QString RimWorkflowHelperProcess::errorMessage( const QJsonObject& error )
{
    QString message = error.value( "message" ).toString();
    if ( message.isEmpty() ) message = "Workflow configuration is invalid";

    QStringList   details;
    const QString task = error.value( "task" ).toString();
    if ( !task.isEmpty() && !message.contains( task ) ) details.append( QString( "task '%1'" ).arg( task ) );
    const QString field = error.value( "field" ).toString();
    if ( !field.isEmpty() && !message.contains( field ) ) details.append( QString( "field '%1'" ).arg( field ) );
    if ( !details.isEmpty() ) message += " (" + details.join( ", " ) + ")";
    return message;
}
