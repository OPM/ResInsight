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

#include "RimElasticPropertyScaling.h"

#include "RigEclipseCaseData.h"
#include "RimColorLegend.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCase.h"
#include "RimFaciesProperties.h"
#include "RimFractureModelTemplate.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"

CAF_PDM_SOURCE_INIT( RimElasticPropertyScaling, "RimElasticPropertyScaling" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticPropertyScaling::RimElasticPropertyScaling()
    : changed( this )
{
    CAF_PDM_InitScriptableObject( "RimElasticPropertyScaling", "", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_formation, "Formation", "Formation", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_facies, "Facies", "Facies", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_property, "Property", "Property", "", "", "" );
    CAF_PDM_InitScriptableField( &m_scale, "Scale", 1.0, "Scale", "", "", "" );

    setUiName( "Property Scaling" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticPropertyScaling::~RimElasticPropertyScaling()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimElasticPropertyScaling::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_formation )
    {
        RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
        if ( !eclipseCaseData ) return options;

        std::vector<QString> formationNames = eclipseCaseData->formationNames();
        for ( const QString& formationName : formationNames )
        {
            options.push_back( caf::PdmOptionItemInfo( formationName, formationName ) );
        }
    }
    else if ( fieldNeedingOptions == &m_facies )
    {
        RimFractureModelTemplate* fractureModelTemplate;
        firstAncestorOrThisOfType( fractureModelTemplate );
        if ( !fractureModelTemplate ) return options;

        RimFaciesProperties* faciesProperties = fractureModelTemplate->faciesProperties();
        if ( !faciesProperties ) return options;

        RimColorLegend* faciesColors = faciesProperties->colorLegend();
        if ( !faciesColors ) return options;

        for ( RimColorLegendItem* item : faciesColors->colorLegendItems() )
        {
            options.push_back( caf::PdmOptionItemInfo( item->categoryName(), item->categoryName() ) );
        }
    }
    else if ( fieldNeedingOptions == &m_property )
    {
        std::vector<RiaDefines::CurveProperty> properties = {RiaDefines::CurveProperty::YOUNGS_MODULUS,
                                                             RiaDefines::CurveProperty::POISSONS_RATIO};
        for ( auto property : properties )
        {
            options.push_back(
                caf::PdmOptionItemInfo( caf::AppEnum<RiaDefines::CurveProperty>::uiText( property ), property ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScaling::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                  const QVariant&            oldValue,
                                                  const QVariant&            newValue )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimElasticPropertyScaling::getEclipseCase()
{
    // Find an eclipse case
    RimProject* proj = RimProject::current();
    if ( proj->eclipseCases().empty() ) return nullptr;

    return proj->eclipseCases()[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData* RimElasticPropertyScaling::getEclipseCaseData()
{
    // Find an eclipse case
    RimEclipseCase* eclipseCase = getEclipseCase();
    if ( !eclipseCase ) return nullptr;

    return eclipseCase->eclipseCaseData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimElasticPropertyScaling::formation() const
{
    return m_formation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimElasticPropertyScaling::facies() const
{
    return m_facies();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::CurveProperty RimElasticPropertyScaling::property() const
{
    return m_property();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimElasticPropertyScaling::scale() const
{
    return m_scale;
}
