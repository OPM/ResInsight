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

#include "RimStimPlanModelTemplate.h"

#include "RiaFractureDefines.h"
#include "RiaStimPlanModelDefines.h"

#include "RigEclipseCaseData.h"

#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimElasticProperties.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFaciesInitialPressureConfig.h"
#include "RimFaciesProperties.h"
#include "RimNonNetLayers.h"
#include "RimPressureTable.h"
#include "RimProject.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelPlot.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"

#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include "cvfBoundingBox.h"

#include <cmath>

#include <QString>

CAF_PDM_SOURCE_INIT( RimStimPlanModelTemplate, "StimPlanModelTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelTemplate::RimStimPlanModelTemplate()
    : changed( this )
{
    CAF_PDM_InitScriptableObject( "StimPlanModelTemplate" );

    CAF_PDM_InitScriptableField( &m_id, "Id", -1, "ID" );
    m_id.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_dynamicEclipseCase, "DynamicEclipseCase", "Dynamic Case" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_initialPressureEclipseCase, "InitialPressureEclipseCase", "Initial Pressure Case" );
    CAF_PDM_InitScriptableFieldWithScriptKeyword( &m_useTableForInitialPressure,
                                                  "UseForInitialPressure",
                                                  "UsePressureTableForInitialPressure",
                                                  false,
                                                  "Use Pressure Table For Initial Pressure" );

    CAF_PDM_InitScriptableFieldWithScriptKeyword( &m_useTableForPressure,
                                                  "UseForPressure",
                                                  "UsePressureTableForPressure",
                                                  false,
                                                  "Use Pressure Table For Pressure" );
    CAF_PDM_InitField( &m_editPressureTable, "EditPressureTable", false, "Edit" );
    m_editPressureTable.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );
    m_editPressureTable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitScriptableFieldNoDefault( &m_staticEclipseCase, "StaticEclipseCase", "Static Case" );

    CAF_PDM_InitScriptableField( &m_useEqlnumForPressureInterpolation,
                                 "UseEqlNumForPressureInterpolation",
                                 true,
                                 "Use EQLNUM For Pressure Interpolation" );

    CAF_PDM_InitScriptableField( &m_defaultPorosity, "DefaultPorosity", RiaDefines::defaultPorosity(), "Default Porosity" );
    CAF_PDM_InitScriptableField( &m_defaultPermeability, "DefaultPermeability", RiaDefines::defaultPermeability(), "Default Permeability" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_defaultFacies, "DefaultFacies", "Default Facies" );

    // Stress unit: bar
    // Stress gradient unit: bar/m
    // Depth is meter
    double defaultStressGradient = 0.238;
    double defaultStressDepth    = computeDefaultStressDepth();
    double defaultStress         = defaultStressDepth * defaultStressGradient;

    CAF_PDM_InitScriptableField( &m_verticalStress, "VerticalStress", defaultStress, "Vertical Stress" );
    m_verticalStress.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
    CAF_PDM_InitScriptableField( &m_verticalStressGradient, "VerticalStressGradient", defaultStressGradient, "Vertical Stress Gradient" );
    CAF_PDM_InitScriptableField( &m_stressDepth, "StressDepth", defaultStressDepth, "Stress Depth" );
    m_stressDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_referenceTemperature, "ReferenceTemperature", 70.0, "Temperature [C]" );
    CAF_PDM_InitScriptableField( &m_referenceTemperatureGradient, "ReferenceTemperatureGradient", 0.025, "Temperature Gradient [C/m]" );
    CAF_PDM_InitScriptableField( &m_referenceTemperatureDepth, "ReferenceTemperatureDepth", 2500.0, "Temperature Depth [m]" );

    CAF_PDM_InitScriptableField( &m_overburdenHeight, "OverburdenHeight", 50.0, "Overburden Height" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_overburdenFormation, "OverburdenFormation", "Overburden Formation" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_overburdenFacies, "OverburdenFacies", "Overburden Facies" );
    CAF_PDM_InitScriptableField( &m_overburdenPorosity, "OverburdenPorosity", 0.0, "Overburden Porosity" );
    CAF_PDM_InitScriptableField( &m_overburdenPermeability, "OverburdenPermeability", 10.0e-6, "Overburden Permeability" );
    CAF_PDM_InitScriptableField( &m_overburdenFluidDensity, "OverburdenFluidDensity", 1.03, "Overburden Fluid Density [g/cm^3]" );

    CAF_PDM_InitScriptableField( &m_underburdenHeight, "UnderburdenHeight", 50.0, "Underburden Height" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_underburdenFormation, "UnderburdenFormation", "Underburden Formation" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_underburdenFacies, "UnderburdenFacies", "Underburden Facies" );
    CAF_PDM_InitScriptableField( &m_underburdenPorosity, "UnderburdenPorosity", 0.0, "Underburden Porosity" );
    CAF_PDM_InitScriptableField( &m_underburdenPermeability, "UnderburdenPermeability", 10.0e-6, "Underburden Permeability" );
    CAF_PDM_InitScriptableField( &m_underburdenFluidDensity, "UnderburdenFluidDensity", 1.03, "Underburden Fluid Density [g/cm^3]" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_faciesInitialPressureConfigs, "FaciesInitialPressureConfigs", "Facies Initial Pressure Configs" );
    m_faciesInitialPressureConfigs.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_faciesInitialPressureConfigs.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_faciesInitialPressureConfigs.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_pressureTable, "PressureTable", "Pressure Table" );
    setPressureTable( new RimPressureTable );

    CAF_PDM_InitScriptableFieldNoDefault( &m_elasticProperties, "ElasticProperties", "Elastic Properties" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_faciesProperties, "FaciesProperties", "Facies Properties" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_nonNetLayers, "NonNetLayers", "Non-Net Layers" );
    setNonNetLayers( new RimNonNetLayers );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelTemplate::~RimStimPlanModelTemplate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_dynamicEclipseCase && m_dynamicEclipseCase )
    {
        // Set a valid default time step
        const int timeStepCount = m_dynamicEclipseCase->timeStepStrings().size();
        if ( timeStepCount > 0 )
        {
            m_timeStep = timeStepCount - 1;
        }
    }
    else if ( changedField == &m_editPressureTable )
    {
        m_editPressureTable = false;
        if ( m_pressureTable() )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( m_pressureTable() );
        }
        return;
    }

    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimStimPlanModelTemplate::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_overburdenFormation || fieldNeedingOptions == &m_underburdenFormation )
    {
        RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
        if ( !eclipseCaseData ) return options;

        std::vector<QString> formationNames = eclipseCaseData->formationNames();
        for ( const QString& formationName : formationNames )
        {
            options.push_back( caf::PdmOptionItemInfo( formationName, formationName ) );
        }
    }
    else if ( fieldNeedingOptions == &m_overburdenFacies || fieldNeedingOptions == &m_underburdenFacies ||
              fieldNeedingOptions == &m_defaultFacies )
    {
        if ( !m_faciesProperties ) return options;

        RimColorLegend* faciesColors = m_faciesProperties->colorLegend();
        if ( faciesColors )
        {
            for ( RimColorLegendItem* item : faciesColors->colorLegendItems() )
            {
                options.push_back( caf::PdmOptionItemInfo( item->categoryName(), item->categoryName() ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_dynamicEclipseCase || fieldNeedingOptions == &m_staticEclipseCase ||
              fieldNeedingOptions == &m_initialPressureEclipseCase )
    {
        RimTools::eclipseCaseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        RimTools::timeStepsForCase( m_dynamicEclipseCase(), &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.add( &m_id );

    uiOrdering.add( &m_staticEclipseCase );

    caf::PdmUiOrdering* pressureDataSourceGroup = uiOrdering.addNewGroup( "Pressure Data Source" );
    pressureDataSourceGroup->add( &m_dynamicEclipseCase );
    pressureDataSourceGroup->add( &m_timeStep );
    pressureDataSourceGroup->add( &m_initialPressureEclipseCase );
    pressureDataSourceGroup->add( &m_useTableForInitialPressure, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
    pressureDataSourceGroup->add( &m_editPressureTable, { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );
    pressureDataSourceGroup->add( &m_useTableForPressure );
    pressureDataSourceGroup->add( &m_useEqlnumForPressureInterpolation );
    m_initialPressureEclipseCase.uiCapability()->setUiReadOnly( m_useTableForInitialPressure() );

    caf::PdmUiOrdering* defaultsGroup = uiOrdering.addNewGroup( "Reservoir Defaults" );
    defaultsGroup->add( &m_defaultPorosity );
    defaultsGroup->add( &m_defaultPermeability );
    defaultsGroup->add( &m_defaultFacies );

    caf::PdmUiOrdering* referenceStressGroup = uiOrdering.addNewGroup( "Reference Stress" );
    referenceStressGroup->add( &m_verticalStress );
    referenceStressGroup->add( &m_verticalStressGradient );
    referenceStressGroup->add( &m_stressDepth );

    caf::PdmUiOrdering* temperatureGroup = uiOrdering.addNewGroup( "Temperature" );
    temperatureGroup->add( &m_referenceTemperature );
    temperatureGroup->add( &m_referenceTemperatureGradient );
    temperatureGroup->add( &m_referenceTemperatureDepth );

    caf::PdmUiOrdering* overburdenGroup = uiOrdering.addNewGroup( "Overburden" );
    overburdenGroup->add( &m_overburdenHeight );
    overburdenGroup->add( &m_overburdenFormation );
    overburdenGroup->add( &m_overburdenFacies );
    overburdenGroup->add( &m_overburdenPorosity );
    overburdenGroup->add( &m_overburdenPermeability );
    overburdenGroup->add( &m_overburdenFluidDensity );

    caf::PdmUiOrdering* underburdenGroup = uiOrdering.addNewGroup( "Underburden" );
    underburdenGroup->add( &m_underburdenHeight );
    underburdenGroup->add( &m_underburdenFormation );
    underburdenGroup->add( &m_underburdenFacies );
    underburdenGroup->add( &m_underburdenPorosity );
    underburdenGroup->add( &m_underburdenPermeability );
    underburdenGroup->add( &m_underburdenFluidDensity );

    caf::PdmUiOrdering* faciesInitialPressureGroup = uiOrdering.addNewGroup( "Facies Pressure Settings" );
    faciesInitialPressureGroup->add( &m_faciesInitialPressureConfigs );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( m_elasticProperties ) uiTreeOrdering.add( m_elasticProperties );
    if ( m_faciesProperties ) uiTreeOrdering.add( m_faciesProperties );
    if ( m_pressureTable ) uiTreeOrdering.add( m_pressureTable );
    if ( m_nonNetLayers ) uiTreeOrdering.add( m_nonNetLayers );

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_stressDepth || field == &m_verticalStress )
    {
        caf::PdmUiDoubleValueEditorAttribute::testAndSetFixedWithTwoDecimals( attribute );
    }

    if ( field == &m_faciesInitialPressureConfigs )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy              = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER;
            tvAttribute->alwaysEnforceResizePolicy = true;
            tvAttribute->minimumHeight             = 300;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setId( int id )
{
    m_id = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimStimPlanModelTemplate::id() const
{
    return m_id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticProperties* RimStimPlanModelTemplate::elasticProperties() const
{
    return m_elasticProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setElasticProperties( RimElasticProperties* elasticProperties )
{
    if ( m_elasticProperties )
    {
        m_elasticProperties->changed.disconnect( this );
    }

    m_elasticProperties = elasticProperties;

    if ( m_elasticProperties )
    {
        m_elasticProperties->changed.connect( this, &RimStimPlanModelTemplate::elasticPropertiesChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaciesProperties* RimStimPlanModelTemplate::faciesProperties() const
{
    return m_faciesProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setFaciesProperties( RimFaciesProperties* faciesProperties )
{
    if ( m_faciesProperties )
    {
        m_faciesProperties->changed.disconnect( this );
    }

    m_faciesProperties = faciesProperties;

    if ( m_faciesProperties )
    {
        RimColorLegend* faciesColors = m_faciesProperties->colorLegend();
        if ( faciesColors )
        {
            for ( RimColorLegendItem* item : faciesColors->colorLegendItems() )
            {
                bool exists = std::find_if( m_faciesInitialPressureConfigs.begin(),
                                            m_faciesInitialPressureConfigs.end(),
                                            [item]( const auto& c )
                                            { return c->faciesValue() == item->categoryValue(); } ) != m_faciesInitialPressureConfigs.end();
                if ( !exists )
                {
                    RimFaciesInitialPressureConfig* fipConfig                   = new RimFaciesInitialPressureConfig;
                    bool                            enableInitialPressureConfig = true;
                    fipConfig->setEnabled( enableInitialPressureConfig );
                    fipConfig->setFaciesName( item->categoryName() );
                    fipConfig->setFaciesValue( item->categoryValue() );
                    fipConfig->setFraction( 1.0 );
                    m_faciesInitialPressureConfigs.push_back( fipConfig );

                    fipConfig->changed.connect( this, &RimStimPlanModelTemplate::faciesPropertiesChanged );
                }
            }
        }

        m_faciesProperties->changed.connect( this, &RimStimPlanModelTemplate::faciesPropertiesChanged );
        RimEclipseCase* eclipseCase = getEclipseCase();
        if ( !eclipseCase ) return;

        m_faciesProperties->setEclipseCase( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setNonNetLayers( RimNonNetLayers* nonNetLayers )
{
    if ( m_nonNetLayers )
    {
        m_nonNetLayers->changed.disconnect( this );
    }

    m_nonNetLayers = nonNetLayers;

    if ( m_nonNetLayers )
    {
        m_nonNetLayers->changed.connect( this, &RimStimPlanModelTemplate::nonNetLayersChanged );
        RimEclipseCase* eclipseCase = getEclipseCase();
        if ( !eclipseCase ) return;

        m_nonNetLayers->setEclipseCase( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimNonNetLayers* RimStimPlanModelTemplate::nonNetLayers() const
{
    return m_nonNetLayers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setPressureTable( RimPressureTable* pressureTable )
{
    if ( m_pressureTable )
    {
        m_pressureTable->changed.disconnect( this );
    }

    m_pressureTable = pressureTable;

    if ( m_pressureTable )
    {
        m_pressureTable->changed.connect( this, &RimStimPlanModelTemplate::pressureTableChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPressureTable* RimStimPlanModelTemplate::pressureTable() const
{
    return m_pressureTable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::initAfterRead()
{
    RimEclipseCase* eclipseCase = getEclipseCase();
    if ( !eclipseCase ) return;

    if ( m_faciesProperties )
    {
        m_faciesProperties->setEclipseCase( eclipseCase );
    }

    for ( auto& fipConfig : m_faciesInitialPressureConfigs.childrenByType() )
    {
        fipConfig->changed.connect( this, &RimStimPlanModelTemplate::faciesPropertiesChanged );
    }

    if ( m_nonNetLayers )
    {
        m_nonNetLayers->setEclipseCase( eclipseCase );
    }

    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2023.03" ) )
    {
        m_defaultFacies = m_overburdenFacies;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::faciesPropertiesChanged( const caf::SignalEmitter* emitter )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::elasticPropertiesChanged( const caf::SignalEmitter* emitter )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::nonNetLayersChanged( const caf::SignalEmitter* emitter )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::pressureTableChanged( const caf::SignalEmitter* emitter )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::loadDataAndUpdate()
{
    if ( m_elasticProperties )
    {
        m_elasticProperties->loadDataAndUpdate();
    }

    if ( m_faciesProperties )
    {
        m_faciesProperties->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultPorosity() const
{
    return m_defaultPorosity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultPermeability() const
{
    return m_defaultPermeability();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModelTemplate::defaultFacies() const
{
    return m_defaultFacies;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::overburdenFluidDensity() const
{
    return m_overburdenFluidDensity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::underburdenFluidDensity() const
{
    return m_underburdenFluidDensity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::overburdenHeight() const
{
    return m_overburdenHeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::underburdenHeight() const
{
    return m_underburdenHeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultOverburdenPorosity() const
{
    return m_overburdenPorosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultUnderburdenPorosity() const
{
    return m_underburdenPorosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultOverburdenPermeability() const
{
    return m_overburdenPermeability;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultUnderburdenPermeability() const
{
    return m_underburdenPermeability;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModelTemplate::overburdenFormation() const
{
    return m_overburdenFormation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModelTemplate::overburdenFacies() const
{
    return m_overburdenFacies;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModelTemplate::underburdenFormation() const
{
    return m_underburdenFormation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModelTemplate::underburdenFacies() const
{
    return m_underburdenFacies;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::verticalStress() const
{
    return m_verticalStress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::verticalStressGradient() const
{
    return m_verticalStressGradient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::stressDepth() const
{
    return m_stressDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::referenceTemperature() const
{
    return m_referenceTemperature;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::referenceTemperatureGradient() const
{
    return m_referenceTemperatureGradient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::referenceTemperatureDepth() const
{
    return m_referenceTemperatureDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::computeDefaultStressDepth() const
{
    const double stressDepth = 1000.0;

    RimEclipseCase* eclipseCase = getEclipseCase();
    if ( !eclipseCase ) return stressDepth;

    // Use top of active cells as reference stress depth
    return -eclipseCase->activeCellsBoundingBox().max().z();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStimPlanModelTemplate::getEclipseCase() const
{
    return m_staticEclipseCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData* RimStimPlanModelTemplate::getEclipseCaseData() const
{
    // Find an eclipse case
    RimEclipseCase* eclipseCase = getEclipseCase();
    if ( !eclipseCase ) return nullptr;

    return eclipseCase->eclipseCaseData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setDynamicEclipseCase( RimEclipseCase* eclipseCase )
{
    m_dynamicEclipseCase = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setTimeStep( int timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setStaticEclipseCase( RimEclipseCase* eclipseCase )
{
    m_staticEclipseCase = eclipseCase;

    if ( m_nonNetLayers ) m_nonNetLayers->setEclipseCase( eclipseCase );
    if ( m_faciesProperties ) m_faciesProperties->setEclipseCase( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStimPlanModelTemplate::dynamicEclipseCase() const
{
    return m_dynamicEclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimStimPlanModelTemplate::timeStep() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStimPlanModelTemplate::staticEclipseCase() const
{
    return m_staticEclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setInitialPressureEclipseCase( RimEclipseCase* eclipseCase )
{
    m_initialPressureEclipseCase = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStimPlanModelTemplate::initialPressureEclipseCase() const
{
    return m_initialPressureEclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, double> RimStimPlanModelTemplate::faciesWithInitialPressure() const
{
    std::map<int, double> valueFractionMap;
    for ( const RimFaciesInitialPressureConfig* c : m_faciesInitialPressureConfigs.childrenByType() )
    {
        if ( c->isEnabled() ) valueFractionMap[c->faciesValue()] = c->fraction();
    }

    return valueFractionMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelTemplate::usePressureTableForProperty( RiaDefines::CurveProperty curveProperty ) const
{
    if ( !m_pressureTable ) return false;

    if ( curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
        return m_useTableForInitialPressure();
    else if ( curveProperty == RiaDefines::CurveProperty::PRESSURE )
        return m_useTableForPressure();
    else
        return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelTemplate::useEqlnumForPressureInterpolation() const
{
    return m_useEqlnumForPressureInterpolation;
}
