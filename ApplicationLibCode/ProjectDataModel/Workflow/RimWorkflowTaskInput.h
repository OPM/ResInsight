/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimWorkflowFieldBinding.h"

#include "cafPdmObjectCollection.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QMap>

class QJsonArray;

//==================================================================================================
/// The input values of one task instance in a job. There is one per task in the workflow; tasks
/// without config fields have no bindings and are hidden.
//==================================================================================================
class RimWorkflowTaskInput : public caf::PdmObjectCollection<RimWorkflowFieldBinding>
{
    CAF_PDM_HEADER_INIT;

public:
    RimWorkflowTaskInput();

    QString taskName() const;
    void    setTaskName( const QString& name );
    QString taskType() const;
    void    setTaskInfo( const QString& taskType, const QString& description );

    void buildFromSchema( const QJsonArray& configFields );
    void syncFromSchema( const QJsonArray& configFields, QMap<QString, QJsonValue>& detachedValues );

    QJsonObject jsonValues() const;
    QJsonObject literalValues() const;

    static QString                  bindingClassKeyword( const QJsonObject& fieldSchema );
    static RimWorkflowFieldBinding* createBinding( const QJsonObject& fieldSchema );
    static QString                  detachedKey( const QString& fieldName, const QString& classKeyword );

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmField<QString> m_taskName;
    caf::PdmField<QString> m_taskType;
    caf::PdmField<QString> m_taskDescription;
};
