/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// +		m_addWellName	{m_fieldValue=true m_defaultFieldValue=true }	caf::PdmField<bool>

//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimNameConfig.h"
#include "Rim3dWellLogCurve.h"
#include "RimProject.h"

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_ABSTRACT_SOURCE_INIT( RimNameConfig, "RimCurveNameConfig" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimNameConfig::RimNameConfig( const QString& customName )
{
    CAF_PDM_InitObject( "Curve Name Generator" );

    CAF_PDM_InitFieldNoDefault( &m_customName, "CustomCurveName", "Custom Name Part" );
    CAF_PDM_InitFieldNoDefault( &m_autoName, "AutoCurveName", "Full Name" );
    m_autoName.registerGetMethod( this, &RimNameConfig::autoName );
    m_autoName.xmlCapability()->disableIO();
    m_autoName.uiCapability()->setUiReadOnly( true );

    m_customName = customName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimNameConfig::~RimNameConfig()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimNameConfig::customName() const
{
    return m_customName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimNameConfig::nameField()
{
    return &m_autoName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimNameConfig::name() const
{
    return m_autoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNameConfig::uiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNameConfig::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_customName );
    uiOrdering.add( &m_autoName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNameConfig::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                      const QVariant&            oldValue,
                                      const QVariant&            newValue )
{
    updateAllSettings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimNameConfig::autoName() const
{
    RimNameConfigHolderInterface* plotHolder;
    this->firstAncestorOrThisOfTypeAsserted( plotHolder );
    return plotHolder->createAutoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNameConfig::setCustomName( const QString& name )
{
    m_customName = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNameConfig::enableAllAutoNameTags( bool enable )
{
    doEnableAllAutoNameTags( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNameConfig::updateAllSettings()
{
    m_autoName.uiCapability()->updateConnectedEditors();
    m_customName.uiCapability()->updateConnectedEditors();

    RimNameConfigHolderInterface* holder;
    this->firstAncestorOrThisOfTypeAsserted( holder );
    holder->updateAutoName();
    caf::PdmObject* pdmObject = dynamic_cast<caf::PdmObject*>( holder );
    if ( pdmObject )
    {
        pdmObject->updateConnectedEditors();
    }
}
