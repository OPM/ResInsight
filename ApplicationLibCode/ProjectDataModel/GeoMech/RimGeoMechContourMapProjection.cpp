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
#include "RimGeoMechContourMapProjection.h"

#include "ContourMap/RigContourMapCalculator.h"
#include "ContourMap/RigContourMapGrid.h"
#include "ContourMap/RigContourMapProjection.h"
#include "ContourMap/RigGeoMechContourMapProjection.h"
#include "RigFemAddressDefines.h"
#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimCellFilterCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechContourMapView.h"
#include "RimPropertyFilterCollection.h"
#include "RimRegularLegendConfig.h"

#include "RivFemElmVisibilityCalculator.h"

#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfArray.h"
#include "cvfStructGridGeometryGenerator.h"
#include "cvfVector3.h"

CAF_PDM_SOURCE_INIT( RimGeoMechContourMapProjection, "RimGeoMechContourMapProjection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapProjection::RimGeoMechContourMapProjection()
{
    CAF_PDM_InitObject( "Map Projection", ":/2DMapProjection16x16.png" );
    CAF_PDM_InitField( &m_limitToPorePressureRegions, "LimitToPorRegion", true, "Limit to Pore Pressure regions" );
    CAF_PDM_InitField( &m_applyPPRegionLimitVertically, "VerticalLimit", false, "Apply Limit Vertically" );
    CAF_PDM_InitField( &m_paddingAroundPorePressureRegion, "PaddingAroundPorRegion", 0.0, "Horizontal Padding around PP regions" );
    m_paddingAroundPorePressureRegion.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    setName( "Map Projection" );
    nameField()->uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapProjection::~RimGeoMechContourMapProjection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechContourMapProjection::resultDescriptionText() const
{
    QString resultText = QString( "%1, %2" ).arg( resultAggregationText() ).arg( view()->cellResult()->resultFieldUiName() );
    return resultText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechContourMapProjection::resultVariableName() const
{
    if ( auto v = view() )
    {
        if ( auto c = v->cellResult() )
        {
            return c->resultFieldUiName();
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimGeoMechContourMapProjection::legendConfig() const
{
    if ( auto v = view() )
    {
        if ( auto c = v->cellResult() )
        {
            return c->legendConfig();
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::updateLegend()
{
    RimGeoMechCellColors* cellColors = view()->cellResult();

    double minVal = m_contourMapProjection->minValue();
    double maxVal = m_contourMapProjection->maxValue();

    std::pair<double, double> minmaxValAllTimeSteps = minmaxValuesAllTimeSteps();

    legendConfig()->setAutomaticRanges( minmaxValAllTimeSteps.first, minmaxValAllTimeSteps.second, minVal, maxVal );

    QString projectionLegendText = QString( "Map Projection\n%1" ).arg( m_resultAggregation().uiText() );
    if ( cellColors->resultAddress().isValid() )
    {
        projectionLegendText += QString( "\nResult: %1" ).arg( cellColors->resultFieldUiName() );
        if ( !cellColors->resultComponentUiName().isEmpty() )
        {
            projectionLegendText += QString( ", %1" ).arg( cellColors->resultComponentUiName() );
        }
    }
    else
    {
        projectionLegendText += QString( "\nNo Result Selected" );
    }

    legendConfig()->setTitle( projectionLegendText );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechContourMapProjection::sampleSpacing() const
{
    RimGeoMechCase* geoMechCase = this->geoMechCase();
    if ( geoMechCase )
    {
        return sampleSpacingFactor() * geoMechCase->characteristicCellSize();
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RimGeoMechContourMapProjection::getCellVisibility() const
{
    cvf::ref<cvf::UByteArray> cellGridIdxVisibility = new cvf::UByteArray( m_femPart->elementCount() );
    RivFemElmVisibilityCalculator::computeAllVisible( cellGridIdxVisibility.p(), m_femPart.p() );

    if ( view()->cellFilterCollection()->isActive() )
    {
        cvf::CellRangeFilter cellRangeFilter;
        view()->cellFilterCollection()->compoundCellRangeFilter( &cellRangeFilter, 0 );

        cvf::UByteArray indexIncludeVis = ( *cellGridIdxVisibility.p() );
        cvf::UByteArray indexExcludeVis = ( *cellGridIdxVisibility.p() );
        view()->cellFilterCollection()->updateCellVisibilityByIndex( &indexIncludeVis, &indexExcludeVis, 0 );

        RivFemElmVisibilityCalculator::computeRangeVisibility( cellGridIdxVisibility.p(),
                                                               m_femPart.p(),
                                                               cellRangeFilter,
                                                               &indexIncludeVis,
                                                               &indexExcludeVis,
                                                               view()->cellFilterCollection()->hasActiveIncludeIndexFilters(),
                                                               view()->cellFilterCollection()->useAndOperation() );
    }
    if ( view()->propertyFilterCollection()->isActive() )
    {
        auto [stepIdx, frameIdx] = view()->currentStepAndDataFrame();

        RivFemElmVisibilityCalculator::computePropertyVisibility( cellGridIdxVisibility.p(),
                                                                  m_femPart.p(),
                                                                  stepIdx,
                                                                  frameIdx,
                                                                  cellGridIdxVisibility.p(),
                                                                  view()->geoMechPropertyFilterCollection() );
    }

    return cellGridIdxVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::updateGridInformation()
{
    RimGeoMechCase* geoMechCase = this->geoMechCase();
    if ( !geoMechCase ) return;

    m_femPart     = geoMechCase->geoMechData()->femParts()->part( 0 );
    m_femPartGrid = m_femPart->getOrCreateStructGrid();
    m_femPart->ensureIntersectionSearchTreeIsBuilt();

    cvf::BoundingBox gridBoundingBox = geoMechCase->activeCellsBoundingBox();
    cvf::BoundingBox expandedBoundingBox;

    if ( m_limitToPorePressureRegions )
    {
        auto [stepIdx, frameIdx] = view()->currentStepAndDataFrame();

        expandedBoundingBox =
            RigGeoMechContourMapProjection::calculateExpandedPorBarBBox( *geoMechCase->geoMechData(),
                                                                         view()->cellResult()->resultComponentName().toStdString(),
                                                                         stepIdx,
                                                                         frameIdx,
                                                                         m_paddingAroundPorePressureRegion() );
        if ( !expandedBoundingBox.isValid() )
        {
            m_limitToPorePressureRegions = false;
        }
    }

    if ( !m_limitToPorePressureRegions )
    {
        expandedBoundingBox = gridBoundingBox;
    }

    cvf::Vec3d minExpandedPoint = expandedBoundingBox.min() - cvf::Vec3d( gridEdgeOffset(), gridEdgeOffset(), 0.0 );
    cvf::Vec3d maxExpandedPoint = expandedBoundingBox.max() + cvf::Vec3d( gridEdgeOffset(), gridEdgeOffset(), 0.0 );
    if ( m_limitToPorePressureRegions && !m_applyPPRegionLimitVertically )
    {
        minExpandedPoint.z() = gridBoundingBox.min().z();
        maxExpandedPoint.z() = gridBoundingBox.max().z();
    }
    expandedBoundingBox = cvf::BoundingBox( minExpandedPoint, maxExpandedPoint );

    m_contourMapGrid       = std::make_unique<RigContourMapGrid>( gridBoundingBox, expandedBoundingBox, sampleSpacing() );
    m_contourMapProjection = std::make_unique<RigGeoMechContourMapProjection>( *geoMechCase->geoMechData(),
                                                                               *m_contourMapGrid,
                                                                               m_limitToPorePressureRegions,
                                                                               m_paddingAroundPorePressureRegion );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGeoMechContourMapProjection::retrieveParameterWeights()
{
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGeoMechContourMapProjection::generateResults( int viewerStepIndex ) const
{
    if ( m_contourMapProjection )
    {
        RimGeoMechCellColors* cellColors    = view()->cellResult();
        RigFemResultAddress   resultAddress = cellColors->resultAddress();
        return generateResultsFromAddress( resultAddress, m_mapCellVisibility, viewerStepIndex );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::generateAndSaveResults( int timeStep )
{
    if ( m_contourMapProjection )
    {
        RimGeoMechCellColors* cellColors    = view()->cellResult();
        RigFemResultAddress   resultAddress = cellColors->resultAddress();
        dynamic_cast<RigGeoMechContourMapProjection*>( m_contourMapProjection.get() )
            ->generateAndSaveResults( resultAddress, m_resultAggregation(), timeStep );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGeoMechContourMapProjection::generateResultsFromAddress( RigFemResultAddress      resultAddress,
                                                                                const std::vector<bool>& mapCellVisibility,
                                                                                int                      viewerStepIndex ) const
{
    return dynamic_cast<RigGeoMechContourMapProjection*>( m_contourMapProjection.get() )
        ->generateResultsFromAddress( resultAddress, mapCellVisibility, m_resultAggregation(), viewerStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechContourMapProjection::resultVariableChanged() const
{
    RimGeoMechCellColors* cellColors = view()->cellResult();
    RigFemResultAddress   resAddr    = cellColors->resultAddress();

    return !m_currentResultAddr.isValid() || !( m_currentResultAddr == resAddr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::clearResultVariable()
{
    m_currentResultAddr = RigFemResultAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridView* RimGeoMechContourMapProjection::baseView() const
{
    return view();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimGeoMechContourMapProjection::geoMechCase() const
{
    return firstAncestorOrThisOfType<RimGeoMechCase>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapView* RimGeoMechContourMapProjection::view() const
{
    return firstAncestorOrThisOfTypeAsserted<RimGeoMechContourMapView>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::updateAfterResultGeneration( int timeStep )
{
    m_currentResultTimestep = timeStep;

    RimGeoMechCellColors* cellColors = view()->cellResult();
    RigFemResultAddress   resAddr    = cellColors->resultAddress();
    m_currentResultAddr              = resAddr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimContourMapProjection::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_limitToPorePressureRegions || changedField == &m_applyPPRegionLimitVertically ||
         changedField == &m_paddingAroundPorePressureRegion )
    {
        clearGridMapping();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoMechContourMapProjection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_resultAggregation )
    {
        std::vector<RigContourMapCalculator::ResultAggregationType> validOptions = { RigContourMapCalculator::TOP_VALUE,
                                                                                     RigContourMapCalculator::MEAN,
                                                                                     RigContourMapCalculator::GEOMETRIC_MEAN,
                                                                                     RigContourMapCalculator::HARMONIC_MEAN,
                                                                                     RigContourMapCalculator::MIN_VALUE,
                                                                                     RigContourMapCalculator::MAX_VALUE,
                                                                                     RigContourMapCalculator::SUM };

        for ( RigContourMapCalculator::ResultAggregationType option : validOptions )
        {
            options.push_back( caf::PdmOptionItemInfo( ResultAggregation::uiText( option ), option ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimContourMapProjection::defineUiOrdering( uiConfigName, uiOrdering );
    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Map Boundaries" );
    group->add( &m_limitToPorePressureRegions );
    group->add( &m_applyPPRegionLimitVertically );
    group->add( &m_paddingAroundPorePressureRegion );
    m_applyPPRegionLimitVertically.uiCapability()->setUiReadOnly( !m_limitToPorePressureRegions() );
    m_paddingAroundPorePressureRegion.uiCapability()->setUiReadOnly( !m_limitToPorePressureRegions() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechContourMapProjection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                            QString                    uiConfigName,
                                                            caf::PdmUiEditorAttribute* attribute )
{
    RimContourMapProjection::defineEditorAttribute( field, uiConfigName, attribute );
    if ( field == &m_paddingAroundPorePressureRegion )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum                       = 0.0;
            myAttr->m_maximum                       = 2.0;
            myAttr->m_sliderTickCount               = 4;
            myAttr->m_delaySliderUpdateUntilRelease = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimGeoMechContourMapProjection::computeMinMaxValuesAllTimeSteps()
{
    double minimum = std::numeric_limits<double>::infinity();
    double maximum = -std::numeric_limits<double>::infinity();

    if ( geoMechCase()->geoMechData()->femPartResults() )
    {
        int steps = geoMechCase()->geoMechData()->femPartResults()->totalSteps();
        for ( int stepIdx = 0; stepIdx < steps; stepIdx++ )
        {
            std::vector<double> aggregatedResults = generateResults( stepIdx );
            minimum                               = std::min( minimum, RigContourMapProjection::minValue( aggregatedResults ) );
            maximum                               = std::max( maximum, RigContourMapProjection::maxValue( aggregatedResults ) );
        }
    }
    return std::make_pair( minimum, maximum );
}
