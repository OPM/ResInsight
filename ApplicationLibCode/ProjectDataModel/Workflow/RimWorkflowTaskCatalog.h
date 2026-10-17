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

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <vector>

//==================================================================================================
/// Task types and registered workflows published by installed packages (`taskmaestro_helper catalog`).
/// The process-wide instance is loaded on first use and cached until `invalidate()` (Rescan).
//==================================================================================================
class RimWorkflowTaskCatalog
{
public:
    static RimWorkflowTaskCatalog fromJson( const QJsonObject& catalog );
    static RimWorkflowTaskCatalog failed( const QString& errorMessage );

    static const RimWorkflowTaskCatalog& instance();
    static void                          invalidate();
    static void                          setInstance( const RimWorkflowTaskCatalog& catalog );

    bool        isValid() const;
    QString     loadError() const;
    QStringList pluginErrors() const;
    QString     taskmaestroVersion() const;

    QJsonObject              taskType( const QString& taskId ) const;
    std::vector<QJsonObject> tasks() const;
    QJsonArray               registeredWorkflows() const;

    void merge( const QJsonObject& taskTypes );

private:
    bool        m_valid = false;
    QString     m_loadError;
    QStringList m_pluginErrors;
    QString     m_taskmaestroVersion;
    QJsonObject m_taskTypes;
    QStringList m_taskOrder;
    QJsonArray  m_workflows;
};
