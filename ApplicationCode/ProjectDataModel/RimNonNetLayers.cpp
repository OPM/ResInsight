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

#include "RimNonNetLayers.h"

#include "RigFormationNames.h"
#include "RimColorLegend.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimFaciesProperties.h"
#include "RimFractureModelTemplate.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimTools.h"

#include "RigEclipseCaseData.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"

CAF_PDM_SOURCE_INIT( RimNonNetLayers, "NonNetLayers" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimNonNetLayers::RimNonNetLayers()
    : changed( this )
{
    CAF_PDM_InitScriptableObject( "RimNonNetLayers", "", "", "" );

    CAF_PDM_InitScriptableField( &m_cutOff, "Cutoff", 0.5, "Cutoff", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_formation, "Formation", "Formation", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_facies, "Facies", "Facies", "", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_resultDefinition, "FaciesDefinition", "", "", "", "" );
    m_resultDefinition.uiCapability()->setUiHidden( true );
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_resultDefinition = new RimEclipseResultDefinition;
    m_resultDefinition->findField( "MResultType" )->uiCapability()->setUiName( "Facies Definiton" );

    setUiName( "Non-Net Layers" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimNonNetLayers::~RimNonNetLayers()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimNonNetLayers::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly )
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

    // QList<caf::PdmOptionItemInfo> options;
    // if ( fieldNeedingOptions == &m_colorLegend )
    // {
    //     RimTools::colorLegendOptionItems( &options );
    // }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNonNetLayers::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* resultDefinitionGroup = uiOrdering.addNewGroup( "Facies Definition" );
    m_resultDefinition->uiOrdering( uiConfigName, *resultDefinitionGroup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNonNetLayers::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNonNetLayers::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_resultDefinition->setEclipseCase( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimEclipseResultDefinition* RimNonNetLayers::resultDefinition() const
{
    return m_resultDefinition.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimNonNetLayers::cutOff() const
{
    return m_cutOff;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimNonNetLayers::formation() const
{
    return m_formation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimNonNetLayers::facies() const
{
    return m_facies();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend* RimNonNetLayers::getFaciesColorLegend()
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
std::vector<QString> RimNonNetLayers::getFormationNames()
{
    RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
    if ( !eclipseCaseData ) return std::vector<QString>();

    return eclipseCaseData->formationNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimNonNetLayers::getEclipseCase()
{
    // Find an eclipse case
    RimProject* proj = RimProject::current();
    if ( proj->eclipseCases().empty() ) return nullptr;

    return proj->eclipseCases()[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData* RimNonNetLayers::getEclipseCaseData()
{
    // Find an eclipse case
    RimEclipseCase* eclipseCase = getEclipseCase();
    if ( !eclipseCase ) return nullptr;

    return eclipseCase->eclipseCaseData();
}
