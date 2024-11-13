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

#include "RigCaseCellResultsData.h"
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

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafProgressInfo.h"

#include <limits>

CAF_PDM_SOURCE_INIT( RimStatisticsContourMap, "RimStatisticalContourMap" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStatisticsContourMap::RimStatisticsContourMap()
{
    CAF_PDM_InitObject( "StatisticsContourMap", ":/Histogram16x16.png" );

    CAF_PDM_InitField( &m_relativeSampleSpacing, "SampleSpacing", 0.9, "Sample Spacing Factor" );
    m_relativeSampleSpacing.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_resultDefinition, "ResultDefinition", "" );
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_resultDefinition = new RimEclipseResultDefinition;
    m_resultDefinition->findField( "MResultType" )->uiCapability()->setUiName( "Result" );
    m_resultDefinition->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
    m_resultDefinition->setResultVariable( "SOIL" );

    CAF_PDM_InitFieldNoDefault( &m_computeStatisticsButton, "ComputeStatisticsButton", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_computeStatisticsButton );
    m_computeStatisticsButton = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStatisticsContourMap::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_relativeSampleSpacing );

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
void RimStatisticsContourMap::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &m_computeStatisticsButton == changedField )
    {
        computeStatistics();
        m_computeStatisticsButton = false;
    }
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

    if ( &m_relativeSampleSpacing == field )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum                       = 0.2;
            myAttr->m_maximum                       = 2.0;
            myAttr->m_sliderTickCount               = 9;
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

    int                                            timeStep          = 0;
    RigContourMapCalculator::ResultAggregationType resultAggregation = RigContourMapCalculator::ResultAggregationType::MEAN;

    cvf::BoundingBox gridBoundingBox = firstEclipseCase->activeCellsBoundingBox();

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

    std::vector<std::vector<double>> results;
    for ( RimEclipseCase* eclipseCase : ensemble->cases() )
    {
        if ( eclipseCase->ensureReservoirCaseIsOpen() )
        {
            RiaLogging::info( QString( "Grid: %1" ).arg( eclipseCase->caseUserDescription() ) );

            auto eclipseCaseData = eclipseCase->eclipseCaseData();
            auto resultData      = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

            RigEclipseContourMapProjection contourMapProjection( *contourMapGrid, *eclipseCaseData, *resultData );
            contourMapProjection.generateGridMapping( resultAggregation, {} );

            std::vector<double> result =
                contourMapProjection.generateResults( m_resultDefinition()->eclipseResultAddress(), resultAggregation, timeStep );
            results.push_back( result );
        }
    }

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

            double p10, p50, p90, mean;

            RigStatisticsMath::calculateStatisticsCurves( samples, &p10, &p50, &p90, &mean, RigStatisticsMath::PercentileStyle::SWITCHED );

            p10Results[i]  = p10;
            p50Results[i]  = p50;
            p90Results[i]  = p90;
            meanResults[i] = mean;

            minResults[i] = RiaStatisticsTools::minimumValue( samples );
            maxResults[i] = RiaStatisticsTools::maximumValue( samples );
        }
    }
}
