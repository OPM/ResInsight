/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-  Equinor ASA
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

#include "RimWellTargetMapping.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaOptionItemFactory.h"
#include "RiaPorosityModel.h"
#include "RiaResultNames.h"

#include "RigActiveCellInfo.h"
#include "RiuMainWindow.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFloodingSettings.h"
#include "RigStatisticsMath.h"
#include "Well/RigWellTargetMapping.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimRegularGridCase.h"
#include "RimTools.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiSliderTools.h"

#include "cvfMath.h"

#include <cmath>
#include <limits>

CAF_PDM_SOURCE_INIT( RimWellTargetMapping, "RimWellTargetMapping" );

namespace caf
{
template <>
void caf::AppEnum<RigWellTargetMapping::VolumeType>::setUp()
{
    addItem( RigWellTargetMapping::VolumeType::OIL, "OIL", "Oil" );
    addItem( RigWellTargetMapping::VolumeType::GAS, "GAS", "Gas" );
    addItem( RigWellTargetMapping::VolumeType::HYDROCARBON, "HYDROCARBON", "Hydrocarbon" );
    setDefault( RigWellTargetMapping::VolumeType::OIL );
}

template <>
void caf::AppEnum<RigWellTargetMapping::VolumeResultType>::setUp()
{
    addItem( RigWellTargetMapping::VolumeResultType::MOBILE, "MOBILE", "Mobile" );
    addItem( RigWellTargetMapping::VolumeResultType::TOTAL, "TOTAL", "Total" );
    setDefault( RigWellTargetMapping::VolumeResultType::TOTAL );
}

template <>
void caf::AppEnum<RigWellTargetMapping::VolumesType>::setUp()
{
    addItem( RigWellTargetMapping::VolumesType::RESERVOIR_VOLUMES, "RESERVOIR_VOLUMES", "Reservoir Volumes (RFIPOIL, RFIPGAS)" );
    addItem( RigWellTargetMapping::VolumesType::SURFACE_VOLUMES_SFIP, "SURFACE_VOLUMES_SFIP", "Surface Volumes (SFIPOIL, SFIPGAS)" );
    addItem( RigWellTargetMapping::VolumesType::SURFACE_VOLUMES_FIP, "SURFACE_VOLUMES_FIP", "Surface Volumes (FIPOIL, FIPGAS)" );
    addItem( RigWellTargetMapping::VolumesType::RESERVOIR_VOLUMES_COMPUTED,
             "RESERVOIR_VOLUMES_COMPUTED",
             "Reservoir Volumes (PORV*SOIL, PORV*SGAS)" );
    setDefault( RigWellTargetMapping::VolumesType::RESERVOIR_VOLUMES_COMPUTED );
}

} // End namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellTargetMapping::RimWellTargetMapping()
{
    CAF_PDM_InitObject( "Well Target Mapping", ":/WellTargets.png" );

    CAF_PDM_InitField( &m_timeStep, "TimeStep", 0, "Time Step" );

    CAF_PDM_InitFieldNoDefault( &m_volumeType, "VolumeType", "Volume" );
    CAF_PDM_InitFieldNoDefault( &m_volumeResultType, "VolumeResultType", "Result" );
    CAF_PDM_InitFieldNoDefault( &m_volumesType, "VolumesType", "" );

    CAF_PDM_InitFieldNoDefault( &m_oilFloodingType, "OilFloodingType", "Residual Oil Given By" );
    m_oilFloodingType.setValue( RigFloodingSettings::FloodingType::WATER_FLOODING );
    CAF_PDM_InitField( &m_userDefinedFloodingOil, "UserDefinedFloodingOil", 0.0, "" );
    m_userDefinedFloodingOil.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_gasFloodingType, "GasFloodingType", RigFloodingSettings::FloodingType::GAS_FLOODING, "Residual Oil-in-Gas Given By" );
    caf::AppEnum<RigFloodingSettings::FloodingType>::setEnumSubset( &m_gasFloodingType,
                                                                    { RigFloodingSettings::FloodingType::GAS_FLOODING,
                                                                      RigFloodingSettings::FloodingType::USER_DEFINED } );

    CAF_PDM_InitField( &m_userDefinedFloodingGas, "UserDefinedFloodingGas", 0.0, "" );
    m_userDefinedFloodingGas.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_saturationOil, "SaturationOil", 0.0, "Saturation Oil" );
    m_saturationOil.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_saturationGas, "SaturationGas", 0.0, "Saturation Gas" );
    m_saturationGas.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_pressure, "Pressure", 0.0, "Pressure" );
    m_pressure.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_permeability, "Permeability", 0.0, "Permeability [K<sub>h</sub>]" );
    m_permeability.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_transmissibility, "Transmissibility", 0.0, "Transmissibility" );
    m_transmissibility.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_resetDefaultButton, "ResetDefaultButton", true, "Reset to Default" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_resetDefaultButton );
    m_resetDefaultButton.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_maxIterations, "Iterations", 100000, "Max Iterations" );
    CAF_PDM_InitField( &m_maxNumTargets, "MaxNumTargets", 5, "Maximum Number of Well Targets" );

    CAF_PDM_InitFieldNoDefault( &m_resultDefinition, "ResultDefinition", "" );
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_resultDefinition = new RimEclipseResultDefinition;
    m_resultDefinition->findField( "MResultType" )->uiCapability()->setUiName( "Result" );
    m_resultDefinition->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
    m_resultDefinition->setResultVariable( "SOIL" );

    CAF_PDM_InitField( &m_cellCountI, "CellCountI", 100, "Cell Count I" );
    CAF_PDM_InitField( &m_cellCountJ, "CellCountJ", 100, "Cell Count J" );
    CAF_PDM_InitField( &m_cellCountK, "CellCountK", 10, "Cell Count K" );

    CAF_PDM_InitField( &m_generateButton, "GenerateButton", true, "Generate" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_generateButton );
    m_generateButton.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_filterView, "FilterView", "Filter By View" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleStatisticsCase, "EnsembleStatisticsCase", "Ensemble Statistics Case" );

    m_minimumSaturationOil = cvf::UNDEFINED_DOUBLE;
    m_maximumSaturationOil = cvf::UNDEFINED_DOUBLE;
    m_defaultSaturationOil = cvf::UNDEFINED_DOUBLE;

    m_minimumSaturationGas = cvf::UNDEFINED_DOUBLE;
    m_maximumSaturationGas = cvf::UNDEFINED_DOUBLE;
    m_defaultSaturationGas = cvf::UNDEFINED_DOUBLE;

    m_minimumPressure = cvf::UNDEFINED_DOUBLE;
    m_maximumPressure = cvf::UNDEFINED_DOUBLE;
    m_defaultPressure = cvf::UNDEFINED_DOUBLE;

    m_minimumPermeability = cvf::UNDEFINED_DOUBLE;
    m_maximumPermeability = cvf::UNDEFINED_DOUBLE;
    m_defaultPermeability = cvf::UNDEFINED_DOUBLE;

    m_minimumTransmissibility = cvf::UNDEFINED_DOUBLE;
    m_maximumTransmissibility = cvf::UNDEFINED_DOUBLE;
    m_defaultTransmissibility = cvf::UNDEFINED_DOUBLE;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellTargetMapping::~RimWellTargetMapping()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetMapping::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    updateAllBoundaries();

    if ( changedField == &m_generateButton )
    {
        auto hasEnsembleParent = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>() != nullptr;
        if ( hasEnsembleParent )
            generateEnsembleStatistics();
        else if ( auto eclipseCase = firstCase() )
        {
            generateCandidates( eclipseCase );
            if ( auto views = eclipseCase->reservoirViews(); !views.empty() )
            {
                auto eclipseView = views.front();
                eclipseView->cellResult()->setResultType( RiaDefines::ResultCatType::GENERATED );
                eclipseView->cellResult()->setResultVariable( RigWellTargetMapping::wellTargetResultName() );

                if ( eclipseView->eclipsePropertyFilterCollection()->propertyFilters().empty() )
                {
                    eclipseView->eclipsePropertyFilterCollection()->addFilterLinkedToCellResult();
                    eclipseView->eclipsePropertyFilterCollection()->updateConnectedEditors();
                }

                if ( RiaGuiApplication::isRunning() || RiuMainWindow::instance() )
                {
                    RiuMainWindow::instance()->selectAsCurrentItem( eclipseView->cellResult() );
                }
            }
        }
    }
    else if ( changedField == &m_resetDefaultButton )
    {
        resetMinimumCellValuesToDefault();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellTargetMapping::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_timeStep )
    {
        if ( auto fc = firstCase() )
        {
            RimTools::timeStepsForCase( fc, &options );
        }
    }
    else if ( fieldNeedingOptions == &m_volumesType )
    {
        if ( auto fc = firstCase() )
        {
            caf::AppEnum<RigWellTargetMapping::VolumesType>::setEnumSubset( &m_volumesType, findAvailableVolumesTypes( fc ) );
        }
    }
    else if ( fieldNeedingOptions == &m_filterView )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        if ( auto fc = firstCase() )
        {
            for ( const auto& view : fc->views() )
            {
                RiaOptionItemFactory::appendOptionItemFromViewNameAndCaseName( view, &options );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetMapping::updateAllBoundaries()
{
    RimEclipseCase* eclipseCase = firstCase();
    if ( !eclipseCase ) return;

    eclipseCase->ensureReservoirCaseIsOpen();

    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return;

    const int timeStepIdx = m_timeStep();

    auto updateBoundaryValues =
        []( auto resultsData, const std::vector<RigEclipseResultAddress>& addresses, size_t timeStepIdx ) -> std::tuple<double, double, double>
    {
        double              globalMin = std::numeric_limits<double>::max();
        double              globalMax = -std::numeric_limits<double>::max();
        std::vector<double> allValues;
        for ( auto address : addresses )
        {
            double currentMinimum;
            double currentMaximum;
            if ( resultsData->ensureKnownResultLoaded( address ) )
            {
                resultsData->minMaxCellScalarValues( address, timeStepIdx, currentMinimum, currentMaximum );

                globalMin                         = std::min( globalMin, currentMinimum );
                globalMax                         = std::max( globalMax, currentMaximum );
                const std::vector<double>& values = resultsData->cellScalarResults( address, timeStepIdx );
                allValues.insert( allValues.end(), values.begin(), values.end() );
            }
        }

        double p10, p50, p90, mean;
        RigStatisticsMath::calculateStatisticsCurves( allValues, &p10, &p50, &p90, &mean, RigStatisticsMath::PercentileStyle::SWITCHED );

        return { globalMin, globalMax, p90 };
    };

    std::tie( m_minimumSaturationOil, m_maximumSaturationOil, m_defaultSaturationOil ) =
        updateBoundaryValues( resultsData, { RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "SOIL" ) }, timeStepIdx );
    m_defaultSaturationOil = 0.3;

    std::tie( m_minimumSaturationGas, m_maximumSaturationGas, m_defaultSaturationGas ) =
        updateBoundaryValues( resultsData, { RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "SGAS" ) }, timeStepIdx );
    m_defaultSaturationGas = 0.3;

    std::tie( m_minimumPressure, m_maximumPressure, m_defaultPressure ) =
        updateBoundaryValues( resultsData, { RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" ) }, timeStepIdx );

    std::tie( m_minimumPermeability, m_maximumPermeability, m_defaultPermeability ) =
        updateBoundaryValues( resultsData, { RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMX" ) }, 0 );

    std::tie( m_minimumTransmissibility, m_maximumTransmissibility, m_defaultTransmissibility ) =
        updateBoundaryValues( resultsData,
                              { RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANX" ),
                                RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANY" ),
                                RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANZ" ) },
                              0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetMapping::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )

{
    if ( field == &m_saturationOil && m_minimumSaturationOil != cvf::UNDEFINED_DOUBLE && m_maximumSaturationOil != cvf::UNDEFINED_DOUBLE )
    {
        if ( auto doubleAttributes = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            doubleAttributes->m_minimum  = m_minimumSaturationOil;
            doubleAttributes->m_maximum  = m_maximumSaturationOil;
            doubleAttributes->m_decimals = 3;
        }
    }

    if ( field == &m_saturationGas && m_minimumSaturationGas != cvf::UNDEFINED_DOUBLE && m_maximumSaturationGas != cvf::UNDEFINED_DOUBLE )
    {
        if ( auto doubleAttributes = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            doubleAttributes->m_minimum  = m_minimumSaturationGas;
            doubleAttributes->m_maximum  = m_maximumSaturationGas;
            doubleAttributes->m_decimals = 3;
        }
    }

    if ( field == &m_pressure && m_minimumPressure != cvf::UNDEFINED_DOUBLE && m_maximumPressure != cvf::UNDEFINED_DOUBLE )
    {
        if ( auto doubleAttributes = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            doubleAttributes->m_minimum  = m_minimumPressure;
            doubleAttributes->m_maximum  = m_maximumPressure;
            doubleAttributes->m_decimals = 3;
        }
    }

    if ( field == &m_permeability && m_minimumPermeability != cvf::UNDEFINED_DOUBLE && m_maximumPermeability != cvf::UNDEFINED_DOUBLE )
    {
        if ( auto doubleAttributes = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            doubleAttributes->m_minimum  = m_minimumPermeability;
            doubleAttributes->m_maximum  = m_maximumPermeability;
            doubleAttributes->m_decimals = 3;
        }
    }

    if ( field == &m_transmissibility && m_minimumTransmissibility != cvf::UNDEFINED_DOUBLE && m_maximumTransmissibility != cvf::UNDEFINED_DOUBLE )
    {
        if ( auto doubleAttributes = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            doubleAttributes->m_minimum  = m_minimumTransmissibility;
            doubleAttributes->m_maximum  = m_maximumTransmissibility;
            doubleAttributes->m_decimals = 3;
        }
    }

    if ( field == &m_generateButton )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Generate";
        }
    }

    if ( field == &m_resetDefaultButton )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Reset to Default";
        }
    }

    if ( ( &m_userDefinedFloodingOil == field ) || ( &m_userDefinedFloodingGas == field ) )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum         = 0.0;
            myAttr->m_maximum         = 1.0;
            myAttr->m_sliderTickCount = 20;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RimWellTargetMapping::getResultGridCellCount() const
{
    return cvf::Vec3st( m_cellCountI, m_cellCountJ, m_cellCountK );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetMapping::generateCandidates( RimEclipseCase* eclipseCase )
{
    RigWellTargetMapping::ClusteringLimits limits = getClusteringLimits();
    RigFloodingSettings floodingSettings( m_oilFloodingType(), m_userDefinedFloodingOil(), m_gasFloodingType(), m_userDefinedFloodingGas() );

    RigWellTargetMapping::generateCandidates( eclipseCase,
                                              m_timeStep(),
                                              m_volumeType(),
                                              m_volumesType(),
                                              m_volumeResultType(),
                                              floodingSettings,
                                              limits,
                                              true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetMapping::generateEnsembleStatistics()
{
    auto ensemble = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
    if ( !ensemble ) return;

    const cvf::Vec3st&                     resultGridCellCount = getResultGridCellCount();
    RigWellTargetMapping::ClusteringLimits limits              = getClusteringLimits();
    RigFloodingSettings floodingSettings( m_oilFloodingType(), m_userDefinedFloodingOil(), m_gasFloodingType(), m_userDefinedFloodingGas() );

    RimRegularGridCase* regularGridCase = RigWellTargetMapping::generateEnsembleCandidates( *ensemble,
                                                                                            m_timeStep(),
                                                                                            resultGridCellCount,
                                                                                            m_volumeType(),
                                                                                            m_volumesType(),
                                                                                            m_volumeResultType(),
                                                                                            floodingSettings,
                                                                                            limits );

    regularGridCase->setCustomCaseName( "Ensemble Grid" );

    m_ensembleStatisticsCase = regularGridCase;

    auto eclipseView = regularGridCase->createAndAddReservoirView();

    eclipseView->cellResult()->setResultType( RiaDefines::ResultCatType::GENERATED );
    eclipseView->cellResult()->setResultVariable( "TOTAL_PORV_SOIL_P10" );

    if ( RiaGuiApplication::isRunning() || RiuMainWindow::instance() )
    {
        RiuMainWindow::instance()->selectAsCurrentItem( eclipseView->cellResult() );
    }

    eclipseView->loadDataAndUpdate();

    m_ensembleStatisticsCase->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetMapping::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* resultGroup = uiOrdering.addNewGroup( "Result" );
    resultGroup->add( &m_timeStep );
    resultGroup->add( &m_volumeType );
    resultGroup->add( &m_volumeResultType );

    bool showGasOptions = m_volumeType() == RigWellTargetMapping::VolumeType::GAS ||
                          m_volumeType() == RigWellTargetMapping::VolumeType::HYDROCARBON;
    bool showOilOptions = m_volumeType() == RigWellTargetMapping::VolumeType::OIL ||
                          m_volumeType() == RigWellTargetMapping::VolumeType::HYDROCARBON;

    if ( m_volumeResultType() == RigWellTargetMapping::VolumeResultType::MOBILE )
    {
        if ( showOilOptions )
        {
            resultGroup->add( &m_oilFloodingType );
            if ( m_oilFloodingType() == RigFloodingSettings::FloodingType::USER_DEFINED )
            {
                resultGroup->add( &m_userDefinedFloodingOil );
            }
        }
        if ( showGasOptions )
        {
            resultGroup->add( &m_gasFloodingType );
            if ( m_gasFloodingType() == RigFloodingSettings::FloodingType::USER_DEFINED )
            {
                resultGroup->add( &m_userDefinedFloodingGas );
            }
        }
    }

    resultGroup->add( &m_volumesType );

    auto hasEnsembleParent = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>() != nullptr;
    if ( !hasEnsembleParent ) uiOrdering.add( &m_filterView );

    caf::PdmUiGroup* minimumCellValuesGroup = uiOrdering.addNewGroup( "Minimum Cell Values" );
    if ( showOilOptions ) minimumCellValuesGroup->add( &m_saturationOil );
    if ( showGasOptions ) minimumCellValuesGroup->add( &m_saturationGas );
    minimumCellValuesGroup->add( &m_pressure );
    minimumCellValuesGroup->add( &m_permeability );
    minimumCellValuesGroup->add( &m_transmissibility );
    minimumCellValuesGroup->add( &m_resetDefaultButton );

    if ( hasEnsembleParent )
    {
        caf::PdmUiGroup* ensembleGridGroup = uiOrdering.addNewGroup( "Ensemble Statistics Grid" );
        ensembleGridGroup->add( &m_cellCountI );
        ensembleGridGroup->add( &m_cellCountJ );
        ensembleGridGroup->add( &m_cellCountK );
    }

    caf::PdmUiGroup* advancedGroup = uiOrdering.addNewGroup( "Advanced" );
    advancedGroup->add( &m_maxNumTargets );
    advancedGroup->setCollapsedByDefault();

    uiOrdering.add( &m_generateButton );

    uiOrdering.skipRemainingFields();

    if ( m_minimumPressure == cvf::UNDEFINED_DOUBLE || m_maximumPressure == cvf::UNDEFINED_DOUBLE )
    {
        updateAllBoundaries();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellTargetMapping::ClusteringLimits RimWellTargetMapping::getClusteringLimits() const
{
    return { .saturationOil    = m_saturationOil,
             .saturationGas    = m_saturationGas,
             .permeability     = m_permeability,
             .pressure         = m_pressure,
             .transmissibility = m_transmissibility,
             .maxNumTargets    = m_maxNumTargets,
             .maxIterations    = m_maxIterations,
             .filterAddress    = m_resultDefinition->eclipseResultAddress(),
             .filter           = getVisibilityFilter() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellTargetMapping::getVisibilityFilter() const
{
    std::vector<double> filter = {};

    // Visibility filter is only valid in the single case setting
    auto hasEnsembleParent = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>() != nullptr;
    if ( !hasEnsembleParent )
    {
        auto fc = firstCase();
        if ( m_filterView() && fc )
        {
            cvf::ref<cvf::UByteArray> visibility = m_filterView->currentTotalCellVisibility();

            auto activeReservoirCellIndices =
                fc->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->activeReservoirCellIndices();
            int numActiveCells = static_cast<int>( activeReservoirCellIndices.size() );

            filter.resize( numActiveCells, std::numeric_limits<double>::infinity() );

            // Create binary filter for active cells: 1.0 can be used, 0.0 is filtered out.
#pragma omp parallel for
            for ( int i = 0; i < numActiveCells; i++ )
            {
                const auto reservoirCellIndex = activeReservoirCellIndices[i];
                filter[i]                     = visibility->val( reservoirCellIndex.value() ) ? 1.0 : 0.0;
            }
        }
    }

    return filter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimWellTargetMapping::firstCase() const
{
    auto ensemble = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
    if ( ensemble && !ensemble->cases().empty() )
        return ensemble->cases()[0];
    else
        return firstAncestorOrThisOfType<RimEclipseCase>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetMapping::initAfterRead()
{
    if ( RimEclipseCase* eclipseCase = firstCase() )
    {
        m_resultDefinition->setEclipseCase( eclipseCase );

        // Automatically generate results on project load
        // Consider to also do this for ensemble cases, but this will be more expensive
        generateCandidates( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetMapping::updateResultDefinition()
{
    RimEclipseCase* eclipseCase = firstCase();
    if ( eclipseCase ) m_resultDefinition->setEclipseCase( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimWellTargetMapping::ensembleStatisticsCase() const
{
    return m_ensembleStatisticsCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetMapping::setDefaults()
{
    auto findFirstByPriority = []( const std::vector<RigWellTargetMapping::VolumesType>& priority,
                                   const std::vector<RigWellTargetMapping::VolumesType>& available )
    {
        for ( auto pri : priority )
        {
            if ( std::find( available.begin(), available.end(), pri ) != available.end() ) return pri;
        }

        return RigWellTargetMapping::VolumesType::RESERVOIR_VOLUMES_COMPUTED;
    };

    // Use last available time step
    if ( RimEclipseCase* eclipseCase = firstCase() )
    {
        m_timeStep = static_cast<int>( eclipseCase->timeStepDates().size() ) - 1;

        std::vector<RigWellTargetMapping::VolumesType> volumesTypesByPriority = { RigWellTargetMapping::VolumesType::RESERVOIR_VOLUMES,
                                                                                  RigWellTargetMapping::VolumesType::SURFACE_VOLUMES_SFIP,
                                                                                  RigWellTargetMapping::VolumesType::SURFACE_VOLUMES_FIP,
                                                                                  RigWellTargetMapping::VolumesType::RESERVOIR_VOLUMES_COMPUTED };
        std::vector<RigWellTargetMapping::VolumesType> availableVolumesTypes  = findAvailableVolumesTypes( eclipseCase );
        m_volumesType = findFirstByPriority( volumesTypesByPriority, availableVolumesTypes );

        updateAllBoundaries();
        resetMinimumCellValuesToDefault();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetMapping::resetMinimumCellValuesToDefault()
{
    m_saturationOil    = std::clamp( m_defaultSaturationOil, m_minimumSaturationOil, m_maximumSaturationOil );
    m_saturationGas    = std::clamp( m_defaultSaturationGas, m_minimumSaturationGas, m_maximumSaturationGas );
    m_pressure         = std::clamp( m_defaultPressure, m_minimumPressure, m_maximumPressure );
    m_permeability     = std::clamp( m_defaultPermeability, m_minimumPermeability, m_maximumPermeability );
    m_transmissibility = std::clamp( m_defaultTransmissibility, std::max( m_minimumTransmissibility, 0.1 ), m_maximumTransmissibility );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWellTargetMapping::VolumesType> RimWellTargetMapping::findAvailableVolumesTypes( RimEclipseCase* eclipseCase )
{
    auto hasResult = []( RigCaseCellResultsData& resultsData, const QString& resultName ) -> bool
    {
        RigEclipseResultAddress address( RiaDefines::ResultCatType::DYNAMIC_NATIVE, resultName );
        return resultsData.ensureKnownResultLoaded( address );
    };

    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return {};

    std::vector<RigWellTargetMapping::VolumesType> availableVolumesTypes;
    if ( hasResult( *resultsData, RiaResultNames::riPorvSoil() ) || hasResult( *resultsData, RiaResultNames::riPorvSgas() ) )
        availableVolumesTypes.push_back( RigWellTargetMapping::VolumesType::RESERVOIR_VOLUMES_COMPUTED );

    if ( hasResult( *resultsData, "RFIPOIL" ) || hasResult( *resultsData, "RFIPGAS" ) )
        availableVolumesTypes.push_back( RigWellTargetMapping::VolumesType::RESERVOIR_VOLUMES );

    if ( hasResult( *resultsData, "SFIPOIL" ) || hasResult( *resultsData, "SFIPGAS" ) )
        availableVolumesTypes.push_back( RigWellTargetMapping::VolumesType::SURFACE_VOLUMES_SFIP );

    if ( hasResult( *resultsData, "FIPOIL" ) || hasResult( *resultsData, "FIPGAS" ) )
        availableVolumesTypes.push_back( RigWellTargetMapping::VolumesType::SURFACE_VOLUMES_FIP );

    return availableVolumesTypes;
}
