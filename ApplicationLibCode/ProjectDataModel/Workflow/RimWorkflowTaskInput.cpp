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

#include "RimWorkflowTaskInput.h"

#include "RimWorkflowBoolBinding.h"
#include "RimWorkflowCaseBinding.h"
#include "RimWorkflowNumberBinding.h"
#include "RimWorkflowStringBinding.h"
#include "RimWorkflowViewBinding.h"
#include "RimWorkflowWellPathBinding.h"

#include <QJsonArray>
#include <QJsonObject>

CAF_PDM_SOURCE_INIT( RimWorkflowTaskInput, "WorkflowTaskInput" );

namespace
{
RimWorkflowFieldBinding* createBinding( const QJsonObject& schema )
{
    const QString resinsightType = schema.value( "resinsight_type" ).toString();
    if ( resinsightType == "EclipseCase" ) return new RimWorkflowCaseBinding;
    if ( resinsightType == "WellPath" ) return new RimWorkflowWellPathBinding;
    if ( resinsightType == "View" ) return new RimWorkflowViewBinding;

    const QString type = schema.value( "type" ).toString( "string" );
    if ( type == "boolean" ) return new RimWorkflowBoolBinding;
    if ( type == "number" || type == "integer" ) return new RimWorkflowNumberBinding;
    return new RimWorkflowStringBinding;
}
} // namespace

RimWorkflowTaskInput::RimWorkflowTaskInput()
{
    CAF_PDM_InitObject( "Task", ":/Bullet.png" );
    CAF_PDM_InitFieldNoDefault( &m_items, "Bindings", "" );

    CAF_PDM_InitFieldNoDefault( &m_taskName, "TaskName", "Task" );
    m_taskName.uiCapability()->setUiReadOnly( true );
}

QString RimWorkflowTaskInput::taskName() const
{
    return m_taskName();
}

void RimWorkflowTaskInput::setTaskName( const QString& name )
{
    m_taskName = name;
    setUiName( name );
}

void RimWorkflowTaskInput::buildFromSchema( const QJsonArray& configFields )
{
    deleteAllItems();
    for ( const QJsonValue& v : configFields )
    {
        const QJsonObject schema  = v.toObject();
        auto*             binding = createBinding( schema );
        binding->applySchema( schema );
        addItem( binding );
    }
}

QString RimWorkflowTaskInput::toTaskYamlBlock() const
{
    QString out = m_taskName() + ":\n";
    if ( count() == 0 )
    {
        out += "  {}\n";
        return out;
    }
    for ( const RimWorkflowFieldBinding* b : items() )
    {
        out += QString( "  %1: %2\n" ).arg( b->fieldName(), b->toYamlValue() );
    }
    return out;
}
