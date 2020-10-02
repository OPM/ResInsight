/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimElasticPropertyScalingCollection.h"

#include "RimElasticPropertyScaling.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimElasticPropertyScalingCollection, "ElasticPropertyScalingCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticPropertyScalingCollection::RimElasticPropertyScalingCollection()
{
    CAF_PDM_InitScriptableObject( "Elastic Property Scalings", "", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_elasticPropertyScalings,
                                          "ElasticPropertyScalings",
                                          "Elastic Property Scalings",
                                          "",
                                          "",
                                          "" );
    m_elasticPropertyScalings.uiCapability()->setUiHidden( true );
    m_elasticPropertyScalings.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticPropertyScalingCollection::~RimElasticPropertyScalingCollection()
{
    m_elasticPropertyScalings.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimElasticPropertyScaling*> RimElasticPropertyScalingCollection::elasticPropertyScalings() const
{
    std::vector<RimElasticPropertyScaling*> templates;
    for ( auto& templ : m_elasticPropertyScalings )
    {
        templates.push_back( templ );
    }
    return templates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScalingCollection::addElasticPropertyScaling( RimElasticPropertyScaling* templ )
{
    m_elasticPropertyScalings.push_back( templ );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScalingCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                          std::vector<caf::PdmObjectHandle*>& referringObjects )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimElasticPropertyScalingCollection::getScaling( const QString&            formationName,
                                                        const QString&            faciesName,
                                                        RiaDefines::CurveProperty property ) const
{
    for ( const RimElasticPropertyScaling* scaling : m_elasticPropertyScalings )
    {
        if ( scaling->property() == property && scaling->formation() == formationName && scaling->facies() == faciesName )
        {
            return scaling->scale();
        }
    }

    // No scaling found. Default is not scaling (1.0).
    return 1.0;
}
