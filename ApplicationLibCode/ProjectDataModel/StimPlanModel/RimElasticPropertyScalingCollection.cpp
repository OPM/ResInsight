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
    : changed( this )
{
    CAF_PDM_InitScriptableObject( "Elastic Property Scalings" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_items, "ElasticPropertyScalings", "Elastic Property Scalings" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScalingCollection::elasticPropertyScalingChanged( const caf::SignalEmitter* emitter )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScalingCollection::onItemsChanged()
{
    connectSignals();
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimElasticPropertyScalingCollection::getScaling( const QString&            formationName,
                                                        const QString&            faciesName,
                                                        RiaDefines::CurveProperty property ) const
{
    for ( const RimElasticPropertyScaling* scaling : items() )
    {
        if ( scaling->property() == property && ( scaling->formation().compare( formationName, Qt::CaseInsensitive ) == 0 ) &&
             ( scaling->facies().compare( faciesName, Qt::CaseInsensitive ) == 0 ) && scaling->isChecked() )
        {
            return scaling->scale();
        }
    }

    // No scaling found. Default is not scaling (1.0).
    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScalingCollection::initAfterRead()
{
    connectSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScalingCollection::connectSignals()
{
    for ( auto& scaling : items() )
    {
        scaling->changed.connect( this, &RimElasticPropertyScalingCollection::elasticPropertyScalingChanged );
    }
}
