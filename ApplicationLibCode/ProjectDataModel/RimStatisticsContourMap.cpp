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
#include "RiaStatisticsTools.h"

#include "RicNewStatisticsContourMapViewFeature.h"

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
#include "RimStatisticsContourMapView.h"
#include "RimTools.h"

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
    uiOrdering.add( nameField() );
    uiOrdering.add( &m_resultAggregation );

    {
        auto group = uiOrdering.addNewGroup( "Time Step Selection" );
        group->setCollapsedByDefault();
        group->add( &m_selectedTimeSteps );
    }

    uiOrdering.add( &m_relativeSampleSpacing );
    uiOrdering.add( &m_boundingBoxExpPercent );

    caf::PdmUiGroup* resultDefinitionGroup = uiOrdering.addNewGroup( "Result Definition" );
    m_resultDefinition->uiOrdering( uiConfigName, *resultDefinitionGroup );

    uiOrdering.add( &m_computeStatisticsButton );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_resultDefinition->setEclipseCase( eclipseCase );
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
        RicNewStatisticsContourMapViewFeature::createAndAddView( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimStatisticsContourMap::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    auto eCase = m_resultDefinition->eclipseCase();

    if ( eCase == nullptr ) return options;

    if ( &m_selectedTimeSteps == fieldNeedingOptions )
    {
        QList<caf::PdmOptionItemInfo> options;

        const auto timeStepStrings = eCase->timeStepStrings();

        int index = 0;
        for ( const auto& text : timeStepStrings )
        {
            options.push_back( caf::PdmOptionItemInfo( text, index++ ) );
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

    if ( ensemble->cases().empty() ) return;

    RimEclipseCase* eclipseCase = ensemble->cases().front();
    setEclipseCase( eclipseCase );
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

    RimEclipseCase* firstEclipseCase = ensemble->cases().front();
    firstEclipseCase->ensureReservoirCaseIsOpen();

    RigContourMapCalculator::ResultAggregationType resultAggregation = m_resultAggregation();

    cvf::BoundingBox gridBoundingBox = firstEclipseCase->activeCellsBoundingBox();
    gridBoundingBox.expandPercent( m_boundingBoxExpPercent() );

    auto computeSampleSpacing = []( auto ec, double relativeSampleSpacing )
    {
        if ( ec )
        {
            if ( auto mainGrid = ec->mainGrid() )
            {
                return relativeSampleSpacing * mainGrid->characteristicIJCellSize();
            }
        }
        return 0.0;
    };

    double sampleSpacing = computeSampleSpacing( firstEclipseCase, m_relativeSampleSpacing() );

    auto contourMapGrid = std::make_unique<RigContourMapGrid>( gridBoundingBox, sampleSpacing );

    auto firstEclipseCaseData = firstEclipseCase->eclipseCaseData();
    auto firstResultData      = firstEclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    RigEclipseContourMapProjection contourMapProjection( *contourMapGrid, *firstEclipseCaseData, *firstResultData );
    m_gridMapping = contourMapProjection.generateGridMapping( resultAggregation, {} );

    caf::ProgressInfo progInfo( ensemble->cases().size() + 1, "Reading Eclipse Ensemble" );

    // std::vector<std::vector<double>> results;

    std::map<size_t, std::vector<std::vector<double>>> timestep_results;

    for ( RimEclipseCase* eclipseCase : ensemble->cases() )
    {
        if ( eclipseCase->ensureReservoirCaseIsOpen() )
        {
            RiaLogging::info( QString( "Grid: %1" ).arg( eclipseCase->caseUserDescription() ) );

            auto eclipseCaseData = eclipseCase->eclipseCaseData();
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
        progInfo.incrementProgress();
    }

    m_contourMapGrid = std::move( contourMapGrid );
    m_timeResults.clear();

    for ( auto [timeStep, results] : timestep_results )
    {
        if ( !results.empty() )
        {
            int                 nCells = static_cast<int>( results[0].size() );
            std::vector<double> p10Results( nCells, std::numeric_limits<double>::infinity() );
            std::vector<double> p50Results( nCells, std::numeric_limits<double>::infinity() );
            std::vector<double> p90Results( nCells, std::numeric_limits<double>::infinity() );
            std::vector<double> meanResults( nCells, std::numeric_limits<double>::infinity() );
            std::vector<double> minResults( nCells, std::numeric_limits<double>::infinity() );
            std::vector<double> maxResults( nCells, std::numeric_limits<double>::infinity() );

#pragma omp parallel for
            for ( int i = 0; i < nCells; i++ )
            {
                size_t              numSamples = results.size();
                std::vector<double> samples( numSamples, 0.0 );
                for ( size_t s = 0; s < numSamples; s++ )
                    samples[s] = results[s][i];

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
                if ( RiaStatisticsTools::isValidNumber( minValue ) && minValue < std::numeric_limits<double>::max() )
                    minResults[i] = minValue;

                double maxValue = RiaStatisticsTools::maximumValue( samples );
                if ( RiaStatisticsTools::isValidNumber( maxValue ) && maxValue > -std::numeric_limits<double>::max() )
                    maxResults[i] = maxValue;
            }

            m_timeResults[timeStep][StatisticsType::P10]  = p10Results;
            m_timeResults[timeStep][StatisticsType::P50]  = p50Results;
            m_timeResults[timeStep][StatisticsType::P90]  = p90Results;
            m_timeResults[timeStep][StatisticsType::MEAN] = meanResults;
            m_timeResults[timeStep][StatisticsType::MIN]  = minResults;
            m_timeResults[timeStep][StatisticsType::MAX]  = maxResults;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStatisticsContourMap::eclipseCase() const
{
    auto ensemble = firstAncestorOrThisOfType<RimEclipseCaseEnsemble>();
    if ( !ensemble || ensemble->cases().empty() ) return nullptr;

    return ensemble->cases().front();
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
    if ( !m_timeResults.contains( timeStep ) ) return {};

    if ( !m_timeResults.at( timeStep ).contains( statisticsType ) ) return {};

    return m_timeResults.at( timeStep ).at( statisticsType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RimStatisticsContourMap::selectedTimeSteps() const
{
    if ( m_selectedTimeSteps().empty() )
    {
        std::vector<int> retVec;
        for ( int i = 0; i < (int)eclipseCase()->timeStepStrings().size(); i++ )
            retVec.push_back( i );
        return retVec;
    }
    return m_selectedTimeSteps();
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
    m_views.push_back( view );
}