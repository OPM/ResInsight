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

#include "RimWorkflowWellPathBinding.h"

#include "RimTools.h"
#include "RimWellPath.h"

CAF_PDM_SOURCE_INIT( RimWorkflowWellPathBinding, "WorkflowWellPathBinding" );

RimWorkflowWellPathBinding::RimWorkflowWellPathBinding()
{
    CAF_PDM_InitFieldNoDefault( &m_wellPath, "WellPath", "Well Path" );
}

QString RimWorkflowWellPathBinding::toYamlValue() const
{
    if ( m_wellPath() == nullptr ) return "null";
    QString name = m_wellPath()->name();
    name.replace( "\"", "\\\"" );
    return QString( "{__resinsight_ref__: WellPath, well_path_name: \"%1\"}" ).arg( name );
}

QList<caf::PdmOptionItemInfo> RimWorkflowWellPathBinding::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_wellPath ) RimTools::wellPathOptionItems( &options );
    return options;
}
