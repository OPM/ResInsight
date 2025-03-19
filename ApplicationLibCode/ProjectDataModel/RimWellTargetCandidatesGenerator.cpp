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

#include "RimWellTargetCandidatesGenerator.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPorosityModel.h"
#include "RiaResultNames.h"

#include "RiuMainWindow.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "Well/RigWellTargetCandidatesGenerator.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseCellColors.h"
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

CAF_PDM_SOURCE_INIT( RimWellTargetCandidatesGenerator, "RimWellTargetCandidatesGenerator" );

namespace caf
{
template <>
void caf::AppEnum<RigWellTargetCandidatesGenerator::VolumeType>::setUp()
{
    addItem( RigWellTargetCandidatesGenerator::VolumeType::OIL, "OIL", "Oil" );
    addItem( RigWellTargetCandidatesGenerator::VolumeType::GAS, "GAS", "Gas" );
    addItem( RigWellTargetCandidatesGenerator::VolumeType::HYDROCARBON, "HYDROCARBON", "Hydrocarbon" );
    setDefault( RigWellTargetCandidatesGenerator::VolumeType::OIL );
}

template <>
void caf::AppEnum<RigWellTargetCandidatesGenerator::VolumeResultType>::setUp()
{
    addItem( RigWellTargetCandidatesGenerator::VolumeResultType::MOBILE, "MOBILE", "Mobile" );
    addItem( RigWellTargetCandidatesGenerator::VolumeResultType::TOTAL, "TOTAL", "Total" );
    setDefault( RigWellTargetCandidatesGenerator::VolumeResultType::TOTAL );
}

template <>
void caf::AppEnum<RigWellTargetCandidatesGenerator::VolumesType>::setUp()
{
    addItem( RigWellTargetCandidatesGenerator::VolumesType::RESERVOIR_VOLUMES, "RESERVOIR", "Reservoir Volumes (RFIPOIL, RFIPGAS)" );
    addItem( RigWellTargetCandidatesGenerator::VolumesType::SURFACE_VOLUMES, "SURFACE", "Surface Volumes (SFIPOIL, SFIPGAS)" );
    addItem( RigWellTargetCandidatesGenerator::VolumesType::COMPUTED_VOLUMES, "COMPUTED", "Computed Volumes (PORV*SOIL, PORV*SGAS)" );
    setDefault( RigWellTargetCandidatesGenerator::VolumesType::COMPUTED_VOLUMES );
}

} // End namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellTargetCandidatesGenerator::RimWellTargetCandidatesGenerator()
{
    CAF_PDM_InitObject( "Well Target Candidates Generator", ":/WellTargets.png" );

    CAF_PDM_InitField( &m_timeStep, "TimeStep", 0, "Time Step" );

    CAF_PDM_InitFieldNoDefault( &m_volumeType, "VolumeType", "Volume" );
    CAF_PDM_InitFieldNoDefault( &m_volumeResultType, "VolumeResultType", "Result" );
    CAF_PDM_InitFieldNoDefault( &m_volumesType, "VolumesType", "" );

    CAF_PDM_InitField( &m_volume, "Volume", 0.0, "Volume" );
    m_volume.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_pressure, "Pressure", 0.0, "Pressure" );
    m_pressure.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_permeability, "Permeability", 0.0, "Permeability" );
    m_permeability.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_transmissibility, "Transmissibility", 0.0, "Transmissibility" );
    m_transmissibility.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_maxIterations, "Iterations", 10000, "Max Iterations" );
    CAF_PDM_InitField( &m_maxClusters, "MaxClusters", 5, "Max Clusters" );

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

    CAF_PDM_InitFieldNoDefault( &m_ensembleStatisticsCase, "EnsembleStatisticsCase", "Ensemble Statistics Case" );

    m_minimumVolume = cvf::UNDEFINED_DOUBLE;
    m_maximumVolume = cvf::UNDEFINED_DOUBLE;

    m_minimumPressure = cvf::UNDEFINED_DOUBLE;
    m_maximumPressure = cvf::UNDEFINED_DOUBLE;

    m_minimumPermeability = cvf::UNDEFINED_DOUBLE;
    m_maximumPermeability = cvf::UNDEFINED_DOUBLE;

    m_minimumTransmissibility = cvf::UNDEFINED_DOUBLE;
    m_maximumTransmissibility = cvf::UNDEFINED_DOUBLE;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellTargetCandidatesGenerator::~RimWellTargetCandidatesGenerator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetCandidatesGenerator::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    updateAllBoundaries();

    if ( changedField == &m_generateButton )
    {
        auto hasEnsembleParent = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>() != nullptr;
        if ( hasEnsembleParent )
            generateEnsembleStatistics();
        else if ( auto eclipseCase = firstCase() )
            generateCandidates( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellTargetCandidatesGenerator::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_timeStep )
    {
        if ( auto fc = firstCase() )
        {
            RimTools::timeStepsForCase( fc, &options );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetCandidatesGenerator::updateAllBoundaries()
{
    RimEclipseCase* eclipseCase = firstCase();
    if ( !eclipseCase ) return;

    eclipseCase->ensureReservoirCaseIsOpen();

    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return;

    const int timeStepIdx = m_timeStep();

    auto updateBoundaryValues =
        []( auto resultsData, const std::vector<RigEclipseResultAddress>& addresses, size_t timeStepIdx ) -> std::pair<double, double>
    {
        double globalMin = std::numeric_limits<double>::max();
        double globalMax = -std::numeric_limits<double>::max();
        for ( auto address : addresses )
        {
            double currentMinimum;
            double currentMaximum;
            resultsData->ensureKnownResultLoaded( address );
            resultsData->minMaxCellScalarValues( address, timeStepIdx, currentMinimum, currentMaximum );
            globalMin = std::min( globalMin, currentMinimum );
            globalMax = std::max( globalMax, currentMaximum );
        }
        return { globalMin, globalMax };
    };

    std::tie( m_minimumPressure, m_maximumPressure ) =
        updateBoundaryValues( resultsData, { RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" ) }, timeStepIdx );

    std::vector<double> volume =
        RigWellTargetCandidatesGenerator::getVolumeVector( *resultsData, m_volumeType(), m_volumesType(), m_volumeResultType(), timeStepIdx );
    if ( !volume.empty() )
    {
        const auto [min, max] = std::minmax_element( volume.begin(), volume.end() );
        m_minimumVolume       = *min;
        m_maximumVolume       = *max;
    }

    std::tie( m_minimumPermeability, m_maximumPermeability ) =
        updateBoundaryValues( resultsData,
                              { RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMX" ),
                                RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMY" ),
                                RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMZ" ) },
                              0 );

    std::tie( m_minimumTransmissibility, m_maximumTransmissibility ) =
        updateBoundaryValues( resultsData,
                              { RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANX" ),
                                RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANY" ),
                                RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANZ" ) },
                              0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetCandidatesGenerator::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                              QString                    uiConfigName,
                                                              caf::PdmUiEditorAttribute* attribute )

{
    if ( field == &m_volume && m_minimumVolume != cvf::UNDEFINED_DOUBLE && m_maximumVolume != cvf::UNDEFINED_DOUBLE )
    {
        if ( auto doubleAttributes = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            doubleAttributes->m_minimum  = m_minimumVolume;
            doubleAttributes->m_maximum  = m_maximumVolume;
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RimWellTargetCandidatesGenerator::getResultGridCellCount() const
{
    return cvf::Vec3st( m_cellCountI, m_cellCountJ, m_cellCountK );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetCandidatesGenerator::generateCandidates( RimEclipseCase* eclipseCase )
{
    RigWellTargetCandidatesGenerator::ClusteringLimits limits = getClusteringLimits();
    RigWellTargetCandidatesGenerator::generateCandidates( eclipseCase, m_timeStep(), m_volumeType(), m_volumesType(), m_volumeResultType(), limits );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetCandidatesGenerator::generateEnsembleStatistics()
{
    auto ensemble = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
    if ( !ensemble ) return;

    const cvf::Vec3st&                                 resultGridCellCount = getResultGridCellCount();
    RigWellTargetCandidatesGenerator::ClusteringLimits limits              = getClusteringLimits();
    RimRegularGridCase* regularGridCase = RigWellTargetCandidatesGenerator::generateEnsembleCandidates( *ensemble,
                                                                                                        m_timeStep(),
                                                                                                        resultGridCellCount,
                                                                                                        m_volumeType(),
                                                                                                        m_volumesType(),
                                                                                                        m_volumeResultType(),
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
void RimWellTargetCandidatesGenerator::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* resultGroup = uiOrdering.addNewGroup( "Result" );
    resultGroup->add( &m_timeStep );
    resultGroup->add( &m_volumeType );
    resultGroup->add( &m_volumeResultType );
    resultGroup->add( &m_volumesType );

    caf::PdmUiGroup* clusterLimitsGroup = uiOrdering.addNewGroup( "Cluster Growth Limits" );
    clusterLimitsGroup->add( &m_volume );
    clusterLimitsGroup->add( &m_pressure );
    clusterLimitsGroup->add( &m_permeability );
    clusterLimitsGroup->add( &m_transmissibility );

    caf::PdmUiGroup* resultDefinitionGroup = uiOrdering.addNewGroup( "Cluster Filter" );
    m_resultDefinition->uiOrdering( uiConfigName, *resultDefinitionGroup );

    auto hasEnsembleParent = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>() != nullptr;

    if ( hasEnsembleParent )
    {
        caf::PdmUiGroup* ensembleGridGroup = uiOrdering.addNewGroup( "Ensemble Statistics Grid" );
        ensembleGridGroup->add( &m_cellCountI );
        ensembleGridGroup->add( &m_cellCountJ );
        ensembleGridGroup->add( &m_cellCountK );
    }

    caf::PdmUiGroup* advancedGroup = uiOrdering.addNewGroup( "Advanced" );
    advancedGroup->add( &m_maxIterations );
    advancedGroup->add( &m_maxClusters );

    uiOrdering.add( &m_generateButton );

    uiOrdering.skipRemainingFields();

    if ( m_minimumVolume == cvf::UNDEFINED_DOUBLE || m_maximumVolume == cvf::UNDEFINED_DOUBLE )
    {
        updateAllBoundaries();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellTargetCandidatesGenerator::ClusteringLimits RimWellTargetCandidatesGenerator::getClusteringLimits() const
{
    return { .volume           = m_volume,
             .permeability     = m_permeability,
             .pressure         = m_pressure,
             .transmissibility = m_transmissibility,
             .maxClusters      = m_maxClusters,
             .maxIterations    = m_maxIterations,
             .filterAddress    = m_resultDefinition->eclipseResultAddress() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimWellTargetCandidatesGenerator::firstCase() const
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
void RimWellTargetCandidatesGenerator::initAfterRead()
{
    RimEclipseCase* eclipseCase = firstCase();
    if ( eclipseCase ) m_resultDefinition->setEclipseCase( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetCandidatesGenerator::updateResultDefinition()
{
    RimEclipseCase* eclipseCase = firstCase();
    if ( eclipseCase ) m_resultDefinition->setEclipseCase( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimWellTargetCandidatesGenerator::ensembleStatisticsCase() const
{
    return m_ensembleStatisticsCase;
}
