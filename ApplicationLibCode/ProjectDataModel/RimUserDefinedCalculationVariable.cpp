/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimUserDefinedCalculationVariable.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimUserDefinedCalculationVariable, "RimUserDefinedCalculationVariable" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculationVariable::RimUserDefinedCalculationVariable()
{
    CAF_PDM_InitObject( "RimUserDefinedCalculationVariable", ":/octave.png" );

    CAF_PDM_InitFieldNoDefault( &m_name, "VariableName", "Variable Name" );
    m_name.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_addressUi, "AddressUi", "Address" );
    m_addressUi.registerGetMethod( this, &RimUserDefinedCalculationVariable::displayString );
    m_addressUi.xmlCapability()->disableIO();
    m_addressUi.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimUserDefinedCalculationVariable::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedCalculationVariable::setName( const QString& name )
{
    m_name = name;
}
