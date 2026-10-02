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
#include <QString>

#include <expected>
#include <optional>
#include <vector>

//==================================================================================================
/// A connectable input or output of a task. An empty name is the whole input or output model.
//==================================================================================================
struct RimWorkflowPort
{
    QString     name{};
    QJsonObject schema{};
    QJsonObject rootSchema{};
    QString     description{};
    QString     typeName{};
    QString     iconResource{};
    bool        required     = true;
    bool        configurable = false;

    bool isWhole() const;
};

//==================================================================================================
/// Static type checks for workflow connections. These mirror taskmaestro's `_is_type_compatible`
/// on the JSON schemas; `taskmaestro workflow describe` has the final say.
//==================================================================================================
namespace RimWorkflowPortCompatibility
{
std::vector<RimWorkflowPort>   inputPorts( const QJsonObject& taskType );
std::vector<RimWorkflowPort>   outputPorts( const QJsonObject& taskType );
std::optional<RimWorkflowPort> inputPort( const QJsonObject& taskType, const QString& name );
std::optional<RimWorkflowPort> outputPort( const QJsonObject& taskType, const QString& name );

bool isCompatible( const QJsonObject& produced, const QJsonObject& producedRoot, const QJsonObject& expected, const QJsonObject& expectedRoot );
bool        isCompatible( const RimWorkflowPort& produced, const RimWorkflowPort& expected );
bool        isAny( const QJsonObject& schema, const QJsonObject& rootSchema );
QString     typeName( const QJsonObject& schema, const QJsonObject& rootSchema );
QString     iconResource( const QJsonObject& schema, const QJsonObject& rootSchema );
QJsonObject portTypes( const std::vector<RimWorkflowPort>& ports );
QJsonObject portIcons( const std::vector<RimWorkflowPort>& ports );

QStringList coveredByWholeOutput( const QJsonObject& upstreamTaskType, const QJsonObject& downstreamTaskType );
bool        wouldCreateCycle( const RimWorkflowDefinition& definition, const QString& from, const QString& to );

// Mapped tasks
QJsonObject mappedTaskType( const QJsonObject& taskType, const RimWorkflowTaskMap& map );
QJsonObject mappedOutputSchema( const QJsonObject& outputSchema );

// Collected inputs: the kind and element schema of a `list[T]` or `dict[str, T]` input
std::optional<std::pair<RimWorkflowDefinitionEdge::Collect, QJsonObject>> collectTarget( const RimWorkflowPort& expected );

// The edge to add for a new connection, collecting into a list or dict input when the upstream
// value is an element of it. An existing connection to the same input is replaced, except for
// other members of the same collection.
std::expected<RimWorkflowDefinitionEdge, QString> resolveConnection( const RimWorkflowDefinition& definition,
                                                                     const QString&               from,
                                                                     const QString&               output,
                                                                     const QString&               to,
                                                                     const QString&               input );
std::expected<void, QString>
    canConnect( const RimWorkflowDefinition& definition, const QString& from, const QString& output, const QString& to, const QString& input );
std::expected<void, QString> checkCollectMember( const RimWorkflowDefinition& definition, const RimWorkflowDefinitionEdge& edge );
} // namespace RimWorkflowPortCompatibility
