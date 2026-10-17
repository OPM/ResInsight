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

#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

#include <expected>

//==================================================================================================
/// Runs `python -m rips.taskmaestro_helper <args>`, which prints exactly one JSON document
//==================================================================================================
namespace RimWorkflowHelperProcess
{
constexpr int defaultTimeoutMs = 60000;

std::expected<QJsonObject, QString>
    runSync( const QStringList& args, const QByteArray& stdinData = {}, int timeoutMs = defaultTimeoutMs, const QString& extraPythonPath = {} );

QStringList                         helperArguments( const QStringList& args );
QProcessEnvironment                 environment( const QString& extraPythonPath );
std::expected<QJsonObject, QString> parseOutput( int exitCode, const QByteArray& standardOutput, const QByteArray& standardError );
QString                             errorMessage( const QJsonObject& error );
} // namespace RimWorkflowHelperProcess
