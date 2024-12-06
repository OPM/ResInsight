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

#include "RiaLogging.h"
#include "RiaPorosityModel.h"
#include "RiaResultNames.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "Well/RigWellTargetCandidatesGenerator.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseView.h"
#include "RimProject.h"
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
    CAF_PDM_InitObject( "Well Target Candidates Generator" );

    CAF_PDM_InitFieldNoDefault( &m_targetCase, "TargetCase", "Target Case" );
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

    CAF_PDM_InitField( &m_generateEnsembleStatistics, "GenerateEnsembleStatistics", true, "Generate Ensemble Statistics" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_generateEnsembleStatistics );

    m_minimumVolume = cvf::UNDEFINED_DOUBLE;
    m_maximumVolume = cvf::UNDEFINED_DOUBLE;

    m_minimumPressure = cvf::UNDEFINED_DOUBLE;
    m_maximumPressure = cvf::UNDEFINED_DOUBLE;

    m_minimumPermeability = cvf::UNDEFINED_DOUBLE;
    m_maximumPermeability = cvf::UNDEFINED_DOUBLE;

    m_minimumTransmissibility = cvf::UNDEFINED_DOUBLE;
    m_maximumTransmissibility = cvf::UNDEFINED_DOUBLE;
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
    generateCandidates( m_targetCase() );

    if ( changedField == &m_generateEnsembleStatistics )
    {
        generateEnsembleStatistics();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellTargetCandidatesGenerator::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_targetCase )
    {
        auto ensemble = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
        if ( !ensemble ) return options;

        if ( ensemble->cases().empty() ) return options;

        for ( auto eclipseCase : ensemble->cases() )
        {
            options.push_back( caf::PdmOptionItemInfo( eclipseCase->caseUserDescription(), eclipseCase, false, eclipseCase->uiIconProvider() ) );
        }
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        if ( m_targetCase() )
        {
            RimTools::timeStepsForCase( m_targetCase(), &options );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetCandidatesGenerator::updateAllBoundaries()
{
    auto ensemble = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
    if ( !ensemble ) return;

    if ( ensemble->cases().empty() ) return;

    RimEclipseCase* eclipseCase = ensemble->cases().front();

    int timeStepIdx = m_timeStep();

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

    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
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

    if ( field == &m_generateEnsembleStatistics )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Create Ensemble Statistics";
        }
    }
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

    RigWellTargetCandidatesGenerator::ClusteringLimits limits = getClusteringLimits();
    RigWellTargetCandidatesGenerator::generateEnsembleCandidates( *m_targetCase(),
                                                                  *ensemble,
                                                                  m_timeStep(),
                                                                  m_volumeType(),
                                                                  m_volumesType(),
                                                                  m_volumeResultType(),
                                                                  limits );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellTargetCandidatesGenerator::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    PdmObject::defineUiOrdering( uiConfigName, uiOrdering );

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
    RigWellTargetCandidatesGenerator::ClusteringLimits limits;
    limits.volume           = m_volume;
    limits.permeability     = m_permeability;
    limits.pressure         = m_pressure;
    limits.transmissibility = m_transmissibility;
    limits.maxClusters      = m_maxClusters;
    limits.maxIterations    = m_maxIterations;

    return limits;
}
