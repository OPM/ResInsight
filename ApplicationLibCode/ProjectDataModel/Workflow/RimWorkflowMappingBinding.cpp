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

#include "RimWorkflowMappingBinding.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseTools.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPath.h"

#include "cafPdmUiTreeSelectionEditor.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

CAF_PDM_SOURCE_INIT( RimWorkflowMappingBinding, "WorkflowMappingBinding" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowMappingBinding::RimWorkflowMappingBinding()
{
    CAF_PDM_InitFieldNoDefault( &m_valueType, "ValueType", "Value Type" );
    m_valueType.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_text, "Text", "Items" );
    m_text.uiCapability()->setUiToolTip( "A JSON object, for example {\"low\": 0.5, \"high\": 2.0}" );

    CAF_PDM_InitFieldNoDefault( &m_cases, "Cases", "Cases" );
    CAF_PDM_InitFieldNoDefault( &m_wellPaths, "WellPaths", "Well Paths" );
    CAF_PDM_InitFieldNoDefault( &m_views, "Views", "Views" );
    for ( caf::PdmFieldHandle* field : std::initializer_list<caf::PdmFieldHandle*>{ &m_cases, &m_wellPaths, &m_views } )
    {
        field->uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
        field->uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWorkflowMappingBinding::valueField()
{
    if ( m_valueType() == "EclipseCase" ) return &m_cases;
    if ( m_valueType() == "WellPath" ) return &m_wellPaths;
    if ( m_valueType() == "View" ) return &m_views;
    return &m_text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowMappingBinding::isObjectReference() const
{
    return !m_valueType().isEmpty();
}

//--------------------------------------------------------------------------------------------------
/// The selected objects as `{key: reference}`, keyed by their names made unique
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QJsonObject>> RimWorkflowMappingBinding::references() const
{
    std::vector<std::pair<QString, QJsonObject>> items;
    if ( m_valueType() == "EclipseCase" )
    {
        for ( RimEclipseCase* eclipseCase : m_cases.ptrReferencedObjectsByType() )
        {
            if ( eclipseCase )
                items.push_back( { eclipseCase->caseUserDescription(),
                                   QJsonObject{ { "__resinsight_ref__", "EclipseCase" }, { "case_id", eclipseCase->caseId() } } } );
        }
    }
    else if ( m_valueType() == "WellPath" )
    {
        for ( RimWellPath* wellPath : m_wellPaths.ptrReferencedObjectsByType() )
        {
            if ( wellPath )
                items.push_back(
                    { wellPath->name(), QJsonObject{ { "__resinsight_ref__", "WellPath" }, { "well_path_name", wellPath->name() } } } );
        }
    }
    else if ( m_valueType() == "View" )
    {
        for ( RimEclipseView* view : m_views.ptrReferencedObjectsByType() )
        {
            if ( view ) items.push_back( { view->name(), QJsonObject{ { "__resinsight_ref__", "View" }, { "view_id", view->id() } } } );
        }
    }

    QSet<QString> used;
    for ( auto& [key, reference] : items )
    {
        const QString base = key.isEmpty() ? QString( "item" ) : key;
        key                = base;
        for ( int index = 2; used.contains( key ); ++index )
            key = QString( "%1_%2" ).arg( base ).arg( index );
        used.insert( key );
    }
    return items;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonValue RimWorkflowMappingBinding::toJsonValue() const
{
    if ( isObjectReference() )
    {
        QJsonObject mapping;
        for ( const auto& [key, reference] : references() )
            mapping[key] = reference;
        return mapping.isEmpty() ? QJsonValue( QJsonValue::Null ) : QJsonValue( mapping );
    }

    if ( !hasValue() ) return QJsonValue::Null;
    const QJsonDocument document = QJsonDocument::fromJson( m_text().toUtf8() );
    return document.isObject() ? QJsonValue( document.object() ) : QJsonValue( m_text() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowMappingBinding::displayValue() const
{
    QStringList keys;
    if ( isObjectReference() )
    {
        for ( const auto& [key, reference] : references() )
            keys.append( key );
    }
    else
    {
        const QJsonValue value = toJsonValue();
        if ( !value.isObject() ) return hasValue() ? "(not a JSON object)" : "(not set)";
        keys = value.toObject().keys();
    }
    if ( keys.isEmpty() ) return "(no items)";

    const QString count = keys.size() == 1 ? QString( "1 item" ) : QString( "%1 items" ).arg( keys.size() );
    QString       names = keys.mid( 0, 3 ).join( ", " );
    if ( keys.size() > 3 ) names += ", …";
    return QString( "%1: %2" ).arg( count, names );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWorkflowMappingBinding::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_cases ) RimTools::eclipseCaseOptionItems( &options );
    if ( fieldNeedingOptions == &m_wellPaths ) RimTools::wellPathOptionItems( &options );
    if ( fieldNeedingOptions == &m_views )
    {
        for ( RimEclipseCase* eclipseCase : RimEclipseCaseTools::eclipseCases() )
        {
            if ( !eclipseCase ) continue;
            for ( RimEclipseView* view : eclipseCase->reservoirViews() )
            {
                if ( view )
                    options.push_back(
                        caf::PdmOptionItemInfo( QString( "%1 / %2" ).arg( eclipseCase->caseUserDescription(), view->name() ), view ) );
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowMappingBinding::applySchema( const QJsonObject& fieldSchema )
{
    m_valueType = fieldSchema.value( "value_resinsight_type" ).toString();
    RimWorkflowFieldBinding::applySchema( fieldSchema );

    const QJsonObject mapping = fieldSchema.value( "default" ).toObject();
    if ( !isObjectReference() )
    {
        if ( fieldSchema.value( "default" ).isObject() )
            m_text = QString::fromUtf8( QJsonDocument( mapping ).toJson( QJsonDocument::Compact ) );
        return;
    }

    // Restore the objects of a mapping of references
    QSet<int>     caseIds;
    QSet<int>     viewIds;
    QSet<QString> wellPathNames;
    for ( const QJsonValue& value : mapping )
    {
        const QJsonObject reference = value.toObject();
        if ( reference.value( "__resinsight_ref__" ).toString() == "EclipseCase" )
            caseIds.insert( reference.value( "case_id" ).toInt( -1 ) );
        if ( reference.value( "__resinsight_ref__" ).toString() == "View" ) viewIds.insert( reference.value( "view_id" ).toInt( -1 ) );
        if ( reference.value( "__resinsight_ref__" ).toString() == "WellPath" )
            wellPathNames.insert( reference.value( "well_path_name" ).toString() );
    }
    if ( caseIds.isEmpty() && viewIds.isEmpty() && wellPathNames.isEmpty() ) return;

    std::vector<RimEclipseCase*> cases;
    std::vector<RimEclipseView*> views;
    for ( RimEclipseCase* eclipseCase : RimEclipseCaseTools::eclipseCases() )
    {
        if ( !eclipseCase ) continue;
        if ( caseIds.contains( eclipseCase->caseId() ) ) cases.push_back( eclipseCase );
        for ( RimEclipseView* view : eclipseCase->reservoirViews() )
        {
            if ( view && viewIds.contains( view->id() ) ) views.push_back( view );
        }
    }
    std::vector<RimWellPath*> wellPaths;
    if ( auto* project = RimProject::current() )
    {
        for ( RimWellPath* wellPath : project->allWellPaths() )
        {
            if ( wellPath && wellPathNames.contains( wellPath->name() ) ) wellPaths.push_back( wellPath );
        }
    }
    m_cases.setValue( cases );
    m_views.setValue( views );
    m_wellPaths.setValue( wellPaths );
}
