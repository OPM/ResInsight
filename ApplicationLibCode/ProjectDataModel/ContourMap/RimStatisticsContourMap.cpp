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
#include "RiaStatisticsTools.h"

#include "RicNewStatisticsContourMapViewFeature.h"

#include "RifReaderSettings.h"

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
#include "RimEclipseResultDefinition.h"
#include "RimSimWellInViewCollection.h"
#include "RimStatisticsContourMapProjection.h"
#include "RimStatisticsContourMapView.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafProgressInfo.h"

#include <limits>
#include <set>

CAF_PDM_SOURCE_INIT( RimStatisticsContourMap, "RimStatisticalContourMap" );

namespace caf
{
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

    CAF_PDM_InitFieldNoDefault( &m_computeStatisticsButton, "ComputeStatisticsButton", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_computeStatisticsButton );
    m_computeStatisticsButton = false;

    CAF_PDM_InitFieldNoDefault( &m_views, "ContourMapViews", "Contour Maps", ":/CrossSection16x16.png" );

    CAF_PDM_InitField( &m_enableFormationFilter, "EnableFormationFilter", false, "Enable Formation Filter" );
    CAF_PDM_InitFieldNoDefault( &m_selectedFormations, "Formations", "Select Formations" );
    m_selectedFormations.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedFormations.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_selectedPolygons, "Polygons", "Select Polygons" );
    m_selectedPolygons.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedPolygons.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( ( eclipseCase() == nullptr ) && ( !ensemble()->cases().empty() ) )
    {
        auto selCase = ensemble()->cases().front();
        setEclipseCase( selCase );
    }

    bool computeOK = !( m_enableFormationFilter && m_selectedFormations().empty() );
    computeOK      = computeOK && !selectedTimeSteps().empty();

    uiOrdering.add( nameField() );
    uiOrdering.add( &m_computeStatisticsButton );
    m_computeStatisticsButton.uiCapability()->setUiReadOnly( !computeOK );
    if ( computeOK )
    {
        m_computeStatisticsButton.uiCapability()->setUiToolTip( "Start statistics computations." );
    }
    else
    {
        m_computeStatisticsButton.uiCapability()->setUiToolTip( "Please check your time step and/or formation filter selections." );
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
    genGrp->add( &m_primaryCase );
    genGrp->add( &m_boundingBoxExpPercent );

    auto tsGroup = uiOrdering.addNewGroup( "Time Step Selection" );
    tsGroup->setCollapsedByDefault();
    tsGroup->add( &m_selectedTimeSteps );

    if ( eclipseCase() && eclipseCase()->activeFormationNames() )
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
RimEclipseCaseEnsemble* RimStatisticsContourMap::ensemble() const
{
    return firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &m_computeStatisticsButton == changedField )
    {
        computeStatistics();
        m_computeStatisticsButton = false;

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
    else if ( &m_primaryCase == changedField )
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

        if ( m_openEclipseCase && !ensemble()->casesInViews().contains( m_openEclipseCase ) )
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
        for ( auto eCase : ensemble()->cases() )
        {
            options.push_back( caf::PdmOptionItemInfo( eCase->caseUserDescription(), eCase, false, eCase->uiIconProvider() ) );
        }
        return options;
    }
    else if ( &m_selectedFormations == fieldNeedingOptions )
    {
        if ( auto eCase = eclipseCase() )
        {
            if ( auto formations = eCase->activeFormationNames() )
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
    if ( &m_computeStatisticsButton == field )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Compute";
        }
    }
    else if ( ( &m_userDefinedFloodingOil == field ) || ( &m_userDefinedFloodingGas == field ) )
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
    auto ensemble = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
    if ( !ensemble ) return;

    switchToSelectedSourceCase();

    for ( auto view : m_views.childrenByType() )
    {
        view->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::doStatisticsCalculation( std::map<size_t, std::vector<std::vector<double>>>& timestepResults )
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

            if ( RiaStatisticsTools::isValidNumber( p10 ) ) p10Results[i] = p10;
            if ( RiaStatisticsTools::isValidNumber( p50 ) ) p50Results[i] = p50;
            if ( RiaStatisticsTools::isValidNumber( p90 ) ) p90Results[i] = p90;
            if ( RiaStatisticsTools::isValidNumber( mean ) ) meanResults[i] = mean;

            double minValue = RiaStatisticsTools::minimumValue( samples );
            if ( RiaStatisticsTools::isValidNumber( minValue ) && minValue < std::numeric_limits<double>::max() ) minResults[i] = minValue;

            double maxValue = RiaStatisticsTools::maximumValue( samples );
            if ( RiaStatisticsTools::isValidNumber( maxValue ) && maxValue > -std::numeric_limits<double>::max() ) maxResults[i] = maxValue;
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
void RimStatisticsContourMap::computeStatistics()
{
    if ( m_computeStatisticsButton.isReadOnly() ) return;

    RiaLogging::info( "Computing statistics" );
    auto ensemble = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
    if ( !ensemble ) return;

    if ( ensemble->cases().empty() ) return;
    if ( eclipseCase() == nullptr ) return;

    RigFloodingSettings floodSettings( m_oilFloodingType(), m_userDefinedFloodingOil(), m_gasFloodingType(), m_userDefinedFloodingGas() );
    RigContourMapCalculator::ResultAggregationType resultAggregation = m_resultAggregation();

    cvf::BoundingBox gridBoundingBox = eclipseCase()->activeCellsBoundingBox();
    gridBoundingBox.expandPercent( m_boundingBoxExpPercent() );

    double sampleSpacing = 1.0;
    if ( auto mainGrid = eclipseCase()->mainGrid() )
    {
        sampleSpacing = sampleSpacingFactor() * mainGrid->characteristicIJCellSize();
    }

    auto contourMapGrid = std::make_unique<RigContourMapGrid>( gridBoundingBox, sampleSpacing );

    const size_t nCases = ensemble->cases().size();

    std::map<size_t, std::vector<std::vector<double>>> timestep_results;

    caf::ProgressInfo progInfo( nCases, QString( "Reading Eclipse Ensemble" ) );

    auto readerSettings                = RiaPreferencesGrid::current()->gridOnlyReaderSettings();
    readerSettings.onlyLoadActiveCells = true;
    auto casesInViews                  = ensemble->casesInViews();
    auto oldReaderType                 = RiaPreferencesGrid::current()->gridModelReaderOverride();
    RiaPreferencesGrid::current()->setGridModelReaderOverride( RiaDefines::GridModelReader::OPM_COMMON );

    int i = 1;
    for ( RimEclipseCase* eCase : ensemble->cases() )
    {
        auto task = progInfo.task( QString( "Processing Case %1 of %2" ).arg( i++ ).arg( nCases ) );

        RifReaderSettings oldSettings = eCase->readerSettings();
        eCase->setReaderSettings( readerSettings );

        if ( eCase->ensureReservoirCaseIsOpen() )
        {
            RiaLogging::info( QString( "Processing Grid: %1" ).arg( eCase->caseUserDescription() ) );

            auto eclipseCaseData = eCase->eclipseCaseData();
            auto resultData      = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

            RigEclipseContourMapProjection contourMapProjection( *contourMapGrid, *eclipseCaseData, *resultData );

            std::set<int> usedKLayers;
            auto          formationNames = selectedFormations();

            bool formationNamesOk = true;
            if ( !formationNames.empty() )
            {
                if ( auto names = eCase->activeFormationNames() )
                {
                    if ( auto fData = names->formationNamesData() )
                    {
                        usedKLayers = fData->findKLayers( formationNames );
                    }
                    else
                    {
                        formationNamesOk = false;
                    }
                }
                else
                {
                    formationNamesOk = false;
                }
            }

            if ( formationNamesOk )
            {
                contourMapProjection.generateGridMapping( resultAggregation, {}, usedKLayers, selectedPolygons() );

                if ( m_resultDefinition()->hasDynamicResult() )
                {
                    std::vector<std::pair<int, int>> timeSteps = mapLocalToGlobalTimeSteps( eCase->timeStepDates() );

                    for ( auto [localTs, globalTs] : timeSteps )
                    {
                        std::vector<double> result = contourMapProjection.generateResults( m_resultDefinition()->eclipseResultAddress(),
                                                                                           resultAggregation,
                                                                                           localTs,
                                                                                           floodSettings );
                        timestep_results[globalTs].push_back( result );
                    }
                }
                else
                {
                    std::vector<double> result =
                        contourMapProjection.generateResults( m_resultDefinition()->eclipseResultAddress(), resultAggregation, 0, floodSettings );
                    timestep_results[0].push_back( result );
                }
            }
            else
            {
                RiaLogging::warning( QString( "Formation names are missing for case %1, skipping case." ).arg( eCase->caseUserDescription() ) );
            }
        }
        eCase->setReaderSettings( oldSettings );

        if ( eCase->views().empty() && eCase != eclipseCase() && !casesInViews.contains( eCase ) )
        {
            eCase->closeReservoirCase();
        }
    }

    RiaPreferencesGrid::current()->setGridModelReaderOverride( oldReaderType );

    m_contourMapGrid = std::move( contourMapGrid );

    doStatisticsCalculation( timestep_results );
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
std::vector<std::vector<cvf::Vec3d>> RimStatisticsContourMap::selectedPolygons()
{
    std::vector<std::vector<cvf::Vec3d>> allLines;

    for ( auto p : m_selectedPolygons() )
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
    if ( !m_contourMapGrid ) computeStatistics();
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
