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

#include "RigCaseCellResultsData.h"
#include "RigContourMapCalculator.h"
#include "RigContourMapGrid.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseContourMapProjection.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigStatisticsMath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseResultDefinition.h"
#include "RimProject.h"
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
{
    CAF_PDM_InitObject( "Statistics Contour Map", ":/Histogram16x16.png" );

    CAF_PDM_InitField( &m_boundingBoxExpPercent, "BoundingBoxExpPercent", 5.0, "Bounding Box Expansion (%)" );
    m_boundingBoxExpPercent.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_relativeSampleSpacing, "SampleSpacing", 0.9, "Sample Spacing Factor" );
    m_relativeSampleSpacing.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_resultAggregation, "ResultAggregation", "Result Aggregation" );

    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "SelectedTimeSteps", "Time Step Selection" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_resultDefinition, "ResultDefinition", "" );
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_resultDefinition = new RimEclipseResultDefinition;
    m_resultDefinition->findField( "MResultType" )->uiCapability()->setUiName( "Result" );
    m_resultDefinition->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
    m_resultDefinition->setResultVariable( "SOIL" );

    CAF_PDM_InitField( &m_uiDataSourceCase, "UiDataSourceCase", RiaResultNames::undefinedResultName(), "UI Data Source Case" );

    CAF_PDM_InitFieldNoDefault( &m_computeStatisticsButton, "ComputeStatisticsButton", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_computeStatisticsButton );
    m_computeStatisticsButton = false;

    CAF_PDM_InitFieldNoDefault( &m_views, "ContourMapViews", "Contour Maps", ":/CrossSection16x16.png" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( ( eclipseCase() == nullptr ) && ( ensemble()->cases().size() > 0 ) )
    {
        auto selCase = ensemble()->cases().front();
        setEclipseCase( selCase );
        m_uiDataSourceCase = selCase->caseUserDescription();
    }

    uiOrdering.add( nameField() );
    uiOrdering.add( &m_resultAggregation );
    uiOrdering.add( &m_uiDataSourceCase );

    auto tsGroup = uiOrdering.addNewGroup( "Time Step Selection" );
    tsGroup->setCollapsedByDefault();
    tsGroup->add( &m_selectedTimeSteps );

    uiOrdering.add( &m_relativeSampleSpacing );
    uiOrdering.add( &m_boundingBoxExpPercent );

    if ( !isColumnResult() )
    {
        auto resultDefinitionGroup = uiOrdering.addNewGroup( "Result Definition" );
        m_resultDefinition->uiOrdering( uiConfigName, *resultDefinitionGroup );
    }

    uiOrdering.add( &m_computeStatisticsButton );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_resultDefinition->setEclipseCase( eclipseCase );

    for ( auto& view : m_views )
    {
        view->setEclipseCase( eclipseCase );
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
    else if ( &m_uiDataSourceCase == changedField )
    {
        switchToSelectedSourceCase();

        // Update views
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
RimEclipseCase* RimStatisticsContourMap::switchToSelectedSourceCase()
{
    RimEclipseCase* sourceResultCase = ensemble()->findByDescription( m_uiDataSourceCase );
    if ( sourceResultCase == nullptr )
    {
        setEclipseCase( nullptr );
        return nullptr;
    }

    if ( sourceResultCase != eclipseCase() )
    {
        auto oldCase = eclipseCase();
        sourceResultCase->ensureReservoirCaseIsOpen();
        setEclipseCase( sourceResultCase );

        if ( oldCase && !ensemble()->casesInViews().contains( oldCase ) )
        {
            oldCase->closeReservoirCase();
        }
    }

    return sourceResultCase;
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
    else if ( &m_uiDataSourceCase == fieldNeedingOptions )
    {
        QStringList sourceCaseNames;
        sourceCaseNames += RiaResultNames::undefinedResultName();

        for ( auto eCase : ensemble()->cases() )
        {
            options.push_back( caf::PdmOptionItemInfo( eCase->caseUserDescription(), eCase->caseUserDescription() ) );
        }
        return options;
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
    else if ( &m_relativeSampleSpacing == field )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum                       = 0.2;
            myAttr->m_maximum                       = 2.0;
            myAttr->m_sliderTickCount               = 9;
            myAttr->m_delaySliderUpdateUntilRelease = true;
        }
    }
    else if ( &m_boundingBoxExpPercent == field )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum                       = 0.0;
            myAttr->m_maximum                       = 25.0;
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

    for ( auto view : m_views )
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
    RiaLogging::info( "Computing statistics" );
    auto ensemble = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
    if ( !ensemble ) return;

    if ( ensemble->cases().empty() ) return;
    if ( eclipseCase() == nullptr ) return;

    RigContourMapCalculator::ResultAggregationType resultAggregation = m_resultAggregation();

    cvf::BoundingBox gridBoundingBox = eclipseCase()->activeCellsBoundingBox();
    gridBoundingBox.expandPercent( m_boundingBoxExpPercent() );

    double sampleSpacing = 1.0;
    if ( auto mainGrid = eclipseCase()->mainGrid() )
    {
        sampleSpacing = m_relativeSampleSpacing * mainGrid->characteristicIJCellSize();
    }

    auto contourMapGrid = std::make_unique<RigContourMapGrid>( gridBoundingBox, sampleSpacing );

    auto firstEclipseCaseData = eclipseCase()->eclipseCaseData();
    auto firstResultData      = firstEclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    RigEclipseContourMapProjection contourMapProjection( *contourMapGrid, *firstEclipseCaseData, *firstResultData );
    m_gridMapping = contourMapProjection.generateGridMapping( resultAggregation, {} );

    const size_t nCases = ensemble->cases().size();

    std::map<size_t, std::vector<std::vector<double>>> timestep_results;

    caf::ProgressInfo progInfo( nCases, QString( "Reading Eclipse Ensemble" ) );

    auto readerSettings = RiaPreferencesGrid::current()->gridOnlyReaderSettings();
    auto casesInViews   = ensemble->casesInViews();

    int i = 1;
    for ( RimEclipseCase* eCase : ensemble->cases() )
    {
        auto task = progInfo.task( QString( "Processing Case %1 of %2" ).arg( i++ ).arg( nCases ) );

        RifReaderSettings oldSettings = eCase->readerSettings();
        eCase->setReaderSettings( readerSettings );

        if ( eCase->ensureReservoirCaseIsOpen() )
        {
            RiaLogging::info( QString( "Grid: %1" ).arg( eCase->caseUserDescription() ) );

            auto eclipseCaseData = eCase->eclipseCaseData();
            auto resultData      = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

            RigEclipseContourMapProjection contourMapProjection( *contourMapGrid, *eclipseCaseData, *resultData );
            contourMapProjection.generateGridMapping( resultAggregation, {} );

            if ( m_resultDefinition()->hasDynamicResult() )
            {
                for ( auto ts : selectedTimeSteps() )
                {
                    std::vector<double> result =
                        contourMapProjection.generateResults( m_resultDefinition()->eclipseResultAddress(), resultAggregation, ts );
                    timestep_results[ts].push_back( result );
                }
            }
            else
            {
                std::vector<double> result =
                    contourMapProjection.generateResults( m_resultDefinition()->eclipseResultAddress(), resultAggregation, 0 );
                timestep_results[0].push_back( result );
            }
        }
        eCase->setReaderSettings( oldSettings );

        if ( eCase->views().empty() && eCase != eclipseCase() && !casesInViews.contains( eCase ) )
        {
            eCase->closeReservoirCase();
        }
    }

    m_contourMapGrid = std::move( contourMapGrid );

    doStatisticsCalculation( timestep_results );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStatisticsContourMap::eclipseCase() const
{
    if ( !m_resultDefinition() ) return nullptr;

    return m_resultDefinition->eclipseCase();
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
    auto steps = m_selectedTimeSteps();
    if ( m_selectedTimeSteps().empty() )
    {
        for ( int i = 0; i < (int)eclipseCase()->timeStepStrings().size(); i++ )
        {
            steps.push_back( i );
        }
    }
    else
    {
        std::sort( steps.begin(), steps.end() );
    }
    return steps;
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
    return m_relativeSampleSpacing;
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
