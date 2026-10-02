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

#include "RimWorkflowCaseBinding.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseTools.h"
#include "RimTools.h"

#include <QJsonObject>

CAF_PDM_SOURCE_INIT( RimWorkflowCaseBinding, "WorkflowCaseBinding" );

RimWorkflowCaseBinding::RimWorkflowCaseBinding()
{
    CAF_PDM_InitFieldNoDefault( &m_case, "Case", "Case" );
}

QString RimWorkflowCaseBinding::displayValue() const
{
    return m_case() ? m_case()->caseUserDescription() : "(not selected)";
}

QJsonValue RimWorkflowCaseBinding::toJsonValue() const
{
    if ( m_case() == nullptr ) return QJsonValue::Null;
    return QJsonObject{ { "__resinsight_ref__", "EclipseCase" }, { "case_id", m_case()->caseId() } };
}

QList<caf::PdmOptionItemInfo> RimWorkflowCaseBinding::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_case ) RimTools::eclipseCaseOptionItems( &options );
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowCaseBinding::isObjectReference() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowCaseBinding::applySchema( const QJsonObject& fieldSchema )
{
    RimWorkflowFieldBinding::applySchema( fieldSchema );
    const QJsonObject reference = fieldSchema.value( "default" ).toObject();
    if ( reference.value( "__resinsight_ref__" ).toString() != "EclipseCase" || !reference.contains( "case_id" ) ) return;

    const int caseId = reference.value( "case_id" ).toInt( -1 );
    for ( RimEclipseCase* eclipseCase : RimEclipseCaseTools::eclipseCases() )
    {
        if ( eclipseCase && eclipseCase->caseId() == caseId ) m_case = eclipseCase;
    }
}
