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
#include "RimElasticProperties.h"
#include "RimFaciesProperties.h"
#include "RimFractureModelTemplate.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"

CAF_PDM_SOURCE_INIT( RimElasticPropertyScaling, "ElasticPropertyScaling" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticPropertyScaling::RimElasticPropertyScaling()
    : changed( this )
{
    CAF_PDM_InitScriptableObject( "ElasticPropertyScaling", "", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_formation, "Formation", "Formation", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_facies, "Facies", "Facies", "", "", "" );
    caf::AppEnum<RiaDefines::CurveProperty> defaultProperty = RiaDefines::CurveProperty::YOUNGS_MODULUS;
    CAF_PDM_InitScriptableField( &m_property, "Property", defaultProperty, "Property", "", "", "" );
    CAF_PDM_InitScriptableField( &m_scale, "Scale", 1.0, "Scale", "", "", "" );

    nameField()->uiCapability()->setUiReadOnly( true );

    setUiName( "Property Scaling" );
    setDeletable( true );
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
        std::vector<QString> formationNames = getFormationNames();
        for ( const QString& formationName : formationNames )
        {
            options.push_back( caf::PdmOptionItemInfo( formationName, formationName ) );
        }
    }
    else if ( fieldNeedingOptions == &m_facies )
    {
        RimColorLegend* faciesColors = getFaciesColorLegend();
        if ( !faciesColors ) return options;

        for ( RimColorLegendItem* item : faciesColors->colorLegendItems() )
        {
            options.push_back( caf::PdmOptionItemInfo( item->categoryName(), item->categoryName() ) );
        }
    }
    else if ( fieldNeedingOptions == &m_property )
    {
        std::vector<RiaDefines::CurveProperty> properties = RimElasticProperties::scalableProperties();
        for ( auto property : properties )
        {
            options.push_back(
                caf::PdmOptionItemInfo( caf::AppEnum<RiaDefines::CurveProperty>::uiText( property ), property ) );
        }
    }

    if ( useOptionsOnly ) *useOptionsOnly = true;

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScaling::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                  const QVariant&            oldValue,
                                                  const QVariant&            newValue )
{
    updateAutoName();
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
void RimElasticPropertyScaling::updateAutoName()
{
    QString name = QString( "%1/%2 - %3: %4" )
                       .arg( m_formation )
                       .arg( m_facies )
                       .arg( caf::AppEnum<RiaDefines::CurveProperty>::uiText( m_property() ) )
                       .arg( m_scale );
    setName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend* RimElasticPropertyScaling::getFaciesColorLegend()
{
    RimFractureModelTemplate* fractureModelTemplate;
    firstAncestorOrThisOfType( fractureModelTemplate );
    if ( !fractureModelTemplate ) return nullptr;

    RimFaciesProperties* faciesProperties = fractureModelTemplate->faciesProperties();
    if ( !faciesProperties ) return nullptr;

    return faciesProperties->colorLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimElasticPropertyScaling::getFormationNames()
{
    RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
    if ( !eclipseCaseData ) return std::vector<QString>();

    return eclipseCaseData->formationNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScaling::setFormation( const QString& formation )
{
    m_formation = formation;
    updateAutoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScaling::setFacies( const QString& facies )
{
    m_facies = facies;
    updateAutoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScaling::setScale( double scale )
{
    m_scale = scale;
    updateAutoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScaling::setProperty( RiaDefines::CurveProperty property )
{
    m_property = property;
    updateAutoName();
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertyScaling::ensureDefaultFormationAndFacies()
{
    RimColorLegend* faciesColorLegend = getFaciesColorLegend();
    if ( faciesColorLegend && !faciesColorLegend->colorLegendItems().empty() )
    {
        m_facies = faciesColorLegend->colorLegendItems().front()->categoryName();
    }

    std::vector<QString> formationNames = getFormationNames();
    if ( !formationNames.empty() )
    {
        m_formation = formationNames.front();
    }

    updateAutoName();
}
