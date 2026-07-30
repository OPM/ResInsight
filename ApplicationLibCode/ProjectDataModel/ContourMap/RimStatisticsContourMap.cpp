/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimStatisticsContourMap.h"

#include "RiaLogging.h"
#include "RiaPreferencesGrid.h"
#include "RiaQStringFormatter.h"
#include "RigStatisticsTools.h"

#include "RicNewStatisticsContourMapViewFeature.h"

#include "RifReaderSettings.h"
#include "RifSurfio.h"

#include "ContourMap/RigContourMapCalculator.h"
#include "ContourMap/RigContourMapGrid.h"
#include "ContourMap/RigEclipseContourMapProjection.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFormationNames.h"
#include "RigMainGrid.h"
#include "RigPolyLinesData.h"
#include "RigStatisticsMath.h"

#include "Formations/RimFormationNames.h"
#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimReservoirGridEnsemble.h"
#include "RimSimWellInViewCollection.h"
#include "RimStatisticsContourMapProjection.h"
#include "RimStatisticsContourMapView.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiButton.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafProgressInfo.h"

#include <QCryptographicHash>
#include <QDir>
#include <QRegularExpression>
#include <QUuid>

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <set>

namespace
{
std::optional<std::set<int>>
    findKLayersForFormations( RimEclipseCase* eCase, const std::vector<QString>& selectedFormations, RimFormationNames* fallbackFormationNames )
{
    if ( selectedFormations.empty() ) return std::set<int>{};

    auto formationNames = eCase->activeFormationNames();
    if ( !formationNames ) formationNames = fallbackFormationNames;
    if ( !formationNames ) return std::nullopt;

    auto fData = formationNames->formationNamesData();
    if ( !fData ) return std::nullopt;

    return fData->findKLayers( selectedFormations );
}

void extractCaseResults( RigEclipseContourMapProjection&                     projection,
                         const RigEclipseResultAddress&                      resultAddress,
                         bool                                                hasDynamicResult,
                         RigContourMapCalculator::ResultAggregationType      resultAggregation,
                         RigFloodingSettings&                                floodSettings,
                         const std::vector<std::pair<int, int>>&             localToGlobalTimeSteps,
                         std::map<size_t, std::vector<std::vector<double>>>& timestepResults )
{
    if ( hasDynamicResult )
    {
        for ( auto [localTs, globalTs] : localToGlobalTimeSteps )
        {
            timestepResults[globalTs].push_back( projection.generateResults( resultAddress, resultAggregation, localTs, floodSettings ) );
        }
    }
    else
    {
        timestepResults[0].push_back( projection.generateResults( resultAddress, resultAggregation, 0, floodSettings ) );
    }
}
} // namespace

CAF_PDM_SOURCE_INIT( RimStatisticsContourMap, "RimStatisticalContourMap" );

namespace caf
{
template <>
void caf::AppEnum<RimStatisticsContourMap::GridImportMode>::setUp()
{
    addItem( RimStatisticsContourMap::GridImportMode::SHARED_GRID, "SHARED_GRID", "Reuse Grid from First Realization" );
    addItem( RimStatisticsContourMap::GridImportMode::INDIVIDUAL_GRIDS, "INDIVIDUAL_GRIDS", "Import All Grids" );
    setDefault( RimStatisticsContourMap::GridImportMode::SHARED_GRID );
}

template <>
void caf::AppEnum<RimStatisticsContourMap::StatisticsType>::setUp()
{
    addItem( RimStatisticsContourMap::StatisticsType::P10, "P10", "P10" );
    addItem( RimStatisticsContourMap::StatisticsType::P50, "P50", "P50" );
    addItem( RimStatisticsContourMap::StatisticsType::P90, "P90", "P90" );
    addItem( RimStatisticsContourMap::StatisticsType::MEAN, "MEAN", "Mean" );
    addItem( RimStatisticsContourMap::StatisticsType::MIN, "MIN", "Minimum" );
    addItem( RimStatisticsContourMap::StatisticsType::MAX, "MAX", "Maximum" );
    setDefault( RimStatisticsContourMap::StatisticsType::MEAN );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsContourMap::RimStatisticsContourMap()
    : m_openEclipseCase( nullptr )
{
    CAF_PDM_InitObject( "Ensemble Contour Map", ":/Histogram16x16.png" );

    CAF_PDM_InitField( &m_boundingBoxExpPercent,
                       "BoundingBoxExpPercent",
                       5.0,
                       "Bounding Box Expansion (%)",
                       "",
                       "How much to increase the bounding box of the primary case to cover for any grid size differences across the "
                       "ensemble." );

    CAF_PDM_InitFieldNoDefault( &m_resolution, "Resolution", "Sampling Resolution" );

    CAF_PDM_InitFieldNoDefault( &m_gridImportMode, "GridImportMode", "Grid Import Mode" );

    CAF_PDM_InitFieldNoDefault( &m_resultAggregation, "ResultAggregation", "Result Aggregation" );

    CAF_PDM_InitFieldNoDefault( &m_oilFloodingType, "OilFloodingType", "Residual Oil Given By" );
    m_oilFloodingType.setValue( RigFloodingSettings::FloodingType::WATER_FLOODING );
    CAF_PDM_InitField( &m_userDefinedFloodingOil, "UserDefinedFloodingOil", 0.0, "" );
    m_userDefinedFloodingOil.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_gasFloodingType, "GasFloodingType", RigFloodingSettings::FloodingType::GAS_FLOODING, "Residual Gas Given By" );
    caf::AppEnum<RigFloodingSettings::FloodingType>::setEnumSubset( &m_gasFloodingType,
                                                                    { RigFloodingSettings::FloodingType::GAS_FLOODING,
                                                                      RigFloodingSettings::FloodingType::USER_DEFINED } );

    CAF_PDM_InitField( &m_userDefinedFloodingGas, "UserDefinedFloodingGas", 0.0, "" );
    m_userDefinedFloodingGas.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "SelectedTimeSteps", "Time Step Selection" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_resultDefinition, "ResultDefinition", "" );
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_resultDefinition = new RimEclipseResultDefinition;
    m_resultDefinition->findField( "MResultType" )->uiCapability()->setUiName( "Result" );
    m_resultDefinition->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
    m_resultDefinition->setResultVariable( "SOIL" );

