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
#include <QMap>
#include <QString>
#include <QStringList>

#include <expected>
#include <optional>
#include <vector>

class RimWorkflowTaskCatalog;

//==================================================================================================
/// Pure functions on RimWorkflowDefinition: JSON conversion, config derivation, edit operations,
/// static validation and the graph JSON shown by RiuWorkflowGraphView.
/// Edit operations return the new definition, or an error message for the user.
//==================================================================================================
namespace RimWorkflowDefinitionTools
{
using EditResult = std::expected<RimWorkflowDefinition, QString>;

constexpr const char* connectTaskId = "resinsight.connect";

// Conversion
std::expected<RimWorkflowDefinition, QString> fromJson( const QJsonObject& json );
QJsonObject                                   toJson( const RimWorkflowDefinition& definition );
RimWorkflowDefinition                         createEmpty( const QString& name );
RimWorkflowDefinition                         editableCopy( const RimWorkflowDefinition& definition, const QString& name );

// Names
bool    isIdentifier( const QString& name );
QString identifierFrom( const QString& text );
QString uniqueInstanceName( const RimWorkflowDefinition& definition, const QString& baseName );
std::expected<void, QString>
            validateInstanceName( const RimWorkflowDefinition& definition, const QString& name, const QString& currentName = {} );
QStringList sinkTasks( const RimWorkflowDefinition& definition );
QString     effectiveResultTask( const RimWorkflowDefinition& definition );
QStringList structuralReadOnlyReasons( const RimWorkflowDefinition& definition );

// Config fields
QStringList           coveredInputFields( const RimWorkflowDefinition& definition, const QString& nodeName );
QStringList           deriveConfigFields( const RimWorkflowDefinition& definition, const QString& nodeName );
RimWorkflowDefinition normalize( RimWorkflowDefinition definition );

// Edit operations
EditResult addTask( const RimWorkflowDefinition&  definition,
                    const RimWorkflowTaskCatalog& catalog,
                    const QString&                taskId,
                    QString*                      addedName = nullptr );
EditResult removeTask( const RimWorkflowDefinition& definition, const QString& nodeName );
EditResult connect( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionEdge& edge );
EditResult disconnect( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionEdge& edge );
EditResult setCollectKey( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionEdge& edge, const QString& key );
EditResult moveCollectMember( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionEdge& edge, int delta );
EditResult setTaskMap( const RimWorkflowDefinition& definition, const QString& nodeName, const std::optional<RimWorkflowTaskMap>& map );
std::expected<void, QString> checkTaskMap( const RimWorkflowDefinition& definition, const QString& nodeName, const RimWorkflowTaskMap& map );
QJsonObject mapOverSchema( const RimWorkflowDefinition& definition, const QString& nodeName );
EditResult  renameTask( const RimWorkflowDefinition& definition, const QString& oldName, const QString& newName );
EditResult  renameWorkflow( const RimWorkflowDefinition& definition, const QString& newName );
EditResult  setResultTask( const RimWorkflowDefinition& definition, const QString& nodeName );
EditResult setOptionalInputConfigured( const RimWorkflowDefinition& definition, const QString& nodeName, const QString& field, bool configured );
RimWorkflowDefinition
    autoWireResInsightInputs( const RimWorkflowDefinition& definition, const RimWorkflowTaskCatalog& catalog, const QString& nodeName );

// Validation and display
std::vector<RimWorkflowIssue> validate( const RimWorkflowDefinition& definition );
QJsonObject graphFromDefinition( const RimWorkflowDefinition& definition, bool editMode, const std::vector<RimWorkflowIssue>& extraIssues = {} );
} // namespace RimWorkflowDefinitionTools
