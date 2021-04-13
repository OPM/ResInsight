/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -     Equinor ASA
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

#include "RimProcess.h"

#include "cafPdmFieldCapability.h"

CAF_PDM_SOURCE_INIT( RimProcess, "RimProcess" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcess::RimProcess()
{
    CAF_PDM_InitObject( "ResInsight Process", ":/Erase.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_command, "Command", "Command", "", "", "" );
    m_command.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_arguments, "Arguments", "Arguments", "", "", "" );
    m_arguments.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProcess::~RimProcess()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                        QString                    uiConfigName,
                                        caf::PdmUiEditorAttribute* attribute )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProcess::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}
