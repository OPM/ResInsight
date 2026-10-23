/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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
#include "RimcFractureTemplate.h"

#include "RiaLogging.h"

#include "RimFractureTemplate.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFractureTemplate, RimcFractureTemplate_setScaleFactors, "SetScaleFactors" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFractureTemplate_setScaleFactors::RimcFractureTemplate_setScaleFactors( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Fracture Template Scale Factors", "", "", "Set Fracture Template Scale Factors." );

    CAF_PDM_InitScriptableField( &m_halfLength, "HalfLength", 1.0, "Half Length" );
    CAF_PDM_InitScriptableField( &m_height, "Height", 1.0, "Height" );
    CAF_PDM_InitScriptableField( &m_dFactor, "DFactor", 1.0, "D Factor" );
    CAF_PDM_InitScriptableField( &m_conductivity, "Conductivity", 1.0, "Conductivity" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimcFractureTemplate_setScaleFactors::setScaleFactors( double halfLength, double height, double dFactor, double conductivity )
{
    m_halfLength   = halfLength;
    m_height       = height;
    m_dFactor      = dFactor;
    m_conductivity = conductivity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcFractureTemplate_setScaleFactors::execute()
{
    if ( m_halfLength() <= 0.0 || m_height() <= 0.0 || m_dFactor() <= 0.0 || m_conductivity() <= 0.0 )
    {
        return std::unexpected( "Invalid scale factors." );
    }

    RimFractureTemplate* fractureTemplate = self<RimFractureTemplate>();
    if ( fractureTemplate )
    {
        fractureTemplate->setScaleFactors( m_halfLength, m_height, m_dFactor, m_conductivity );
        fractureTemplate->loadDataAndUpdateGeometryHasChanged();
    }

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFractureTemplate, RimFractureTemplate_setContainment, "setContainment" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplate_setContainment::RimFractureTemplate_setContainment( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Fracture Containment", "", "", "Set the K layer range fractures from this template are contained within" );

    CAF_PDM_InitScriptableField( &m_topLayer, "TopLayer", -1, "Top Layer", "", "", "Zero-based K index of the top layer" );
    CAF_PDM_InitScriptableField( &m_baseLayer, "BaseLayer", -1, "Base Layer", "", "", "Zero-based K index of the base layer" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate_setContainment::setLayers( int topLayer, int baseLayer )
{
    m_topLayer  = topLayer;
    m_baseLayer = baseLayer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimFractureTemplate_setContainment::execute()
{
    auto* fractureTemplate = self<RimFractureTemplate>();
    if ( !fractureTemplate ) return std::unexpected( "No fracture template is available." );

    if ( m_topLayer() < 0 || m_baseLayer() < 0 ) return std::unexpected( "Top and base layer must be non-negative." );
    if ( m_topLayer() > m_baseLayer() ) return std::unexpected( "Top layer must not be below the base layer." );

    fractureTemplate->setContainmentTopKLayer( m_topLayer() );
    fractureTemplate->setContainmentBaseKLayer( m_baseLayer() );
    fractureTemplate->loadDataAndUpdateGeometryHasChanged();

    return nullptr;
}
