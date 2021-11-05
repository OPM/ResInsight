/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimViewNameConfig.h"

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimViewNameConfig, "RimViewNameConfig" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewNameConfig::RimViewNameConfig()
    : RimNameConfig( "" )
    , m_hideCaseNameField( false )
    , m_hideAggregationTypeField( false )
    , m_hidePropertyField( false )
    , m_hideSampleSpacingField( false )
{
    CAF_PDM_InitObject( "View Name Generator" );

    CAF_PDM_InitField( &m_addCaseName, "AddCaseName", false, "Add Case Name" );
    CAF_PDM_InitField( &m_addAggregationType, "AddAggregationType", true, "Add Aggregation Type" );
    CAF_PDM_InitField( &m_addProperty, "AddProperty", true, "Add Property Type" );
    CAF_PDM_InitField( &m_addSampleSpacing, "AddSampleSpacing", false, "Add Sample Spacing" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::setAddCaseName( bool add )
{
    m_addCaseName = add;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewNameConfig::addCaseName() const
{
    return m_addCaseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::setAddAggregationType( bool add )
{
    m_addAggregationType = add;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewNameConfig::addAggregationType() const
{
    return m_addAggregationType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::setAddProperty( bool add )
{
    m_addProperty = add;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewNameConfig::addProperty() const
{
    return m_addProperty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::setAddSampleSpacing( bool add )
{
    m_addSampleSpacing = add;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewNameConfig::addSampleSpacing() const
{
    return m_addSampleSpacing();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::doEnableAllAutoNameTags( bool enable )
{
    m_addCaseName        = enable;
    m_addAggregationType = enable;
    m_addProperty        = enable;
    m_addSampleSpacing   = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::hideCaseNameField( bool hide )
{
    m_hideCaseNameField = hide;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::hideAggregationTypeField( bool hide )
{
    m_hideAggregationTypeField = hide;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::hidePropertyField( bool hide )
{
    m_hidePropertyField = hide;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::hideSampleSpacingField( bool hide )
{
    m_hideSampleSpacingField = hide;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimNameConfig::defineUiOrdering( uiConfigName, uiOrdering );
    if ( !m_hideCaseNameField ) uiOrdering.add( &m_addCaseName );
    if ( !m_hideAggregationTypeField ) uiOrdering.add( &m_addAggregationType );
    if ( !m_hidePropertyField ) uiOrdering.add( &m_addProperty );
    if ( !m_hideSampleSpacingField ) uiOrdering.add( &m_addSampleSpacing );
}
