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
#include "RigMainGrid.h"
#include "RigPolyLinesData.h"
#include "RigStatisticsMath.h"

#include "ContourMap/RimContourMapInViewCollection.h"
#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Rim3dView.h"
#include "RimCellFilter.h"
#include "RimCellFilterTools.h"
#include "RimDataFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimProject.h"
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
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUuid>

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Restrict the grid mapping to the cells accepted by the data filter, evaluated against the given
/// realization at the given (local) time step.
//--------------------------------------------------------------------------------------------------
void applyDataFilterVisibility( RigEclipseContourMapProjection& projection, RimCellFilter* dataFilter, RimEclipseCase* eCase, size_t timeStepIndex )
{
    if ( !dataFilter ) return;

    projection.setCellVisibility( RimCellFilterTools::computeReservoirCellVisibility( dataFilter, eCase, timeStepIndex ) );
}

//--------------------------------------------------------------------------------------------------
/// Generate one result per selected time step if the mapped property is dynamic or the active data
/// filter is dynamic (its visible cells then differ per time step, even for a static property).
/// Otherwise generate a single, time-independent result.
//--------------------------------------------------------------------------------------------------
void extractCaseResults( RigEclipseContourMapProjection&                     projection,
                         RimCellFilter*                                      dataFilter,
                         RimEclipseCase*                                     eCase,
                         const RigEclipseResultAddress&                      resultAddress,
                         bool                                                hasDynamicResult,
                         bool                                                hasDynamicFilter,
                         RigContourMapCalculator::ResultAggregationType      resultAggregation,
                         RigFloodingSettings&                                floodSettings,
                         const std::vector<std::vector<cvf::Vec3d>>&         selectedPolygons,
                         const std::vector<std::pair<int, int>>&             localToGlobalTimeSteps,
                         std::map<size_t, std::vector<std::vector<double>>>& timestepResults )
{
    if ( hasDynamicResult || hasDynamicFilter )
    {
        for ( auto [localTs, globalTs] : localToGlobalTimeSteps )
        {
            if ( hasDynamicFilter )
            {
                applyDataFilterVisibility( projection, dataFilter, eCase, static_cast<size_t>( localTs ) );
                projection.generateGridMapping( resultAggregation, {}, selectedPolygons );
            }

            const int resultTimeStep = hasDynamicResult ? localTs : 0;
            timestepResults[globalTs].push_back( projection.generateResults( resultAddress, resultAggregation, resultTimeStep, floodSettings ) );
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
    // Only a settings picker here; actual results are read per realization in computeStatisticsForMaps().
    m_resultDefinition->setEagerResultLoadingEnabled( false );

    CAF_PDM_InitFieldNoDefault( &m_primaryCase,
                                "PrimaryEclipseCase",
                                "Primary Case",
                                "",
                                "Eclipse Case used for wells and faults shown in views, initializing available result list, timesteps, "
                                "etc." );

    CAF_PDM_InitFieldNoDefault( &m_views, "ContourMapViews", "Contour Maps", ":/CrossSection16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_selectedPolygons, "Polygons", "Select Polygons" );
    m_selectedPolygons.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedPolygons.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );

    CAF_PDM_InitFieldNoDefault( &m_dataFilter, "DataFilter", "Data Filter", "", "Only cells accepted by the ensemble data filter are used." );

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

    CAF_PDM_InitFieldNoDefault( &m_cacheMapSize, "CacheMapSize", "Cache Map Size" );
    m_cacheMapSize.uiCapability()->setUiHidden( true );

    // Obsolete built-in formation filter, replaced by ensemble data filters (see #14710).
    CAF_PDM_InitField( &m_enableFormationFilter_OBSOLETE, "EnableFormationFilter", false, "Enable Formation Filter" );
    m_enableFormationFilter_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_enableFormationFilter_OBSOLETE.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_selectedFormations_OBSOLETE, "Formations", "Select Formations" );
    m_selectedFormations_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_selectedFormations_OBSOLETE.uiCapability()->setUiHidden( true );

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

    bool computeOK = !selectedTimeSteps().empty();

    uiOrdering.add( nameField() );

    {
        auto* btn = uiOrdering.addNewButton( "Compute", [this]() { onComputeStatisticsClicked(); } );
        btn->setUiToolTip( computeOK ? "Start statistics computations." : "Please check your time step selection." );
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

    if ( auto* gridEnsemble = firstAncestorOrThisOfType<RimReservoirGridEnsemble>() )
    {
        if ( m_dataFilter() || ( gridEnsemble->dataFilterCollection() && !gridEnsemble->dataFilterCollection()->filters().empty() ) )
        {
            auto dataFilterGrp = uiOrdering.addNewGroup( "Data Filter" );
            if ( !m_dataFilter() ) dataFilterGrp->setCollapsedByDefault();
            dataFilterGrp->add( &m_dataFilter );
        }
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
    else if ( &m_dataFilter == changedField )
    {
        // Refresh the filter label overlay immediately. The contour map data itself still requires
        // an explicit "Compute Statistics" to reflect the new filter selection.
        for ( auto& view : m_views )
        {
            view->updateFilterLabel();
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
    else if ( &m_dataFilter == fieldNeedingOptions )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        if ( auto* gridEnsemble = firstAncestorOrThisOfType<RimReservoirGridEnsemble>() )
        {
            if ( auto* dataFilterCollection = gridEnsemble->dataFilterCollection() )
            {
                for ( RimCellFilter* filter : dataFilterCollection->filters() )
                {
                    if ( !filter ) continue;
                    options.push_back( caf::PdmOptionItemInfo( filter->fullName(), filter, false, filter->uiIconProvider() ) );
                }
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
    // Formation filter removed in 2026.09.1 in favor of ensemble data filters (#14710).
    bool hasObsoleteFormationFilter = m_enableFormationFilter_OBSOLETE() || !m_selectedFormations_OBSOLETE().empty();
    if ( hasObsoleteFormationFilter && RimProject::current() && RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2026.09.1" ) )
    {
        QString formations = QStringList( m_selectedFormations_OBSOLETE().begin(), m_selectedFormations_OBSOLETE().end() ).join( ", " );
        QString message    = QString( "Ensemble contour map '%1' had a formation filter selection ('%2') from an older "
                                      "ResInsight version. The built-in formation filter has been removed and is no "
                                      "longer applied. Use an ensemble data filter on formation names instead." )
                              .arg( name() )
                              .arg( formations );
        RiaLogging::warning( message.toStdString() );
    }

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

    if ( auto ensemble = firstAncestorOrThisOfType<RimReservoirGridEnsemble>() )
    {
        ensemble->reloadMetaDataIfNeeded();
    }
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
        gridBoundingBox.expandPercent( map->m_boundingBoxExpPercent(), map->m_boundingBoxExpPercent() );

        double sampleSpacing = 1.0;
        if ( auto mainGrid = primaryCase->mainGrid() ) sampleSpacing = map->sampleSpacingFactor() * mainGrid->characteristicIJCellSize();

        ctx.contourMapGrid = std::make_unique<RigContourMapGrid>( gridBoundingBox, sampleSpacing );

        if ( ctx.active )
        {
            if ( ctx.useSharedGrid )
            {
                auto primaryCaseData   = primaryCase->eclipseCaseData();
                auto primaryResultData = primaryCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

                ctx.sharedProjection =
                    std::make_unique<RigEclipseContourMapProjection>( ctx.contourMapGrid.get(), primaryCaseData, primaryResultData );
                // With a data filter the mapping is generated per realization
                if ( !map->m_dataFilter() ) ctx.sharedProjection->generateGridMapping( ctx.resultAggregation, {}, map->selectedPolygons() );
            }
        }

        contexts.push_back( std::move( ctx ) );
    }

    const bool anyActive = std::any_of( contexts.begin(), contexts.end(), []( const MapContext& ctx ) { return ctx.active; } );

    if ( anyActive )
    {
        RiaLogging::info( std::format( "Computing statistics for {} ensemble contour map(s)", contexts.size() ) );

        // All maps belong to the same ensemble, so the realization cases are shared
        auto cases = contexts.front().map->ensembleCases();

        const size_t      nCases = cases.size();
        caf::ProgressInfo progInfo( nCases, QString( "Reading Eclipse Ensemble" ) );
        int               i = 1;

        // The key point of this loop is that each realization is opened once, contributes to all pending contour maps,
        // and is then closed again to release memory.
        for ( RimEclipseCase* eCase : cases )
        {
            auto task = progInfo.task( QString( "Processing Case %1 of %2" ).arg( i++ ).arg( nCases ) );

            bool closeCase = !eCase->isReservoirCaseOpen();

            RifReaderSettings oldSettings = eCase->readerSettings();
            eCase->setReaderSettings( readerSettings );

            if ( eCase->ensureReservoirCaseIsOpen() )
            {
                RiaLogging::info( std::format( "Processing Grid: {}", eCase->caseUserDescription() ) );

                auto eclipseCaseData = eCase->eclipseCaseData();
                auto activeCellInfo  = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
                auto resultData      = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

                // Make sure at least one dynamic result this case needs is loaded before asking for its time step
                // dates: allTimeStepDatesFromEclipseReader() and the loaded-result based fallback below both only
                // report the full time step count once such a result is known.
                for ( auto& ctx : contexts )
                {
                    if ( !ctx.active ) continue;
                    if ( ctx.map->m_resultDefinition()->hasDynamicResult() )
                        resultData->ensureKnownResultLoaded( ctx.map->m_resultDefinition()->eclipseResultAddress() );
                    if ( auto address = RimCellFilterTools::dynamicResultAddress( ctx.map->m_dataFilter() ) )
                        resultData->ensureKnownResultLoaded( *address );
                }

                // The case's own time step dates, read directly from the reader rather than from already-loaded
                // results: a realization case may not have any dynamic result loaded yet, in which case
                // eCase->timeStepDates() would incorrectly report only a single (or zero) time step.
                auto caseTimeStepDates = resultData->allTimeStepDatesFromEclipseReader( eCase->gridFileName() );
                if ( caseTimeStepDates.empty() ) caseTimeStepDates = eCase->timeStepDates();

                // Prefetch every dynamic result time step this case needs before reading any of it for real.
                // This case has no view of its own, so it is the only chance a cloud-backed reader gets to
                // fetch exactly the time steps needed before the case is closed again - see
                // RimEclipseCase::prefetchDynamicResult. A no-op for a case that reads from disk. Grouped by
                // result name so every time step a property needs is requested together as one parallel
                // batch (rather than one blocking round trip per time step); prefetchDynamicResult() itself
                // blocks until that whole batch has arrived before returning, so nothing it requested is
                // left in flight when this case is closed again below - no separate wait is needed here.
                std::map<QString, std::vector<size_t>> stepsToPrefetchByResult;
                for ( auto& ctx : contexts )
                {
                    if ( !ctx.active ) continue;

                    auto localToGlobalTimeSteps = ctx.map->mapLocalToGlobalTimeSteps( caseTimeStepDates );

                    if ( ctx.map->m_resultDefinition()->hasDynamicResult() )
                    {
                        auto& steps = stepsToPrefetchByResult[ctx.map->m_resultDefinition()->eclipseResultAddress().resultName()];
                        for ( auto [localTs, globalTs] : localToGlobalTimeSteps )
                            steps.push_back( static_cast<size_t>( localTs ) );
                    }
                }

                for ( auto& [resultName, steps] : stepsToPrefetchByResult )
                {
                    std::sort( steps.begin(), steps.end() );
                    steps.erase( std::unique( steps.begin(), steps.end() ), steps.end() );

                    eCase->prefetchDynamicResult( resultName, steps );
                }

                for ( auto& ctx : contexts )
                {
                    if ( !ctx.active ) continue;

                    RimStatisticsContourMap* map                    = ctx.map;
                    auto                     localToGlobalTimeSteps = map->mapLocalToGlobalTimeSteps( caseTimeStepDates );

                    if ( ctx.useSharedGrid )
                    {
                        ctx.sharedProjection->updateRealizationData( activeCellInfo, resultData );

                        // A data filter can accept different cells in each realization, e.g. a property filter or
                        // formation names defined per realization. A dynamic filter's visible cells also change per
                        // time step, so its mapping is (re)generated per time step inside extractCaseResults() instead.
                        const bool filterIsDynamic = RimCellFilterTools::isDynamicFilter( map->m_dataFilter() );
                        if ( map->m_dataFilter() && !filterIsDynamic )
                        {
                            applyDataFilterVisibility( *ctx.sharedProjection, map->m_dataFilter(), eCase, 0 );
                            ctx.sharedProjection->generateGridMapping( ctx.resultAggregation, {}, map->selectedPolygons() );
                        }

                        extractCaseResults( *ctx.sharedProjection,
                                            map->m_dataFilter(),
                                            eCase,
                                            map->m_resultDefinition()->eclipseResultAddress(),
                                            map->m_resultDefinition()->hasDynamicResult(),
                                            filterIsDynamic,
                                            ctx.resultAggregation,
                                            ctx.floodSettings,
                                            map->selectedPolygons(),
                                            localToGlobalTimeSteps,
                                            ctx.timestepResults );
                    }
                    else
                    {
                        RigEclipseContourMapProjection contourMapProjection( ctx.contourMapGrid.get(), eclipseCaseData, resultData );
                        const bool                     filterIsDynamic = RimCellFilterTools::isDynamicFilter( map->m_dataFilter() );
                        if ( !filterIsDynamic )
                        {
                            applyDataFilterVisibility( contourMapProjection, map->m_dataFilter(), eCase, 0 );
                            contourMapProjection.generateGridMapping( ctx.resultAggregation, {}, map->selectedPolygons() );
                        }
                        extractCaseResults( contourMapProjection,
                                            map->m_dataFilter(),
                                            eCase,
                                            map->m_resultDefinition()->eclipseResultAddress(),
                                            map->m_resultDefinition()->hasDynamicResult(),
                                            filterIsDynamic,
                                            ctx.resultAggregation,
                                            ctx.floodSettings,
                                            map->selectedPolygons(),
                                            localToGlobalTimeSteps,
                                            ctx.timestepResults );
                    }
                }
            }

            eCase->setReaderSettings( oldSettings );

            // Release the grid data for cases that were opened only to compute statistics. A case is kept open if it has
            // its own views, if it is the primary case of one of the contour maps, or if it is displayed in one of the
            // ensemble views.
            if ( closeCase )
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
RimCellFilter* RimStatisticsContourMap::dataFilter() const
{
    return m_dataFilter();
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
/// A single, time-independent result is enough when both the mapped property and the active data
/// filter are static. Otherwise the result (or the filter's visible cells) can differ per time step,
/// so fall back to the user-selected time steps.
//--------------------------------------------------------------------------------------------------
std::vector<int> RimStatisticsContourMap::selectedTimeSteps() const
{
    if ( !m_resultDefinition->hasDynamicResult() && !RimCellFilterTools::isDynamicFilter( m_dataFilter() ) )
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
        // Read time steps directly from the reader rather than from already-loaded results: the primary case may
        // not have any dynamic result loaded yet, in which case eCase->timeStepDates() would under-report them.
        auto* resultData = eCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        auto  allDates   = resultData ? resultData->allTimeStepDatesFromEclipseReader( eCase->gridFileName() ) : std::vector<QDateTime>();
        if ( allDates.empty() ) allDates = eCase->timeStepDates();

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

    // contour map calculations on shared grids clears the ensemble meta data, reload it
    if ( auto ensemble = firstAncestorOrThisOfType<RimReservoirGridEnsemble>() )
    {
        ensemble->reloadMetaDataIfNeeded();
    }
}

//--------------------------------------------------------------------------------------------------
/// Hash of all settings and input grid files that affect the computed statistics. Cached results are only reused when
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

    // Include the filter configuration, so that editing the filter invalidates the cache
    if ( m_dataFilter() ) parts << m_dataFilter()->writeObjectToXmlString();

    for ( const auto& polygonLine : selectedPolygons() )
    {
        for ( const auto& point : polygonLine )
            parts << QString( "%1,%2,%3" ).arg( point.x(), 0, 'f', 3 ).arg( point.y(), 0, 'f', 3 ).arg( point.z(), 0, 'f', 3 );
    }

    // Identify a grid file by path, size and modification time, so that the cache is invalidated both when the
    // ensemble is replaced or its cases are added/removed, and when a grid file is overwritten in place
    auto gridFileFingerprint = []( const RimEclipseCase* gridCase ) -> QString
    {
        const QString   gridFileName = gridCase->gridFileName();
        const QFileInfo fileInfo( gridFileName );
        if ( !fileInfo.exists() ) return gridFileName;

        return QString( "%1,%2,%3" ).arg( gridFileName ).arg( fileInfo.size() ).arg( fileInfo.lastModified().toMSecsSinceEpoch() );
    };

    // Label and count the two sets of cases, so that a primary case can not be mistaken for the first ensemble case
    parts << "primaryCase" << ( eclipseCase() != nullptr ? gridFileFingerprint( eclipseCase() ) : QString() );

    const std::vector<RimEclipseCase*> cases = ensembleCases();
    parts << "ensembleCases" << QString::number( cases.size() );
    for ( const RimEclipseCase* eCase : cases )
        parts << gridFileFingerprint( eCase );

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

    // The map size is stored explicitly, as recomputing it from the extent of the expanded bounding box is sensitive to
    // the limited precision of the doubles in the project file
    if ( m_cacheMapSize().size() != 2 || m_cacheMapSize()[0] <= 0 || m_cacheMapSize()[1] <= 0 ) return false;

    const QString expectedKey = computeCacheValidityKey();
    if ( m_cacheValidityKey().isEmpty() || m_cacheValidityKey() != expectedKey ) return false;

    auto toBoundingBox = []( const std::vector<double>& c )
    { return cvf::BoundingBox( cvf::Vec3d( c[0], c[1], c[2] ), cvf::Vec3d( c[3], c[4], c[5] ) ); };

    const cvf::BoundingBox originalBoundingBox = toBoundingBox( m_cacheOriginalBoundingBox() );
    const cvf::BoundingBox expandedBoundingBox = toBoundingBox( m_cacheExpandedBoundingBox() );

    const cvf::Vec2ui storedMapSize( static_cast<cvf::uint>( m_cacheMapSize()[0] ), static_cast<cvf::uint>( m_cacheMapSize()[1] ) );

    auto contourMapGrid =
        std::make_unique<RigContourMapGrid>( originalBoundingBox, expandedBoundingBox, m_cacheSampleSpacing(), storedMapSize );

    const cvf::Vec2ui& mapSize = contourMapGrid->mapSize();

    std::map<size_t, std::map<StatisticsType, std::vector<double>>> timeResults;
    for ( int timeStep : m_cacheTimeSteps() )
    {
        for ( size_t statisticsTypeIndex = 0; statisticsTypeIndex < caf::AppEnum<StatisticsType>::size(); ++statisticsTypeIndex )
        {
            const StatisticsType statisticsType = caf::AppEnum<StatisticsType>::fromIndex( statisticsTypeIndex );

            const QString fileName = cacheFileName( m_cacheFileBaseName(), statisticsType, timeStep );

            // The surface reader throws on malformed files, fall back to recomputation
            std::expected<std::pair<RigRegularSurfaceData, std::vector<float>>, std::string> surfaceData;
            try
            {
                surfaceData = RifSurfio::importSurfaceData( fileName.toStdString() );
            }
            catch ( ... )
            {
                surfaceData = std::unexpected( "Unexpected file content" );
            }

            if ( !surfaceData.has_value() )
            {
                RiaLogging::warning( std::format( "Failed to read ensemble contour map statistics cache, recomputing: {}", fileName ) );
                return false;
            }

            const auto& [regularSurface, values] = surfaceData.value();
            if ( regularSurface.nx != static_cast<int>( mapSize.x() ) || regularSurface.ny != static_cast<int>( mapSize.y() ) ||
                 values.size() != static_cast<size_t>( mapSize.x() ) * static_cast<size_t>( mapSize.y() ) )
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
        m_cacheMapSize =
            std::vector<int>{ static_cast<int>( m_contourMapGrid->mapSize().x() ), static_cast<int>( m_contourMapGrid->mapSize().y() ) };
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
    m_cacheMapSize             = std::vector<int>();
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

    // The 3d views mirror every contour map of the project, these included
    RimContourMapInViewCollection::updateViewTreeItemsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    if ( childArray != &m_views ) return;

    RimContourMapInViewCollection::updateViewTreeItemsInAllViews();

    // A deleted contour map may have been visible in a 3d view
    if ( RimProject* project = RimProject::current() )
    {
        for ( Rim3dView* view : project->allViews() )
        {
            if ( view ) view->scheduleCreateDisplayModelAndRedraw();
        }
    }
}
