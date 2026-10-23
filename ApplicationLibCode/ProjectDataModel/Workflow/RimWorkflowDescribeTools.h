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

//==================================================================================================
/// Converts the output of `taskmaestro workflow describe --json` into the workflow graph used by
/// the Workflow UI: { name, description, tasks: [{ name, inputs, outputs, config_fields }], edges }
//==================================================================================================
namespace RimWorkflowDescribeTools
{
QJsonObject graphFromDescribe( const QJsonObject& describe );
QString     errorFromDescribe( const QJsonObject& describe );
QString     resinsightTypeFromPythonType( const QString& pythonType );
} // namespace RimWorkflowDescribeTools
