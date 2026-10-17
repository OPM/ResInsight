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

#include "RimWorkflowViewBinding.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseTools.h"
#include "RimEclipseView.h"
#include "RimProject.h"

#include <QJsonObject>

CAF_PDM_SOURCE_INIT( RimWorkflowViewBinding, "WorkflowViewBinding" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowViewBinding::RimWorkflowViewBinding()
{
    CAF_PDM_InitFieldNoDefault( &m_view, "View", "View" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowViewBinding::displayValue() const
{
    return m_view() ? m_view()->name() : "(not selected)";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonValue RimWorkflowViewBinding::toJsonValue() const
{
    if ( m_view() == nullptr ) return QJsonValue::Null;
    return QJsonObject{ { "__resinsight_ref__", "View" }, { "view_id", m_view()->id() } };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWorkflowViewBinding::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions != &m_view ) return options;

    auto* project = RimProject::current();
    if ( project == nullptr ) return options;

    for ( RimEclipseCase* eclipseCase : RimEclipseCaseTools::eclipseCases() )
    {
        if ( eclipseCase == nullptr ) continue;
        for ( RimEclipseView* view : eclipseCase->reservoirViews() )
        {
            if ( view == nullptr ) continue;
            QString label = QString( "%1 / %2" ).arg( eclipseCase->caseUserDescription(), view->name() );
            options.push_back( caf::PdmOptionItemInfo( label, view ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowViewBinding::isObjectReference() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowViewBinding::applySchema( const QJsonObject& fieldSchema )
{
    RimWorkflowFieldBinding::applySchema( fieldSchema );
    const QJsonObject reference = fieldSchema.value( "default" ).toObject();
    if ( reference.value( "__resinsight_ref__" ).toString() != "View" || !reference.contains( "view_id" ) ) return;

    const int viewId = reference.value( "view_id" ).toInt( -1 );
    for ( RimEclipseCase* eclipseCase : RimEclipseCaseTools::eclipseCases() )
    {
        if ( !eclipseCase ) continue;
        for ( RimEclipseView* view : eclipseCase->reservoirViews() )
        {
            if ( view && view->id() == viewId ) m_view = view;
        }
    }
}
