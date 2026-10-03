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

#include "RimWorkflowArrayBinding.h"
#include "RimWorkflowBoolBinding.h"
#include "RimWorkflowCaseBinding.h"
#include "RimWorkflowDateBinding.h"
#include "RimWorkflowFilePathBinding.h"
#include "RimWorkflowFloatBinding.h"
#include "RimWorkflowIntBinding.h"
#include "RimWorkflowMappingBinding.h"
#include "RimWorkflowStringBinding.h"
#include "RimWorkflowViewBinding.h"
#include "RimWorkflowWellPathBinding.h"

#include "cafPdmUiOrdering.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include <QJsonArray>

CAF_PDM_SOURCE_INIT( RimWorkflowTaskInput, "WorkflowTaskInput" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowTaskInput::RimWorkflowTaskInput()
{
    CAF_PDM_InitObject( "Task", ":/Bullet.png" );
    CAF_PDM_InitFieldNoDefault( &m_items, "Bindings", "" );

    CAF_PDM_InitFieldNoDefault( &m_taskName, "TaskName", "Task" );
    m_taskName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_taskType, "TaskType", "Task Type" );
    m_taskType.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_taskDescription, "TaskDescription", "Description" );
    m_taskDescription.uiCapability()->setUiReadOnly( true );
    m_taskDescription.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowTaskInput::taskName() const
{
    return m_taskName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowTaskInput::setTaskName( const QString& name )
{
    m_taskName = name;
    setUiName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowTaskInput::taskType() const
{
    return m_taskType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowTaskInput::setTaskInfo( const QString& taskType, const QString& description )
{
    m_taskType        = taskType;
    m_taskDescription = description;
}

//--------------------------------------------------------------------------------------------------
/// The class keyword of the binding used for a config field schema
//--------------------------------------------------------------------------------------------------
QString RimWorkflowTaskInput::bindingClassKeyword( const QJsonObject& fieldSchema )
{
    if ( fieldSchema.value( "map_over" ).toBool() ) return RimWorkflowMappingBinding::classKeywordStatic();

    const QString resinsightType = fieldSchema.value( "resinsight_type" ).toString();
    if ( resinsightType == "EclipseCase" ) return RimWorkflowCaseBinding::classKeywordStatic();
    if ( resinsightType == "WellPath" ) return RimWorkflowWellPathBinding::classKeywordStatic();
    if ( resinsightType == "View" ) return RimWorkflowViewBinding::classKeywordStatic();

    const QString type   = fieldSchema.value( "type" ).toString( "string" );
    const QString format = fieldSchema.value( "format" ).toString();
    if ( type == "string" && format == "date" ) return RimWorkflowDateBinding::classKeywordStatic();
    if ( type == "string" && ( format == "path" || format == "file-path" || format == "directory-path" ) )
        return RimWorkflowFilePathBinding::classKeywordStatic();
    if ( type == "boolean" ) return RimWorkflowBoolBinding::classKeywordStatic();
    if ( type == "integer" ) return RimWorkflowIntBinding::classKeywordStatic();
    if ( type == "number" ) return RimWorkflowFloatBinding::classKeywordStatic();
    if ( type == "array" ) return RimWorkflowArrayBinding::classKeywordStatic();
    return RimWorkflowStringBinding::classKeywordStatic();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowFieldBinding* RimWorkflowTaskInput::createBinding( const QJsonObject& fieldSchema )
{
    const QString keyword = bindingClassKeyword( fieldSchema );
    if ( keyword == RimWorkflowCaseBinding::classKeywordStatic() ) return new RimWorkflowCaseBinding;
    if ( keyword == RimWorkflowWellPathBinding::classKeywordStatic() ) return new RimWorkflowWellPathBinding;
    if ( keyword == RimWorkflowViewBinding::classKeywordStatic() ) return new RimWorkflowViewBinding;
    if ( keyword == RimWorkflowDateBinding::classKeywordStatic() ) return new RimWorkflowDateBinding;
    if ( keyword == RimWorkflowFilePathBinding::classKeywordStatic() ) return new RimWorkflowFilePathBinding;
    if ( keyword == RimWorkflowBoolBinding::classKeywordStatic() ) return new RimWorkflowBoolBinding;
    if ( keyword == RimWorkflowIntBinding::classKeywordStatic() ) return new RimWorkflowIntBinding;
    if ( keyword == RimWorkflowFloatBinding::classKeywordStatic() ) return new RimWorkflowFloatBinding;
    if ( keyword == RimWorkflowArrayBinding::classKeywordStatic() ) return new RimWorkflowArrayBinding;
    if ( keyword == RimWorkflowMappingBinding::classKeywordStatic() ) return new RimWorkflowMappingBinding;
    return new RimWorkflowStringBinding;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowTaskInput::detachedKey( const QString& fieldName, const QString& classKeyword )
{
    return fieldName + "/" + classKeyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowTaskInput::buildFromSchema( const QJsonArray& configFields )
{
    QMap<QString, QJsonValue> noDetachedValues;
    deleteAllItems();
    syncFromSchema( configFields, noDetachedValues );
}

//--------------------------------------------------------------------------------------------------
/// Update the bindings to a new list of config fields. A binding is kept with its value when the
/// field still has the same kind of binding. Values of removed bindings are moved to
/// `detachedValues`, and restored from there when the field comes back.
//--------------------------------------------------------------------------------------------------
void RimWorkflowTaskInput::syncFromSchema( const QJsonArray& configFields, QMap<QString, QJsonValue>& detachedValues )
{
    std::vector<RimWorkflowFieldBinding*> existing = items();
    m_items.clearWithoutDelete();

    for ( const QJsonValue& value : configFields )
    {
        QJsonObject   schema  = value.toObject();
        const QString field   = schema.value( "name" ).toString();
        const QString keyword = bindingClassKeyword( schema );

        RimWorkflowFieldBinding* binding = nullptr;
        for ( auto it = existing.begin(); it != existing.end(); ++it )
        {
            if ( *it && ( *it )->fieldName() == field && ( *it )->classKeyword() == keyword )
            {
                binding = *it;
                existing.erase( it );
                break;
            }
        }

        if ( binding )
        {
            const QJsonValue current = binding->toJsonValue();
            if ( !current.isNull() ) schema["default"] = current;
        }
        else
        {
            binding           = createBinding( schema );
            const QString key = detachedKey( field, keyword );
            if ( detachedValues.contains( key ) ) schema["default"] = detachedValues.take( key );
        }
        binding->applySchema( schema );
        addItem( binding );
    }

    for ( RimWorkflowFieldBinding* removed : existing )
    {
        if ( !removed ) continue;
        const QJsonValue value = removed->toJsonValue();
        if ( !value.isNull() ) detachedValues.insert( detachedKey( removed->fieldName(), removed->classKeyword() ), value );
        delete removed;
    }
}

//--------------------------------------------------------------------------------------------------
/// All values that are set, including references to ResInsight objects
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowTaskInput::jsonValues() const
{
    QJsonObject values;
    for ( const RimWorkflowFieldBinding* binding : items() )
    {
        const QJsonValue value = binding->toJsonValue();
        if ( !value.isNull() ) values.insert( binding->fieldName(), value );
    }
    return values;
}

//--------------------------------------------------------------------------------------------------
/// Values that can be stored in input.yaml, without references to objects in the project
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflowTaskInput::literalValues() const
{
    QJsonObject values;
    for ( const RimWorkflowFieldBinding* binding : items() )
    {
        if ( binding->isObjectReference() ) continue;
        const QJsonValue value = binding->toJsonValue();
        if ( !value.isNull() ) values.insert( binding->fieldName(), value );
    }
    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowTaskInput::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( !m_taskType().isEmpty() ) uiOrdering.add( &m_taskType );
    if ( !m_taskDescription().isEmpty() ) uiOrdering.add( &m_taskDescription );

    for ( RimWorkflowFieldBinding* binding : items() )
    {
        if ( binding && binding->valueField() ) uiOrdering.add( binding->valueField() );
    }
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowTaskInput::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.skipRemainingChildren( true );
}
