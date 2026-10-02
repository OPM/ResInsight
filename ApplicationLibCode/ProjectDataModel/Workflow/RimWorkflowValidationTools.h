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

#include "RimWorkflowDefinition.h"

#include <QJsonObject>
#include <QStringList>

#include <vector>

//==================================================================================================
/// Turns the result of `taskmaestro_helper save --describe` (or `load --describe`) into issues
/// attached to the tasks of the workflow. Issues with an empty task belong to the workflow.
//==================================================================================================
namespace RimWorkflowValidationTools
{
struct Result
{
    std::vector<RimWorkflowIssue> issues;
    bool                          describeSucceeded = true;
};

Result  issuesFromHelperResult( const QJsonObject& helperResult, const QStringList& nodeNames );
Result  issuesFromDescribe( const QJsonObject& describe, const QJsonObject& describeError, const QStringList& nodeNames );
QString taskFromMessage( const QString& message, const QStringList& nodeNames );
} // namespace RimWorkflowValidationTools
