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
#include <QJsonValue>
#include <QString>
#include <QStringList>

#include <optional>
#include <vector>

//==================================================================================================
/// A mapped task (`map:` in YAML) runs once per entry of the job config value `over`, a mapping
/// with string keys. Each key goes to the input field `keyAs` and each value to `valueAs`.
/// The output is `dict[str, Output]` keyed like `over`.
//==================================================================================================
struct RimWorkflowTaskMap
{
    QString over{};
    QString keyAs{};
    QString valueAs{};
    QString errorMode = "fail_fast";

    bool operator==( const RimWorkflowTaskMap& other ) const = default;
};

//==================================================================================================
/// One task instance in a workflow
//==================================================================================================
struct RimWorkflowDefinitionNode
{
    QString     name{};
    QString     taskId{};
    QStringList configFields{};

    // Fields the user wants to configure even when they are optional or cannot be derived
    QStringList explicitConfigFields{};

    std::optional<RimWorkflowTaskMap> map{};

    // Keys of the YAML entry the editor does not model (nested workflow); kept for read-only display
    QJsonObject extra{};

    bool operator==( const RimWorkflowDefinitionNode& other ) const = default;
};

//==================================================================================================
/// A connection between two tasks. An empty output or input means the whole model.
/// Several edges can collect outputs into one `list[T]` or `dict[str, T]` input (`collect:` in
/// YAML). List members are ordered like the edges; dict members have a key.
//==================================================================================================
struct RimWorkflowDefinitionEdge
{
    enum class Collect
    {
        None,
        List,
        Dict
    };

    QString from{};
    QString output{};
    QString to{};
    QString input{};
    Collect collect = Collect::None;
    QString key{};

    bool isWholeInput() const;
    bool isWholeOutput() const;
    bool isCollected() const;
    bool sameConnection( const RimWorkflowDefinitionEdge& other ) const;

    bool operator==( const RimWorkflowDefinitionEdge& other ) const = default;
};

//==================================================================================================
/// A problem found by static validation or by `taskmaestro workflow describe`
//==================================================================================================
struct RimWorkflowIssue
{
    enum class Severity
    {
        Error,
        Warning
    };

    QString  task{};
    QString  field{};
    Severity severity = Severity::Error;
    QString  message{};

    bool operator==( const RimWorkflowIssue& other ) const = default;
};

//==================================================================================================
/// Editable model of a taskmaestro workflow, exchanged as JSON with `rips.taskmaestro_helper`.
/// A plain value type: copies are cheap enough to serve as undo snapshots.
//==================================================================================================
struct RimWorkflowDefinition
{
    int         format = 1;
    QString     name;
    QString     resultTask;
    QString     headerComment;
    QJsonObject passthrough;
    QJsonObject source;

    std::vector<RimWorkflowDefinitionNode> nodes;
    std::vector<RimWorkflowDefinitionEdge> edges;

    // Task instance name -> { field: value }
    QJsonObject inputs;

    // Task id -> descriptor { id, name, python_type, description, input_schema, output_schema }
    QJsonObject taskTypes;

    bool        editable = true;
    QStringList readOnlyReasons;
    QStringList warnings;

    QJsonObject describe;
    QJsonObject describeError;

    const RimWorkflowDefinitionNode*       findNode( const QString& nodeName ) const;
    RimWorkflowDefinitionNode*             findNode( const QString& nodeName );
    int                                    nodeIndex( const QString& nodeName ) const;
    QJsonObject                            taskType( const QString& taskId ) const;
    QJsonObject                            taskTypeForNode( const QString& nodeName ) const;
    std::vector<RimWorkflowDefinitionEdge> incomingEdges( const QString& nodeName ) const;
    std::vector<RimWorkflowDefinitionEdge> outgoingEdges( const QString& nodeName ) const;
    QStringList                            nodeNames() const;

    bool operator==( const RimWorkflowDefinition& other ) const = default;
};
