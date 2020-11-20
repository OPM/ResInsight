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

#include "RimColorLegend.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimFaciesProperties.h"
#include "RimProject.h"
#include "RimStimPlanModelTemplate.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"

#include <QDoubleValidator>

CAF_PDM_SOURCE_INIT( RimNonNetLayers, "NonNetLayers" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimNonNetLayers::RimNonNetLayers()
    : changed( this )
{
    CAF_PDM_InitScriptableObject( "RimNonNetLayers", "", "", "" );

    CAF_PDM_InitScriptableField( &m_cutOff, "Cutoff", 0.0, "Cutoff", "", "", "" );
    m_cutOff.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableFieldNoDefault( &m_facies, "Facies", "Facies", "", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_resultDefinition, "FaciesDefinition", "", "", "", "" );
    m_resultDefinition.uiCapability()->setUiHidden( true );
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_resultDefinition = new RimEclipseResultDefinition;
    m_resultDefinition->findField( "MResultType" )->uiCapability()->setUiName( "Facies Definiton" );
    m_resultDefinition->setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
    m_resultDefinition->setResultVariable( "NTG" );

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
    if ( fieldNeedingOptions == &m_facies )
    {
        RimColorLegend* faciesColors = getFaciesColorLegend();
        if ( !faciesColors ) return options;

        for ( RimColorLegendItem* item : faciesColors->colorLegendItems() )
        {
            options.push_back( caf::PdmOptionItemInfo( item->categoryName(), item->categoryName() ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNonNetLayers::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                             QString                    uiConfigName,
                                             caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_cutOff )
    {
        auto uiDoubleValueEditorAttr = dynamic_cast<caf::PdmUiDoubleValueEditorAttribute*>( attribute );
        if ( uiDoubleValueEditorAttr )
        {
            uiDoubleValueEditorAttr->m_validator = new QDoubleValidator( 0.0, 1.0, 2 );
        }
    }
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
const QString& RimNonNetLayers::facies() const
{
    return m_facies();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend* RimNonNetLayers::getFaciesColorLegend()
{
    RimStimPlanModelTemplate* stimPlanModelTemplate;
    firstAncestorOrThisOfType( stimPlanModelTemplate );
    if ( !stimPlanModelTemplate ) return nullptr;

    RimFaciesProperties* faciesProperties = stimPlanModelTemplate->faciesProperties();
    if ( !faciesProperties ) return nullptr;

    return faciesProperties->colorLegend();
}
