/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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
#include "RimcElasticProperties.h"

#include "RimElasticProperties.h"
#include "RimElasticPropertyScaling.h"
#include "RimElasticPropertyScalingCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimElasticProperties, RimcElasticProperties_addPropertyScaling, "AddPropertyScaling" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcElasticProperties_addPropertyScaling::RimcElasticProperties_addPropertyScaling( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Add Elastic Propery Scaling", "", "", "Add Elastic Property Scaling" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_formation, "Formation", "", "", "", "Formation" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_facies, "Facies", "", "", "", "Facies" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_property, "Property", "", "", "", "Property" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_scale, "Scale", "", "", "", "Scale" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcElasticProperties_addPropertyScaling::execute()
{
    RimElasticProperties* elasticProperties = self<RimElasticProperties>();

    RimElasticPropertyScalingCollection* scalingColl = elasticProperties->scalingCollection();
    if ( !scalingColl ) return nullptr;

    RimElasticPropertyScaling* propertyScaling = new RimElasticPropertyScaling;
    propertyScaling->setFormation( m_formation() );
    propertyScaling->setFacies( m_facies() );
    propertyScaling->setScale( m_scale() );

    caf::AppEnum<RiaDefines::CurveProperty> property;
    property.setFromText( m_property() );
    propertyScaling->setProperty( property );

    scalingColl->addElasticPropertyScaling( propertyScaling );
    scalingColl->updateConnectedEditors();

    return propertyScaling;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcElasticProperties_addPropertyScaling::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcElasticProperties_addPropertyScaling::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimElasticPropertyScaling );
}
