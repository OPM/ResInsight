/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Equinor ASA
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

#include "RimEclipseContourMapProjection.h"

#include "ContourMap/RigContourMapCalculator.h"
#include "ContourMap/RigContourMapGrid.h"
#include "ContourMap/RigEclipseContourMapProjection.h"
#include "RiaPorosityModel.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimEclipseContourMapProjection, "RimEclipseContourMapProjection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapProjection::RimEclipseContourMapProjection()
    : RimContourMapProjection()
{
    CAF_PDM_InitObject( "RimEclipseContourMapProjection", ":/2DMapProjection16x16.png" );

    CAF_PDM_InitField( &m_weightByParameter, "WeightByParameter", false, "Weight by Result Parameter" );
    CAF_PDM_InitFieldNoDefault( &m_weightingResult, "WeightingResult", "" );
    m_weightingResult.uiCapability()->setUiTreeChildrenHidden( true );
    m_weightingResult = new RimEclipseResultDefinition;
    m_weightingResult->findField( "MResultType" )->uiCapability()->setUiName( "Result Type" );

    setName( "Map Projection" );
    nameField()->uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapProjection::~RimEclipseContourMapProjection()
{
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseContourMapProjection::resultDescriptionText() const
{
    QString resultText = resultAggregationText();
    if ( !isColumnResult() )
    {
        resultText += QString( ", %1" ).arg( view()->cellResult()->resultVariable() );
    }

    return resultText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseContourMapProjection::resultVariableName() const
{
    if ( !isColumnResult() ) return view()->cellResult()->resultVariable();
    return resultAggregationText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseContourMapProjection::weightingParameter() const
{
    QString parameter = "None";
    if ( m_weightByParameter() && !m_weightingResult->isTernarySaturationSelected() )
    {
        parameter = m_weightingResult->resultVariableUiShortName();
    }
    return parameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimEclipseContourMapProjection::legendConfig() const
{
    return view()->cellResult()->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::updateLegend()
{
    RimEclipseCellColors* cellColors = view()->cellResult();

    auto [minValAllTimeSteps, maxValAllTimeSteps] = minmaxValuesAllTimeSteps();

    double minVal = m_contourMapProjection ? m_contourMapProjection->minValue() : std::numeric_limits<double>::infinity();
    double maxVal = m_contourMapProjection ? m_contourMapProjection->maxValue() : -std::numeric_limits<double>::infinity();

    legendConfig()->setAutomaticRanges( minValAllTimeSteps, maxValAllTimeSteps, minVal, maxVal );

    if ( isColumnResult() )
    {
        legendConfig()->setTitle( QString( "Map Projection\n%1" ).arg( m_resultAggregation().uiText() ) );
    }
    else
    {
        QString projectionLegendText = QString( "Map Projection\n%1" ).arg( m_resultAggregation().uiText() );
        if ( weightingParameter() != "None" )
        {
            projectionLegendText += QString( "(W: %1)" ).arg( weightingParameter() );
        }
        projectionLegendText += QString( "\nResult: %1" ).arg( cellColors->resultVariableUiShortName() );

        legendConfig()->setTitle( projectionLegendText );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseContourMapProjection::sampleSpacing() const
{
    if ( auto ec = eclipseCase() )
    {
        if ( auto mainGrid = ec->mainGrid() )
        {
            return sampleSpacingFactor() * mainGrid->characteristicIJCellSize();
        }
    }

    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::clearGridMappingAndRedraw()
{
    clearGridMapping();
    updateConnectedEditors();
    generateResultsIfNecessary( view()->currentTimeStep() );
    updateLegend();

    RimEclipseView* parentView = firstAncestorOrThisOfTypeAsserted<RimEclipseView>();
    parentView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimEclipseContourMapProjection::generateResults( int timeStep ) const
{
    m_weightingResult->loadResult();

    if ( m_contourMapProjection )
    {
        RimEclipseCellColors*   cellColors = view()->cellResult();
        RigEclipseResultAddress resAddr( cellColors->resultType(),
                                         cellColors->resultVariable(),
                                         cellColors->timeLapseBaseTimeStep(),
                                         cellColors->caseDiffIndex() );

        RigFloodingSettings fl( m_oilFloodingType(), m_userDefinedFloodingOil(), m_gasFloodingType(), m_userDefinedFloodingGas() );

        return dynamic_cast<RigEclipseContourMapProjection*>( m_contourMapProjection.get() )
            ->generateResults( resAddr, m_resultAggregation(), timeStep, fl );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::generateAndSaveResults( int timeStep )
{
    m_weightingResult->loadResult();

    if ( m_contourMapProjection )
    {
        RimEclipseCellColors*   cellColors = view()->cellResult();
        RigEclipseResultAddress resAddr( cellColors->resultType(),
                                         cellColors->resultVariable(),
                                         cellColors->timeLapseBaseTimeStep(),
                                         cellColors->caseDiffIndex() );

        RigFloodingSettings fl( m_oilFloodingType(), m_userDefinedFloodingOil(), m_gasFloodingType(), m_userDefinedFloodingGas() );

        dynamic_cast<RigEclipseContourMapProjection*>( m_contourMapProjection.get() )
            ->generateAndSaveResults( resAddr, m_resultAggregation(), timeStep, fl );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseContourMapProjection::resultVariableChanged() const
{
    if ( !m_currentResultName.isEmpty() )
    {
        RimEclipseCellColors* cellColors = view()->cellResult();
        if ( cellColors->resultVariable() != m_currentResultName )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::clearResultVariable()
{
    m_currentResultName = "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::updateGridInformation()
{
    auto eclipseCase     = this->eclipseCase();
    auto eclipseCaseData = eclipseCase->eclipseCaseData();
    auto resultData      = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    cvf::BoundingBox gridBoundingBox = eclipseCase->activeCellsBoundingBox();
    m_contourMapGrid                 = std::make_unique<RigContourMapGrid>( gridBoundingBox, sampleSpacing() );
    m_contourMapProjection           = std::make_unique<RigEclipseContourMapProjection>( *m_contourMapGrid, *eclipseCaseData, *resultData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimEclipseContourMapProjection::retrieveParameterWeights()
{
    std::vector<double> weights;
    if ( m_weightByParameter() )
    {
        RigEclipseResultAddress gridScalarResultIdx = m_weightingResult->eclipseResultAddress();
        if ( gridScalarResultIdx.isValid() )
        {
            m_weightingResult->loadResult();
            int timeStep = 0;
            if ( m_weightingResult->hasDynamicResult() )
            {
                timeStep = view()->currentTimeStep();
            }
            weights = m_weightingResult->currentGridCellResults()->cellScalarResults( gridScalarResultIdx, timeStep );
        }
    }
    return weights;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimEclipseContourMapProjection::eclipseCase() const
{
    auto view = firstAncestorOrThisOfType<Rim3dView>();
    if ( !view ) return nullptr;

    return dynamic_cast<RimEclipseCase*>( view->ownerCase() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RimEclipseContourMapProjection::baseView() const
{
    return view();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapView* RimEclipseContourMapProjection::view() const
{
    return firstAncestorOrThisOfTypeAsserted<RimEclipseContourMapView>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::updateAfterResultGeneration( int timeStep )
{
    m_currentResultTimestep = timeStep;

    RimEclipseCellColors* cellColors = view()->cellResult();
    m_currentResultName              = cellColors->resultVariable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimContourMapProjection::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_weightByParameter || changedField == &m_weightingResult )
    {
        clearGridMapping();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimContourMapProjection::defineUiOrdering( uiConfigName, uiOrdering );

    caf::PdmUiGroup* weightingGroup = uiOrdering.addNewGroup( "Mean Weighting Options" );
    weightingGroup->add( &m_weightByParameter );
    weightingGroup->setCollapsedByDefault();

    m_weightByParameter.uiCapability()->setUiReadOnly( !isMeanResult() );
    if ( !isMeanResult() )
    {
        m_weightByParameter = false;
    }

    if ( m_weightByParameter() )
    {
        m_weightingResult->uiOrdering( uiConfigName, *weightingGroup );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseContourMapProjection::initAfterRead()
{
    RimContourMapProjection::initAfterRead();
    if ( eclipseCase() )
    {
        m_weightingResult->setEclipseCase( eclipseCase() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimEclipseContourMapProjection::computeMinMaxValuesAllTimeSteps()
{
    double minimum = std::numeric_limits<double>::infinity();
    double maximum = -std::numeric_limits<double>::infinity();

    int timeStepCount = std::max( static_cast<int>( eclipseCase()->timeStepStrings().size() ), 1 );
    for ( int i = 0; i < (int)timeStepCount; ++i )
    {
        std::vector<double> aggregatedResults = generateResults( i );
        minimum                               = std::min( minimum, RigContourMapProjection::minValue( aggregatedResults ) );
        maximum                               = std::max( maximum, RigContourMapProjection::maxValue( aggregatedResults ) );
    }
    return std::make_pair( minimum, maximum );
}
