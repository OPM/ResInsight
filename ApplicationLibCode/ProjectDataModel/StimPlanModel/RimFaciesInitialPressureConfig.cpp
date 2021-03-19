/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include "RimFaciesInitialPressureConfig.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiLineEditor.h"

CAF_PDM_SOURCE_INIT( RimFaciesInitialPressureConfig, "FaciesInitialPressureConfig" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaciesInitialPressureConfig::RimFaciesInitialPressureConfig()
    : changed( this )
{
    m_isChecked.uiCapability()->setUiHidden( false );

    CAF_PDM_InitFieldNoDefault( &m_faciesName, "FaciesName", "Facies", "", "", "" );
    m_faciesName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_faciesValue, "FaciesValue", "Value", "", "", "" );
    m_faciesValue.uiCapability()->setUiHidden( true );

    // Use unicode for delta letter
#ifdef Q_OS_WIN
    QString deltaPressureFractionString = QString::fromStdWString( L"\x0394" ) + " Pressure Fraction";
#else
    QString deltaPressureFractionString = QString::fromUtf8( "\u0394 Pressure Fraction" );
#endif

    CAF_PDM_InitField( &m_fraction, "Fraction", 1.0, deltaPressureFractionString, "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaciesInitialPressureConfig::~RimFaciesInitialPressureConfig()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesInitialPressureConfig::setEnabled( bool enable )
{
    m_isChecked = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaciesInitialPressureConfig::isEnabled() const
{
    return m_isChecked;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesInitialPressureConfig::setFaciesName( const QString& name )
{
    m_faciesName = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimFaciesInitialPressureConfig::faciesName() const
{
    return m_faciesName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesInitialPressureConfig::setFaciesValue( int faciesValue )
{
    m_faciesValue = faciesValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFaciesInitialPressureConfig::faciesValue() const
{
    return m_faciesValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesInitialPressureConfig::setFraction( double fraction )
{
    m_fraction = fraction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaciesInitialPressureConfig::fraction() const
{
    return m_fraction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaciesInitialPressureConfig::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                       const QVariant&            oldValue,
                                                       const QVariant&            newValue )
{
    changed.send();
}
