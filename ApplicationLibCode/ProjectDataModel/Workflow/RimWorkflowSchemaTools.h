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

#include <QJsonObject>
#include <QString>
#include <QStringList>

//==================================================================================================
/// Helpers for the JSON schemas that taskmaestro generates for task input and output models
//==================================================================================================
namespace RimWorkflowSchemaTools
{
QJsonObject resolveRef( const QString& ref, const QJsonObject& rootSchema );
QJsonObject resolveReferences( QJsonObject schema, const QJsonObject& rootSchema );
QJsonObject underlyingSchema( QJsonObject schema, const QJsonObject& rootSchema );
bool        isOpaque( const QJsonObject& schema, const QJsonObject& rootSchema );
QString     objectPythonType( const QJsonObject& typeSchema, const QJsonObject& rootSchema );
QString     jsonType( const QJsonObject& typeSchema );
QString     resinsightTypeFromPythonType( const QString& pythonType );
QStringList requiredFields( const QJsonObject& modelSchema );
QStringList propertyNames( const QJsonObject& modelSchema );
bool        isConfigurable( const QJsonObject& property, const QJsonObject& rootSchema );

QJsonObject configFieldSchema( const QString&     fieldName,
                               const QJsonObject& inputSchema,
                               const QStringList& requiredFields,
                               const QJsonObject& configValues );
} // namespace RimWorkflowSchemaTools