    CAF_PDM_InitFieldNoDefault( &m_primaryCase,
                                "PrimaryEclipseCase",
                                "Primary Case",
                                "",
                                "Eclipse Case used for wells and faults shown in views, initializing available result list, timesteps, "
                                "etc." );

    CAF_PDM_InitFieldNoDefault( &m_views, "ContourMapViews", "Contour Maps", ":/CrossSection16x16.png" );

    CAF_PDM_InitField( &m_enableFormationFilter, "EnableFormationFilter", false, "Enable Formation Filter" );
    CAF_PDM_InitFieldNoDefault( &m_selectedFormations, "Formations", "Select Formations" );
    m_selectedFormations.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedFormations.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );

    CAF_PDM_InitFieldNoDefault( &m_selectedPolygons, "Polygons", "Select Polygons" );
    m_selectedPolygons.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedPolygons.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );

    CAF_PDM_InitField( &m_cacheFileBaseName, "CacheFileBaseName", QString(), "Cache File Base Name" );
    m_cacheFileBaseName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_cacheValidityKey, "CacheValidityKey", QString(), "Cache Validity Key" );
    m_cacheValidityKey.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_cacheTimeSteps, "CacheTimeSteps", "Cache Time Steps" );
    m_cacheTimeSteps.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_cacheSampleSpacing, "CacheSampleSpacing", 0.0, "Cache Sample Spacing" );
    m_cacheSampleSpacing.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_cacheOriginalBoundingBox, "CacheOriginalBoundingBox", "Cache Original Bounding Box" );
    m_cacheOriginalBoundingBox.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_cacheExpandedBoundingBox, "CacheExpandedBoundingBox", "Cache Expanded Bounding Box" );
    m_cacheExpandedBoundingBox.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( ( eclipseCase() == nullptr ) && ( !ensembleCases().empty() ) )
    {
        auto selCase = ensembleCases().front();
        setEclipseCase( selCase );
    }

    bool computeOK = !( m_enableFormationFilter && m_selectedFormations().empty() );
    computeOK      = computeOK && !selectedTimeSteps().empty();

    uiOrdering.add( nameField() );

    {
        auto* btn = uiOrdering.addNewButton( "Compute", [this]() { onComputeStatisticsClicked(); } );
        btn->setUiToolTip( computeOK ? "Start statistics computations." : "Please check your time step and/or formation filter selections." );
        btn->setUiReadOnly( !computeOK );
    }

    auto genGrp = uiOrdering.addNewGroup( "General" );

    genGrp->add( &m_resultAggregation );

    if ( RigContourMapCalculator::isMobileColumnResult( m_resultAggregation() ) )
    {
        if ( m_resultAggregation() != RigContourMapCalculator::MOBILE_GAS_COLUMN )
        {
            genGrp->add( &m_oilFloodingType );
            if ( m_oilFloodingType() == RigFloodingSettings::FloodingType::USER_DEFINED )
            {
                genGrp->add( &m_userDefinedFloodingOil );
            }
        }
        if ( m_resultAggregation() != RigContourMapCalculator::MOBILE_OIL_COLUMN )
        {
            genGrp->add( &m_gasFloodingType );
            if ( m_gasFloodingType() == RigFloodingSettings::FloodingType::USER_DEFINED )
            {
                genGrp->add( &m_userDefinedFloodingGas );
            }
        }
    }

    genGrp->add( &m_resolution );

    if ( auto* gridEnsemble = firstAncestorOrThisOfType<RimReservoirGridEnsembleBase>() )
    {
        if ( gridEnsemble->gridMode() == RimReservoirGridEnsembleBase::GridModeType::SHARED_GRID ) genGrp->add( &m_gridImportMode );
    }

    genGrp->add( &m_primaryCase );
    genGrp->add( &m_boundingBoxExpPercent );

    auto tsGroup = uiOrdering.addNewGroup( "Time Step Selection" );
    tsGroup->setCollapsedByDefault();
    tsGroup->add( &m_selectedTimeSteps );

    if ( activeFormationNames() )
    {
        auto formationGrp = uiOrdering.addNewGroup( "Formation Selection" );
        if ( !m_enableFormationFilter ) formationGrp->setCollapsedByDefault();
        formationGrp->add( &m_enableFormationFilter );
        if ( m_enableFormationFilter ) formationGrp->add( &m_selectedFormations );
    }

    if ( auto polygonCollection = RimTools::polygonCollection() )
    {
        if ( !polygonCollection->allPolygons().empty() )
        {
            auto polyGrp = uiOrdering.addNewGroup( "Polygon Selection" );
            polyGrp->setCollapsedByDefault();
            polyGrp->add( &m_selectedPolygons );
        }
    }

    if ( !isColumnResult() )
    {
        auto resultDefinitionGroup = uiOrdering.addNewGroup( "Result Definition" );
        m_resultDefinition->uiOrdering( uiConfigName, *resultDefinitionGroup );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::setEclipseCase( RimEclipseCase* eCase )
{
    m_resultDefinition->setEclipseCase( eCase );
    m_primaryCase = eCase;

    if ( eCase != nullptr )
    {
        if ( m_selectedTimeSteps().empty() )
        {
            int nSteps = (int)eCase->timeStepStrings().size();
            if ( nSteps > 0 )
            {
                m_selectedTimeSteps.setValue( { nSteps - 1 } );
            }
        }
    }

    for ( auto& view : m_views )
    {
        view->setEclipseCase( eCase );
    }
    m_resultDefinition->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::setGridImportMode( GridImportMode mode )
{
    m_gridImportMode = mode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMap::ensembleName() const
{
    if ( auto* ens = firstAncestorOrThisOfType<RimReservoirGridEnsembleBase>() ) return ens->ensembleName();
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames* RimStatisticsContourMap::activeFormationNames() const
{
    if ( auto* gridCase = eclipseCase() )
    {
        if ( auto* formationNames = gridCase->activeFormationNames() ) return formationNames;
    }
    if ( auto* ensemble = firstAncestorOrThisOfType<RimReservoirGridEnsembleBase>() )
    {
        return ensemble->activeFormationNames();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimStatisticsContourMap::ensembleCases() const
{
    if ( auto* ens = firstAncestorOrThisOfType<RimReservoirGridEnsembleBase>() ) return ens->sourceCases();

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimEclipseCase*> RimStatisticsContourMap::ensembleCasesInViews() const
{
    if ( auto* ens = firstAncestorOrThisOfType<RimReservoirGridEnsembleBase>() ) return ens->casesInViews();
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &m_primaryCase == changedField )
    {
        switchToSelectedSourceCase();

        // Update well views as wells might have changed from last case
        for ( auto& view : m_views )
        {
            view->wellCollection()->wells.deleteChildren();
            view->updateDisplayModelForWellResults();
            view->wellCollection()->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::switchToSelectedSourceCase()
{
    auto newCase = eclipseCase();
    if ( newCase == nullptr ) return;

    if ( m_openEclipseCase != newCase )
    {
        newCase->ensureReservoirCaseIsOpen();

        if ( m_openEclipseCase && !ensembleCasesInViews().contains( m_openEclipseCase ) )
        {
            m_openEclipseCase->closeReservoirCase();
        }
        m_openEclipseCase = newCase;
        setEclipseCase( newCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimStatisticsContourMap::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( &m_selectedTimeSteps == fieldNeedingOptions )
    {
        if ( auto eCase = eclipseCase() )
        {
            const auto timeStepStrings = eCase->timeStepStrings();

            int index = 0;
            for ( const auto& text : timeStepStrings )
            {
                options.push_back( caf::PdmOptionItemInfo( text, index++ ) );
            }
        }
        return options;
    }
    else if ( &m_primaryCase == fieldNeedingOptions )
    {
        for ( auto eCase : ensembleCases() )
        {
            options.push_back( caf::PdmOptionItemInfo( eCase->caseUserDescription(), eCase, false, eCase->uiIconProvider() ) );
        }
        return options;
    }
    else if ( &m_selectedFormations == fieldNeedingOptions )
    {
        if ( auto formations = activeFormationNames() )
        {
            if ( formations->formationNamesData() )
            {
                for ( auto& f : formations->formationNamesData()->formationNames() )
                {
                    options.push_back( caf::PdmOptionItemInfo( f, f, false ) );
                }
            }
        }
    }
    else if ( &m_selectedPolygons == fieldNeedingOptions )
    {
        if ( auto polygonCollection = RimTools::polygonCollection() )
        {
            for ( auto p : polygonCollection->allPolygons() )
            {
                options.push_back( caf::PdmOptionItemInfo( p->name(), p, false ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( ( &m_userDefinedFloodingOil == field ) || ( &m_userDefinedFloodingGas == field ) )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum                       = 0.0;
            myAttr->m_maximum                       = 1.0;
            myAttr->m_sliderTickCount               = 20;
            myAttr->m_delaySliderUpdateUntilRelease = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::initAfterRead()
{
    if ( ensembleCases().empty() ) return;

    switchToSelectedSourceCase();

    for ( auto view : m_views.childrenByType() )
    {
        view->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::doStatisticsCalculation( TimestepResultsMap& timestepResults )
{
    m_timeResults.clear();

    for ( const auto& [timeStep, res] : timestepResults )
    {
        if ( res.empty() ) continue;

        int                 nCells = static_cast<int>( res[0].size() );
        std::vector<double> p10Results( nCells, std::numeric_limits<double>::infinity() );
        std::vector<double> p50Results( nCells, std::numeric_limits<double>::infinity() );
        std::vector<double> p90Results( nCells, std::numeric_limits<double>::infinity() );
        std::vector<double> meanResults( nCells, std::numeric_limits<double>::infinity() );
        std::vector<double> minResults( nCells, std::numeric_limits<double>::infinity() );
        std::vector<double> maxResults( nCells, std::numeric_limits<double>::infinity() );

        const size_t numSamples = res.size();

// Clang version 16.0.6 does not handle OpenMP here, the compiler crashes.
#ifndef __clang__
#pragma omp parallel for
#endif
        for ( int i = 0; i < nCells; i++ )
        {
            std::vector<double> samples( numSamples, 0.0 );
            for ( size_t s = 0; s < numSamples; s++ )
            {
                samples[s] = res[s][i];
            }

            double p10  = std::numeric_limits<double>::infinity();
            double p50  = std::numeric_limits<double>::infinity();
            double p90  = std::numeric_limits<double>::infinity();
            double mean = std::numeric_limits<double>::infinity();

            RigStatisticsMath::calculateStatisticsCurves( samples, &p10, &p50, &p90, &mean, RigStatisticsMath::PercentileStyle::SWITCHED );

            if ( RigStatisticsTools::isValidNumber( p10 ) ) p10Results[i] = p10;
            if ( RigStatisticsTools::isValidNumber( p50 ) ) p50Results[i] = p50;
            if ( RigStatisticsTools::isValidNumber( p90 ) ) p90Results[i] = p90;
            if ( RigStatisticsTools::isValidNumber( mean ) ) meanResults[i] = mean;

            double minValue = RigStatisticsTools::minimumValue( samples );
            if ( RigStatisticsTools::isValidNumber( minValue ) && minValue < std::numeric_limits<double>::max() ) minResults[i] = minValue;

            double maxValue = RigStatisticsTools::maximumValue( samples );
            if ( RigStatisticsTools::isValidNumber( maxValue ) && maxValue > -std::numeric_limits<double>::max() ) maxResults[i] = maxValue;
        }

        m_timeResults[timeStep][StatisticsType::P10]  = p10Results;
        m_timeResults[timeStep][StatisticsType::P50]  = p50Results;
        m_timeResults[timeStep][StatisticsType::P90]  = p90Results;
        m_timeResults[timeStep][StatisticsType::MEAN] = meanResults;
        m_timeResults[timeStep][StatisticsType::MIN]  = minResults;
        m_timeResults[timeStep][StatisticsType::MAX]  = maxResults;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::onComputeStatisticsClicked()
{
    computeStatistics();

    if ( m_views.empty() )
    {
        auto view = RicNewStatisticsContourMapViewFeature::createAndAddView( this );
        updateConnectedEditors();
        Riu3DMainWindowTools::selectAsCurrentItem( view );
        Riu3DMainWindowTools::setExpanded( this );
        Riu3DMainWindowTools::setExpanded( view );
    }
    else
    {
        for ( auto& view : m_views )
        {
            auto proj = dynamic_cast<RimStatisticsContourMapProjection*>( view->contourMapProjection() );
            if ( proj != nullptr )
                proj->clearGridMappingAndRedraw();
            else
                view->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::computeStatistics()
{
    computeStatisticsForMaps( { this } );
}

//--------------------------------------------------------------------------------------------------
/// Compute statistics for several contour maps in one sweep over the ensemble realizations, so that
/// each realization is opened once instead of once per contour map. All maps must belong to the
/// same ensemble.
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::computeStatisticsForMaps( const std::vector<RimStatisticsContourMap*>& maps )
{
    struct MapContext
    {
        explicit MapContext( RimStatisticsContourMap* m )
            : map( m )
            , floodSettings( m->m_oilFloodingType(), m->m_userDefinedFloodingOil(), m->m_gasFloodingType(), m->m_userDefinedFloodingGas() )
            , resultAggregation( m->m_resultAggregation() )
        {
        }

        RimStatisticsContourMap*                        map;
        RigFloodingSettings                             floodSettings;
        RigContourMapCalculator::ResultAggregationType  resultAggregation;
        std::unique_ptr<RigContourMapGrid>              contourMapGrid;
        TimestepResultsMap                              timestepResults;
        bool                                            useSharedGrid = false;
        std::unique_ptr<RigEclipseContourMapProjection> sharedProjection;
        std::set<int>                                   kLayers;
        bool                                            active = false;
    };

    auto readerSettings                = RiaPreferencesGrid::gridOnlyReaderSettings();
    readerSettings.onlyLoadActiveCells = true;

    auto oldReaderType = RiaPreferencesGrid::current()->gridModelReaderOverride();
    RiaPreferencesGrid::current()->setGridModelReaderOverride( RiaDefines::GridModelReader::OPM_COMMON );

    std::map<RimEclipseCase*, RifReaderSettings> primaryOldSettings;

    std::vector<MapContext>            contexts;
    std::set<RimStatisticsContourMap*> uniqueMaps;

    for ( RimStatisticsContourMap* map : maps )
    {
        if ( map == nullptr || !uniqueMaps.insert( map ).second ) continue;
        if ( map->ensembleCases().empty() ) continue;

        RimEclipseCase* primaryCase = map->eclipseCase();
        if ( primaryCase == nullptr ) continue;

        if ( !primaryOldSettings.contains( primaryCase ) )
        {
            primaryOldSettings[primaryCase] = primaryCase->readerSettings();
            primaryCase->setReaderSettings( readerSettings );
        }

        // A sibling map can be computed before its own initAfterRead() has run, and then the result
        // definition has no case to resolve the result address against, producing empty results
        if ( map->m_resultDefinition->eclipseCase() == nullptr ) map->m_resultDefinition->setEclipseCase( primaryCase );

        MapContext ctx( map );

        auto gridEnsemble = map->firstAncestorOrThisOfType<RimReservoirGridEnsembleBase>();
        ctx.useSharedGrid = gridEnsemble && gridEnsemble->gridMode() == RimReservoirGridEnsembleBase::GridModeType::SHARED_GRID &&
                            map->m_gridImportMode() == GridImportMode::SHARED_GRID;

        ctx.active = primaryCase->ensureReservoirCaseIsOpen();

        // The bounding box is empty unless the primary case is open with active cell info
        cvf::BoundingBox gridBoundingBox = primaryCase->activeCellsBoundingBox();
        gridBoundingBox.expandPercent( map->m_boundingBoxExpPercent() );

        double sampleSpacing = 1.0;
        if ( auto mainGrid = primaryCase->mainGrid() ) sampleSpacing = map->sampleSpacingFactor() * mainGrid->characteristicIJCellSize();

        ctx.contourMapGrid = std::make_unique<RigContourMapGrid>( gridBoundingBox, sampleSpacing );

        if ( ctx.active )
        {
            if ( ctx.useSharedGrid )
            {
                if ( auto kLayers = findKLayersForFormations( primaryCase, map->selectedFormations(), map->activeFormationNames() ) )
                {
                    ctx.kLayers = *kLayers;

                    auto primaryCaseData   = primaryCase->eclipseCaseData();
                    auto primaryResultData = primaryCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

                    ctx.sharedProjection =
                        std::make_unique<RigEclipseContourMapProjection>( ctx.contourMapGrid.get(), primaryCaseData, primaryResultData );
                    ctx.sharedProjection->generateGridMapping( ctx.resultAggregation, {}, ctx.kLayers, map->selectedPolygons() );
                }
                else
                {
                    RiaLogging::warning( "Formation names are missing for primary case, skipping statistics computation." );
                    ctx.active = false;
                }
            }
        }

        contexts.push_back( std::move( ctx ) );
    }

    const bool anyActive = std::any_of( contexts.begin(), contexts.end(), []( const MapContext& ctx ) { return ctx.active; } );

    if ( anyActive )
    {
        RiaLogging::info( std::format( "Computing statistics for {} ensemble contour map(s)", contexts.size() ) );

        // All maps belong to the same ensemble, so the realization cases are shared
        auto cases        = contexts.front().map->ensembleCases();
        auto casesInViews = contexts.front().map->ensembleCasesInViews();

        std::set<RimEclipseCase*> primaryCases;
        for ( const auto& ctx : contexts )
            primaryCases.insert( ctx.map->eclipseCase() );

        const size_t      nCases = cases.size();
        caf::ProgressInfo progInfo( nCases, QString( "Reading Eclipse Ensemble" ) );
        int               i = 1;

        // The key point of this loop is that each realization is opened once, contributes to all pending contour maps,
        // and is then closed again to release memory.
        for ( RimEclipseCase* eCase : cases )
        {
            auto task = progInfo.task( QString( "Processing Case %1 of %2" ).arg( i++ ).arg( nCases ) );

            RifReaderSettings oldSettings = eCase->readerSettings();
            eCase->setReaderSettings( readerSettings );

            if ( eCase->ensureReservoirCaseIsOpen() )
            {
                RiaLogging::info( std::format( "Processing Grid: {}", eCase->caseUserDescription() ) );

                auto eclipseCaseData = eCase->eclipseCaseData();
                auto activeCellInfo  = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
                auto resultData      = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

                for ( auto& ctx : contexts )
                {
                    if ( !ctx.active ) continue;

                    RimStatisticsContourMap* map                    = ctx.map;
                    auto                     localToGlobalTimeSteps = map->mapLocalToGlobalTimeSteps( eCase->timeStepDates() );

                    if ( ctx.useSharedGrid )
                    {
                        ctx.sharedProjection->updateRealizationData( activeCellInfo, resultData );
                        extractCaseResults( *ctx.sharedProjection,
                                            map->m_resultDefinition()->eclipseResultAddress(),
                                            map->m_resultDefinition()->hasDynamicResult(),
                                            ctx.resultAggregation,
                                            ctx.floodSettings,
                                            localToGlobalTimeSteps,
                                            ctx.timestepResults );
                    }
                    else
                    {
                        if ( auto kLayers = findKLayersForFormations( eCase, map->selectedFormations(), map->activeFormationNames() ) )
                        {
                            RigEclipseContourMapProjection contourMapProjection( ctx.contourMapGrid.get(), eclipseCaseData, resultData );
                            contourMapProjection.generateGridMapping( ctx.resultAggregation, {}, *kLayers, map->selectedPolygons() );
                            extractCaseResults( contourMapProjection,
                                                map->m_resultDefinition()->eclipseResultAddress(),
                                                map->m_resultDefinition()->hasDynamicResult(),
                                                ctx.resultAggregation,
                                                ctx.floodSettings,
                                                localToGlobalTimeSteps,
                                                ctx.timestepResults );
                        }
                        else
                        {
                            RiaLogging::warning(
                                std::format( "Formation names are missing for case {}, skipping case.", eCase->caseUserDescription() ) );
                        }
                    }
                }
            }

            eCase->setReaderSettings( oldSettings );

            // Release the grid data for cases that were opened only to compute statistics. A case is kept open if it has
            // its own views, if it is the primary case of one of the contour maps, or if it is displayed in one of the
            // ensemble views.
            if ( eCase->views().empty() && !primaryCases.contains( eCase ) && !casesInViews.contains( eCase ) )
            {
                eCase->closeReservoirCase();
            }
        }
    }

    for ( auto& [primaryCase, settings] : primaryOldSettings )
        primaryCase->setReaderSettings( settings );

    RiaPreferencesGrid::current()->setGridModelReaderOverride( oldReaderType );

    for ( auto& ctx : contexts )
    {
        ctx.map->m_contourMapGrid = std::move( ctx.contourMapGrid );
        ctx.map->doStatisticsCalculation( ctx.timestepResults );
        ctx.map->m_computedValidityKey = ctx.active ? ctx.map->computeCacheValidityKey() : QString();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStatisticsContourMap::eclipseCase() const
{
    return m_primaryCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewStatisticsContourMapViewFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigContourMapGrid* RimStatisticsContourMap::contourMapGrid() const
{
    return m_contourMapGrid.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStatisticsContourMap::result( size_t timeStep, StatisticsType statisticsType ) const
{
    auto realTimeSteps = selectedTimeSteps();
    if ( timeStep >= realTimeSteps.size() ) return {};

    timeStep = (size_t)realTimeSteps[timeStep];

    if ( !m_timeResults.contains( timeStep ) ) return {};

    if ( !m_timeResults.at( timeStep ).contains( statisticsType ) ) return {};

    return m_timeResults.at( timeStep ).at( statisticsType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RimStatisticsContourMap::selectedTimeSteps() const
{
    if ( !m_resultDefinition->hasDynamicResult() )
    {
        return { 0 };
    }

    auto steps = m_selectedTimeSteps();
    std::sort( steps.begin(), steps.end() );
    return steps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimStatisticsContourMap::selectedTimeStepDates() const
{
    std::vector<QDateTime> retDates;

    auto eCase = eclipseCase();
    if ( eCase != nullptr )
    {
        auto allDates = eCase->timeStepDates();
        for ( auto i : selectedTimeSteps() )
        {
            if ( i < (int)allDates.size() ) retDates.push_back( allDates[i] );
        }
    }
    return retDates;
}

//--------------------------------------------------------------------------------------------------
/// returns pair of (local date index, matching global date index)
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<int, int>> RimStatisticsContourMap::mapLocalToGlobalTimeSteps( std::vector<QDateTime> localDates ) const
{
    std::vector<std::pair<int, int>> indexSubset;

    auto globalDates   = selectedTimeStepDates();
    auto globalIndexes = selectedTimeSteps();

    for ( int i = 0; i < (int)localDates.size(); i++ )
    {
        auto pos = std::find( globalDates.begin(), globalDates.end(), localDates[i] );
        if ( pos == globalDates.end() ) continue;

        int foundIdx = (int)( pos - globalDates.begin() );
        indexSubset.emplace_back( i, globalIndexes[foundIdx] );
    }

    return indexSubset;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimStatisticsContourMap::selectedFormations() const
{
    if ( !m_enableFormationFilter ) return {};
    return m_selectedFormations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>> RimStatisticsContourMap::selectedPolygons() const
{
    std::vector<std::vector<cvf::Vec3d>> allLines;

    for ( auto p : m_selectedPolygons.ptrReferencedObjectsByType() )
    {
        auto pData = p->polyLinesData();
        if ( pData.isNull() ) continue;

        const std::vector<std::vector<cvf::Vec3d>> lines = pData->completePolyLines();
        for ( auto l : lines )
        {
            allLines.push_back( l );
        }
    }

    return allLines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMap::timeStepName( int timeStep ) const
{
    if ( eclipseCase() == nullptr ) return "";

    if ( ( timeStep < 0 ) || ( timeStep >= eclipseCase()->timeStepStrings().size() ) ) return "";

    return eclipseCase()->timeStepName( timeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::ensureResultsComputed()
{
    // The contour map grid is not stored in the project file, and is used as the flag telling if statistics have been
    // computed in this session. It is created by computeStatisticsForMaps(), and is never cleared.
    // Use the Compute button to force a recomputation after changing settings.
    if ( m_contourMapGrid ) return;

    if ( loadCachedResults() ) return;

    // Compute all pending sibling contour maps in the same sweep over the ensemble realizations, so
    // that each realization is opened once instead of once per contour map
    std::vector<RimStatisticsContourMap*> maps = { this };
    if ( auto ensemble = firstAncestorOrThisOfType<RimReservoirGridEnsembleBase>() )
    {
        for ( auto sibling : ensemble->statisticsContourMaps() )
        {
            if ( sibling != this && !sibling->m_contourMapGrid && !sibling->views().empty() && !sibling->loadCachedResults() )
                maps.push_back( sibling );
        }
    }

    computeStatisticsForMaps( maps );
}

//--------------------------------------------------------------------------------------------------
/// Hash of all settings that affect the computed statistics. Cached results are only reused when
/// the stored key matches the key computed from the current settings.
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMap::computeCacheValidityKey() const
{
    QStringList parts;

    parts << m_resultAggregation().text();
    parts << m_resolution().text();
    parts << m_gridImportMode().text();
    parts << QString::number( m_boundingBoxExpPercent(), 'g', 12 );

    parts << m_oilFloodingType().text() << QString::number( m_userDefinedFloodingOil(), 'g', 12 );
    parts << m_gasFloodingType().text() << QString::number( m_userDefinedFloodingGas(), 'g', 12 );

    if ( m_resultDefinition() != nullptr )
    {
        parts << caf::AppEnum<RiaDefines::ResultCatType>::text( m_resultDefinition->resultType() );
        parts << m_resultDefinition->resultVariable();
        parts << caf::AppEnum<RiaDefines::PorosityModelType>::text( m_resultDefinition->porosityModel() );
    }

    for ( int timeStep : selectedTimeSteps() )
        parts << QString::number( timeStep );

    parts << ( m_enableFormationFilter() ? "formationFilter" : "noFormationFilter" );
    for ( const QString& formation : m_selectedFormations() )
        parts << formation;

    for ( const auto& polygonLine : selectedPolygons() )
    {
        for ( const auto& point : polygonLine )
            parts << QString( "%1,%2,%3" ).arg( point.x(), 0, 'f', 3 ).arg( point.y(), 0, 'f', 3 ).arg( point.z(), 0, 'f', 3 );
    }

    if ( auto primaryCase = eclipseCase() ) parts << primaryCase->gridFileName();

    for ( RimEclipseCase* eCase : ensembleCases() )
        parts << eCase->gridFileName();

    const QByteArray hash = QCryptographicHash::hash( parts.join( ";" ).toUtf8(), QCryptographicHash::Md5 );
    return QString::fromLatin1( hash.toHex() );
}

//--------------------------------------------------------------------------------------------------
/// Restore the contour map grid and statistics results from the project-adjacent cache files.
/// Returns false when no valid cache exists, and computation is needed.
//--------------------------------------------------------------------------------------------------
bool RimStatisticsContourMap::loadCachedResults()
{
    if ( m_cacheFileBaseName().isEmpty() || m_cacheTimeSteps().empty() ) return false;
    if ( m_cacheSampleSpacing() <= 0.0 || m_cacheOriginalBoundingBox().size() != 6 || m_cacheExpandedBoundingBox().size() != 6 )
        return false;

    const QString expectedKey = computeCacheValidityKey();
    if ( m_cacheValidityKey().isEmpty() || m_cacheValidityKey() != expectedKey ) return false;

    auto toBoundingBox = []( const std::vector<double>& c )
    { return cvf::BoundingBox( cvf::Vec3d( c[0], c[1], c[2] ), cvf::Vec3d( c[3], c[4], c[5] ) ); };

    auto contourMapGrid = std::make_unique<RigContourMapGrid>( toBoundingBox( m_cacheOriginalBoundingBox() ),
                                                               toBoundingBox( m_cacheExpandedBoundingBox() ),
                                                               m_cacheSampleSpacing() );

    const cvf::Vec2ui& mapSize = contourMapGrid->mapSize();

    std::map<size_t, std::map<StatisticsType, std::vector<double>>> timeResults;
    for ( int timeStep : m_cacheTimeSteps() )
    {
        for ( size_t statisticsTypeIndex = 0; statisticsTypeIndex < caf::AppEnum<StatisticsType>::size(); ++statisticsTypeIndex )
        {
            const StatisticsType statisticsType = caf::AppEnum<StatisticsType>::fromIndex( statisticsTypeIndex );

            const QString fileName    = cacheFileName( m_cacheFileBaseName(), statisticsType, timeStep );
            auto          surfaceData = RifSurfio::importSurfaceData( fileName.toStdString() );
            if ( !surfaceData.has_value() )
            {
                RiaLogging::warning( std::format( "Failed to read ensemble contour map statistics cache, recomputing: {}", fileName ) );
                return false;
            }

            const auto& [regularSurface, values] = surfaceData.value();
            if ( regularSurface.nx != static_cast<int>( mapSize.x() ) || regularSurface.ny != static_cast<int>( mapSize.y() ) )
            {
                RiaLogging::warning(
                    std::format( "Ensemble contour map statistics cache does not match the sample grid, recomputing: {}", fileName ) );
                return false;
            }

            // Undefined cells are stored as undefined surface values, imported as NaN
            std::vector<double> doubleValues;
            doubleValues.reserve( values.size() );
            for ( float value : values )
            {
                doubleValues.push_back( std::isnan( value ) ? std::numeric_limits<double>::infinity() : value );
            }

            timeResults[timeStep][statisticsType] = std::move( doubleValues );
        }
    }

    if ( timeResults.empty() ) return false;

    m_contourMapGrid      = std::move( contourMapGrid );
    m_timeResults         = std::move( timeResults );
    m_computedValidityKey = expectedKey;

    RiaLogging::info(
        std::format( "Loaded ensemble contour map statistics from cache: {}/{}_*.gri", getCacheDirectoryPath(), m_cacheFileBaseName() ) );
    return true;
}

//--------------------------------------------------------------------------------------------------
/// Write one GRI surface file per statistics type per time step. The surface nodes are placed at
/// the cell centers of the contour map sample grid, so the files can be imported as regular
/// surfaces in ResInsight or other tools.
//--------------------------------------------------------------------------------------------------
bool RimStatisticsContourMap::writeCachedResults( const QString& baseName ) const
{
    if ( !m_contourMapGrid || m_timeResults.empty() ) return false;

    const cvf::Vec2ui& mapSize       = m_contourMapGrid->mapSize();
    const double       sampleSpacing = m_contourMapGrid->sampleSpacing();

    RigRegularSurfaceData surfaceData;
    surfaceData.nx         = static_cast<int>( mapSize.x() );
    surfaceData.ny         = static_cast<int>( mapSize.y() );
    surfaceData.originX    = m_contourMapGrid->expandedBoundingBox().min().x() + sampleSpacing / 2.0;
    surfaceData.originY    = m_contourMapGrid->expandedBoundingBox().min().y() + sampleSpacing / 2.0;
    surfaceData.incrementX = sampleSpacing;
    surfaceData.incrementY = sampleSpacing;
    surfaceData.rotation   = 0.0;

    for ( const auto& [timeStep, statisticsResults] : m_timeResults )
    {
        for ( const auto& [statisticsType, values] : statisticsResults )
        {
            if ( values.size() != static_cast<size_t>( surfaceData.nx ) * static_cast<size_t>( surfaceData.ny ) ) return false;

            // Undefined cells are stored as NaN, written as the undefined surface value
            std::vector<float> floatValues;
            floatValues.reserve( values.size() );
            for ( double value : values )
            {
                floatValues.push_back( std::isfinite( value ) ? static_cast<float>( value ) : std::numeric_limits<float>::quiet_NaN() );
            }

            const QString fileName = cacheFileName( baseName, statisticsType, timeStep );
            if ( !RifSurfio::exportToGri( fileName.toStdString(), surfaceData, floatValues ) ) return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Write the computed statistics to cache files next to the project file, so that the ensemble
/// sweep can be skipped when the project is loaded again with unchanged settings
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::setupBeforeSave()
{
    // Delete any existing cache files, they are rewritten below if valid results exist
    deleteCacheFiles();

    // Only write results computed from (or previously cached for) the current settings
    if ( !m_contourMapGrid || m_timeResults.empty() || m_computedValidityKey.isEmpty() )
    {
        clearCacheFields();
        return;
    }

    QDir::root().mkpath( getCacheDirectoryPath() );

    const QString baseName = getValidCacheFileBaseName();
    if ( writeCachedResults( baseName ) )
    {
        auto boundingBoxCoords = []( const cvf::BoundingBox& boundingBox ) -> std::vector<double>
        {
            const cvf::Vec3d& min = boundingBox.min();
            const cvf::Vec3d& max = boundingBox.max();
            return { min.x(), min.y(), min.z(), max.x(), max.y(), max.z() };
        };

        std::vector<int> timeStepIndices;
        for ( const auto& [timeStep, statisticsResults] : m_timeResults )
            timeStepIndices.push_back( static_cast<int>( timeStep ) );

        m_cacheFileBaseName        = baseName;
        m_cacheValidityKey         = m_computedValidityKey;
        m_cacheTimeSteps           = timeStepIndices;
        m_cacheSampleSpacing       = m_contourMapGrid->sampleSpacing();
        m_cacheOriginalBoundingBox = boundingBoxCoords( m_contourMapGrid->originalBoundingBox() );
        m_cacheExpandedBoundingBox = boundingBoxCoords( m_contourMapGrid->expandedBoundingBox() );
    }
    else
    {
        RiaLogging::warning( std::format( "Failed to write ensemble contour map statistics cache for '{}'", name() ) );
        deleteCacheFiles();
        clearCacheFields();
    }
}

//--------------------------------------------------------------------------------------------------
/// Full path of the cache file for one statistics type and time step, e.g.
/// "<project>_cache/Ensemble_Contour_Map_1-1a2b3c4d_MEAN_36.gri"
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMap::cacheFileName( const QString& baseName, StatisticsType statisticsType, size_t timeStep ) const
{
    return QString( "%1/%2_%3_%4.gri" )
        .arg( getCacheDirectoryPath(), baseName, caf::AppEnum<StatisticsType>::text( statisticsType ), QString::number( timeStep ) );
}

//--------------------------------------------------------------------------------------------------
/// Base file name of the cache files within the cache directory, derived from the contour map name
/// with a unique suffix. The name is kept stable once assigned.
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMap::getValidCacheFileBaseName() const
{
    if ( !m_cacheFileBaseName().isEmpty() ) return m_cacheFileBaseName();

    QString sanitizedName = name();
    sanitizedName.replace( QRegularExpression( "[^a-zA-Z0-9-]+" ), "_" );

    return sanitizedName + "-" + QUuid::createUuid().toString( QUuid::WithoutBraces ).left( 8 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::deleteCacheFiles() const
{
    if ( m_cacheFileBaseName().isEmpty() ) return;

    QDir cacheDir( getCacheDirectoryPath() );
    for ( const QString& fileName : cacheDir.entryList( { m_cacheFileBaseName() + "_*.gri" }, QDir::Files ) )
    {
        cacheDir.remove( fileName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::clearCacheFields()
{
    m_cacheFileBaseName        = QString();
    m_cacheValidityKey         = QString();
    m_cacheTimeSteps           = std::vector<int>();
    m_cacheSampleSpacing       = 0.0;
    m_cacheOriginalBoundingBox = std::vector<double>();
    m_cacheExpandedBoundingBox = std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMap::getCacheDirectoryPath()
{
    return RimTools::getCacheRootDirectoryPathFromProject() + "_cache";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMap::resultAggregationText() const
{
    return m_resultAggregation().uiText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStatisticsContourMap::resultVariable() const
{
    if ( m_resultDefinition().isNull() ) return "";
    return m_resultDefinition()->resultVariable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStatisticsContourMap::isColumnResult() const
{
    return RigContourMapCalculator::isColumnResult( m_resultAggregation() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStatisticsContourMap::sampleSpacingFactor() const
{
    return RimContourMapResolutionTools::resolutionFromEnumValue( m_resolution() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimStatisticsContourMapView*> RimStatisticsContourMap::views() const
{
    return m_views.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::addView( RimStatisticsContourMapView* view )
{
    // make sure to update the other views as the calculated data might have changed
    for ( auto view : m_views )
    {
        view->scheduleCreateDisplayModelAndRedraw();
    }
    m_views.push_back( view );
}
