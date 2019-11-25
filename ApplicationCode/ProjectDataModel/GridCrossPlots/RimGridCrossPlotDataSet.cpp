/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimGridCrossPlotDataSet.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaLogging.h"

#include "RifTextDataTableFormatter.h"

#include "RigActiveCellInfo.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigCaseCellResultCalculator.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseCrossPlotDataExtractor.h"

#include "RigFormationNames.h"
#include "RigMainGrid.h"

#include "RiuGridCrossQwtPlot.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimFormationNames.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurve.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimTools.h"

#include "CellFilters/RimPlotCellFilterCollection.h"

#include "cafCategoryMapper.h"
#include "cafColorTable.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"
#include "cvfScalarMapper.h"
#include "cvfqtUtils.h"

#include <QString>

CAF_PDM_SOURCE_INIT( RimGridCrossPlotDataSet, "GridCrossPlotCurveSet" );

namespace caf
{
template <>
void RimGridCrossPlotDataSet::CurveGroupingEnum::setUp()
{
    addItem( RigGridCrossPlotCurveGrouping::NO_GROUPING, "NONE", "Nothing" );
    addItem( RigGridCrossPlotCurveGrouping::GROUP_BY_TIME, "TIME", "Time Step" );
    addItem( RigGridCrossPlotCurveGrouping::GROUP_BY_FORMATION, "FORMATION", "Formations" );
    addItem( RigGridCrossPlotCurveGrouping::GROUP_BY_RESULT, "RESULT", "Result Property" );
    setDefault( RigGridCrossPlotCurveGrouping::NO_GROUPING );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotDataSet::RimGridCrossPlotDataSet()
{
    CAF_PDM_InitObject( "Cross Plot Data Set", ":/WellLogCurve16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_case, "Case", "Case", "", "", "" );
    m_case.uiCapability()->setUiTreeChildrenHidden( true );
    CAF_PDM_InitField( &m_timeStep, "TimeStep", -1, "Time Step", "", "", "" );
    m_timeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_cellFilterView, "VisibleCellView", "Filter by 3d View Visibility", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_grouping, "Grouping", "Group Data by", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_xAxisProperty, "XAxisProperty", "X-Axis Property", "", "", "" );
    m_xAxisProperty = new RimEclipseResultDefinition( caf::PdmUiItemInfo::TOP );
    m_xAxisProperty.uiCapability()->setUiHidden( true );
    m_xAxisProperty.uiCapability()->setUiTreeChildrenHidden( true );
    m_xAxisProperty->setTernaryEnabled( false );

    CAF_PDM_InitFieldNoDefault( &m_yAxisProperty, "YAxisProperty", "Y-Axis Property", "", "", "" );
    m_yAxisProperty = new RimEclipseResultDefinition( caf::PdmUiItemInfo::TOP );
    m_yAxisProperty.uiCapability()->setUiHidden( true );
    m_yAxisProperty.uiCapability()->setUiTreeChildrenHidden( true );

    m_yAxisProperty->setTernaryEnabled( false );

    CAF_PDM_InitFieldNoDefault( &m_groupingProperty, "GroupingProperty", "Data Grouping Property", "", "", "" );
    m_groupingProperty = new RimEclipseCellColors;
    m_groupingProperty.uiCapability()->setUiHidden( true );
    CVF_ASSERT( m_groupingProperty->legendConfig() );
    m_groupingProperty->legendConfig()->setMappingMode( RimRegularLegendConfig::CATEGORY_INTEGER );
    m_groupingProperty->setTernaryEnabled( false );

    CAF_PDM_InitFieldNoDefault( &m_nameConfig, "NameConfig", "Name", "", "", "" );
    m_nameConfig = new RimGridCrossPlotDataSetNameConfig();
    m_nameConfig.uiCapability()->setUiTreeHidden( true );
    m_nameConfig.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_crossPlotCurves, "CrossPlotCurves", "Curves", "", "", "" );
    m_crossPlotCurves.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitField( &m_useCustomColor, "UseCustomColor", false, "Use Custom Color", "", "", "" );
    CAF_PDM_InitField( &m_customColor, "CustomColor", cvf::Color3f( cvf::Color3f::BLACK ), "Custom Color", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_plotCellFilterCollection, "PlotCellFilterCollection", "Cell Filters", "", "", "" );
    m_plotCellFilterCollection.uiCapability()->setUiHidden( true );
    m_plotCellFilterCollection.uiCapability()->setUiTreeHidden( true );
    m_plotCellFilterCollection.uiCapability()->setUiTreeChildrenHidden( true );
    m_plotCellFilterCollection = new RimPlotCellFilterCollection;

    setDefaults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::setCellFilterView( RimGridView* cellFilterView )
{
    m_cellFilterView = cellFilterView;
    auto eclipseView = dynamic_cast<RimEclipseView*>( m_cellFilterView() );

    if ( eclipseView )
    {
        m_groupingProperty->setReservoirView( eclipseView );
        RigEclipseResultAddress resAddr = eclipseView->cellResult()->eclipseResultAddress();
        if ( resAddr.isValid() )
        {
            m_xAxisProperty->setResultType( resAddr.m_resultCatType );
            m_xAxisProperty->setResultVariable( resAddr.m_resultName );
            m_yAxisProperty->setResultType( RiaDefines::STATIC_NATIVE );
            m_yAxisProperty->setResultVariable( "DEPTH" );
            m_timeStep = eclipseView->currentTimeStep();
            m_grouping = NO_GROUPING;
            if ( eclipseView->eclipseCase() && eclipseView->eclipseCase()->activeFormationNames() )
            {
                m_grouping = GROUP_BY_FORMATION;
                m_groupingProperty->legendConfig()->setColorRange( RimRegularLegendConfig::CATEGORY );
            }

            RimGridCrossPlot* parentPlot = nullptr;
            firstAncestorOrThisOfType( parentPlot );
            if ( parentPlot )
            {
                parentPlot->setYAxisInverted( true );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::loadDataAndUpdate( bool updateParentPlot )
{
    onLoadDataAndUpdate( updateParentPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::setParentQwtPlotNoReplot( QwtPlot* parent )
{
    for ( auto& curve : m_crossPlotCurves() )
    {
        curve->setParentQwtPlotNoReplot( m_isChecked() ? parent : nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotDataSet::xAxisName() const
{
    return m_xAxisProperty->resultVariableUiShortName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotDataSet::yAxisName() const
{
    return m_yAxisProperty->resultVariableUiShortName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotDataSet::infoText() const
{
    if ( !m_case() ) return "";

    if ( visibleCurveCount() == 0u ) return "";

    QStringList textLines;
    textLines += QString( "<b>Case:</b> %1" ).arg( m_case()->caseUserDescription() );
    textLines += QString( "<b>Parameters:</b>: %1 x %2" )
                     .arg( m_xAxisProperty->resultVariableUiShortName() )
                     .arg( m_yAxisProperty->resultVariableUiShortName() );
    if ( m_timeStep != -1 )
    {
        textLines += QString( "<b>Time step:</b> %1" ).arg( timeStepString() );
    }
    if ( m_grouping != NO_GROUPING )
    {
        textLines += QString( "<b>Grouped By:</b> %1" ).arg( groupParameter() );
    }
    if ( m_cellFilterView() )
    {
        textLines += QString( "<b>Filter view:</b> %1" ).arg( m_cellFilterView->name() );
    }
    textLines += QString( "<b>Sample Count:</b> %1" ).arg( sampleCount() );
    return textLines.join( "<br/>\n" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCrossPlotDataSet::indexInPlot() const
{
    RimGridCrossPlot* parent;
    this->firstAncestorOrThisOfTypeAsserted( parent );
    return parent->indexOfDataSet( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotDataSet::createAutoName() const
{
    if ( m_case() == nullptr )
    {
        return "Undefined";
    }

    QStringList nameTags;
    if ( !m_nameConfig->customName().isEmpty() )
    {
        nameTags += m_nameConfig->customName();
    }

    if ( m_nameConfig->addCaseName() )
    {
        nameTags += caseNameString();
    }

    if ( m_nameConfig->addAxisVariables() )
    {
        nameTags += axisVariableString();
    }

    if ( m_nameConfig->addTimestep() && !timeStepString().isEmpty() )
    {
        nameTags += timeStepString();
    }

    if ( m_nameConfig->addGrouping() && groupParameter() != "None" )
    {
        QString catTitle = groupTitle();
        if ( !catTitle.isEmpty() ) nameTags += catTitle;
    }

    return nameTags.join( ", " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotDataSet::groupTitle() const
{
    if ( m_grouping != NO_GROUPING )
    {
        return QString( "Grouped by %1" ).arg( groupParameter() );
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotDataSet::groupParameter() const
{
    if ( m_grouping() == GROUP_BY_TIME )
    {
        return QString( "Time Steps" );
    }
    else if ( m_grouping() == GROUP_BY_FORMATION )
    {
        return QString( "Formations" );
    }
    else if ( m_grouping() == GROUP_BY_RESULT && m_groupingProperty->hasResult() )
    {
        return QString( "%1" ).arg( m_groupingProperty->resultVariableUiShortName() );
    }
    return "None";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::detachAllCurves()
{
    for ( auto curve : m_crossPlotCurves() )
    {
        curve->detachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::cellFilterViewUpdated()
{
    if ( m_cellFilterView() )
    {
        loadDataAndUpdate( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimGridCrossPlotDataSet::legendConfig() const
{
    return m_groupingProperty->legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridCrossPlotCurve*> RimGridCrossPlotDataSet::curves() const
{
    return m_crossPlotCurves.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotDataSet::caseNameString() const
{
    if ( m_case() )
    {
        return m_case->caseUserDescription();
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotDataSet::axisVariableString() const
{
    return QString( "%1 x %2" ).arg( xAxisName(), yAxisName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotDataSet::timeStepString() const
{
    // If using time categorization, the time step will be included as a category, so skip it here.
    if ( m_grouping() != RigGridCrossPlotCurveGrouping::GROUP_BY_TIME )
    {
        if ( m_case() && ( m_xAxisProperty->hasDynamicResult() || m_yAxisProperty->hasDynamicResult() ) )
        {
            if ( m_timeStep == -1 )
            {
                return "All Time Steps";
            }
            return m_case->timeStepStrings()[m_timeStep];
        }
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<RimGridCrossPlotDataSet::NameComponents, QString> RimGridCrossPlotDataSet::nameComponents() const
{
    std::map<RimGridCrossPlotDataSet::NameComponents, QString> componentNames;
    if ( m_nameConfig->addCaseName() ) componentNames[GCP_CASE_NAME] = caseNameString();
    if ( m_nameConfig->addAxisVariables() ) componentNames[GCP_AXIS_VARIABLES] = axisVariableString();
    if ( m_nameConfig->addTimestep() ) componentNames[GCP_TIME_STEP] = timeStepString();
    if ( m_nameConfig->addGrouping() ) componentNames[GCP_GROUP_NAME] = groupTitle();

    return componentNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::initAfterRead()
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
    if ( eclipseCase )
    {
        m_xAxisProperty->setEclipseCase( eclipseCase );
        m_yAxisProperty->setEclipseCase( eclipseCase );
        m_groupingProperty->setEclipseCase( eclipseCase );
        m_plotCellFilterCollection->setCase( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::onLoadDataAndUpdate( bool updateParentPlot )
{
    updateDataSetName();

    if ( m_case() == nullptr )
    {
        return;
    }

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );

    if ( eclipseCase == nullptr )
    {
        return;
    }

    if ( !eclipseCase->ensureReservoirCaseIsOpen() )
    {
        RiaLogging::warning( QString( "Failed to open eclipse grid file %1" ).arg( eclipseCase->gridFileName() ) );

        return;
    }

    RigEclipseResultAddress xAddress( m_xAxisProperty->resultType(), m_xAxisProperty->resultVariable() );
    RigEclipseResultAddress yAddress( m_yAxisProperty->resultType(), m_yAxisProperty->resultVariable() );
    RigEclipseResultAddress groupAddress( m_groupingProperty->resultType(), m_groupingProperty->resultVariable() );

    std::map<int, cvf::UByteArray> timeStepCellVisibilityMap = calculateCellVisibility( eclipseCase );

    updateLegendRange();

    RigEclipseCrossPlotResult result = RigEclipseCrossPlotDataExtractor::extract( eclipseCase->eclipseCaseData(),
                                                                                  m_timeStep(),
                                                                                  xAddress,
                                                                                  yAddress,
                                                                                  m_grouping(),
                                                                                  groupAddress,
                                                                                  timeStepCellVisibilityMap );

    if ( isXAxisLogarithmic() || isYAxisLogarithmic() )
    {
        filterInvalidCurveValues( &result );
    }

    assignCurveDataGroups( result );

    if ( m_crossPlotCurves.size() != m_groupedResults.size() )
    {
        destroyCurves();
    }

    if ( m_crossPlotCurves.empty() )
    {
        createCurves( result );
    }
    else
    {
        fillCurveDataInExistingCurves( result );
    }

    updateLegendIcons();

    if ( updateParentPlot )
    {
        triggerPlotNameUpdateAndReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::assignCurveDataGroups( const RigEclipseCrossPlotResult& result )
{
    m_groupedResults.clear();

    if ( groupingEnabled() && ( result.groupValuesContinuous.empty() && result.groupValuesDiscrete.empty() ) )
    {
        // Basis for group determination (i.e. formation list) may have been deleted since the grouping was assigned.
        m_grouping = NO_GROUPING;
    }

    if ( !groupingEnabled() )
    {
        m_groupedResults[0] = result;
    }
    else
    {
        std::vector<double> tickValues;

        if ( groupingByCategoryResult() )
        {
            for ( size_t i = 0; i < result.xValues.size(); ++i )
            {
                int categoryNum = m_grouping == GROUP_BY_RESULT ? static_cast<int>( result.groupValuesContinuous[i] )
                                                                : result.groupValuesDiscrete[i];

                m_groupedResults[categoryNum].xValues.push_back( result.xValues[i] );
                m_groupedResults[categoryNum].yValues.push_back( result.yValues[i] );
                if ( !result.groupValuesContinuous.empty() )
                    m_groupedResults[categoryNum].groupValuesContinuous.push_back( result.groupValuesContinuous[i] );
                if ( !result.groupValuesDiscrete.empty() )
                    m_groupedResults[categoryNum].groupValuesDiscrete.push_back( result.groupValuesDiscrete[i] );
            }
        }
        else
        {
            legendConfig()->scalarMapper()->majorTickValues( &tickValues );

            for ( size_t i = 0; i < result.xValues.size(); ++i )
            {
                auto upperBoundIt    = std::lower_bound( tickValues.begin(),
                                                      tickValues.end(),
                                                      result.groupValuesContinuous[i] );
                int  upperBoundIndex = static_cast<int>( upperBoundIt - tickValues.begin() );
                int  categoryNum     = std::min( (int)tickValues.size() - 2, std::max( 0, upperBoundIndex - 1 ) );
                m_groupedResults[categoryNum].xValues.push_back( result.xValues[i] );
                m_groupedResults[categoryNum].yValues.push_back( result.yValues[i] );
                if ( !result.groupValuesContinuous.empty() )
                    m_groupedResults[categoryNum].groupValuesContinuous.push_back( result.groupValuesContinuous[i] );
                if ( !result.groupValuesDiscrete.empty() )
                    m_groupedResults[categoryNum].groupValuesDiscrete.push_back( result.groupValuesDiscrete[i] );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::createCurves( const RigEclipseCrossPlotResult& result )
{
    if ( !groupingEnabled() )
    {
        RimGridCrossPlotCurve* curve = new RimGridCrossPlotCurve();
        if ( m_useCustomColor )
        {
            curve->setColor( m_customColor() );
        }
        else
        {
            const caf::ColorTable& colors     = RiaColorTables::contrastCategoryPaletteColors();
            int                    colorIndex = indexInPlot();
            curve->setColor( colors.cycledColor3f( colorIndex ) );
        }
        curve->setSymbolEdgeColor( curve->color() );
        curve->setGroupingInformation( indexInPlot(), 0 );
        curve->setSamples( result.xValues, result.yValues );
        curve->setCurveAutoAppearance();
        curve->updateUiIconFromPlotSymbol();
        m_crossPlotCurves.push_back( curve );
    }
    else
    {
        std::vector<double> tickValues;

        if ( !groupingByCategoryResult() )
        {
            legendConfig()->scalarMapper()->majorTickValues( &tickValues );
        }

        // NB : Make sure iteration of curve and groups are syncronized with createCurves()
        for ( auto it = m_groupedResults.rbegin(); it != m_groupedResults.rend(); ++it )
        {
            RimGridCrossPlotCurve* curve = new RimGridCrossPlotCurve();
            curve->setGroupingInformation( indexInPlot(), it->first );
            if ( groupingByCategoryResult() )
            {
                curve->setColor( cvf::Color3f( legendConfig()->scalarMapper()->mapToColor( it->first ) ) );
            }
            else
            {
                curve->setColor( cvf::Color3f( legendConfig()->scalarMapper()->mapToColor( tickValues[it->first] ) ) );
            }
            curve->setSymbolEdgeColor( curve->color() );
            curve->setSamples( it->second.xValues, it->second.yValues );
            curve->showLegend( m_crossPlotCurves.empty() );
            curve->setLegendEntryText( createAutoName() );
            curve->setCurveAutoAppearance();
            curve->updateUiIconFromPlotSymbol();
            m_crossPlotCurves.push_back( curve );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::fillCurveDataInExistingCurves( const RigEclipseCrossPlotResult& result )
{
    if ( !groupingEnabled() )
    {
        CVF_ASSERT( m_crossPlotCurves.size() == 1u );
        RimGridCrossPlotCurve* curve = m_crossPlotCurves[0];
        curve->setSamples( result.xValues, result.yValues );
        curve->setGroupingInformation( indexInPlot(), 0 );
        curve->updateCurveAppearance();
        curve->updateUiIconFromPlotSymbol();
    }
    else
    {
        // NB : Make sure iteration of curve and groups are syncronized with fillCurveDataInExistingCurves()
        auto curveIt = m_crossPlotCurves.begin();
        auto groupIt = m_groupedResults.rbegin();
        for ( ; curveIt != m_crossPlotCurves.end() && groupIt != m_groupedResults.rend(); ++curveIt, ++groupIt )
        {
            RimGridCrossPlotCurve* curve = *curveIt;
            curve->setSamples( groupIt->second.xValues, groupIt->second.yValues );
            curve->setGroupingInformation( indexInPlot(), groupIt->first );
            curve->updateCurveAppearance();
            curve->updateUiIconFromPlotSymbol();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::destroyCurves()
{
    detachAllCurves();
    m_crossPlotCurves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimGridCrossPlotDataSet::visibleCurveCount() const
{
    size_t visibleCurves = 0;
    for ( auto curve : m_crossPlotCurves )
    {
        if ( curve && curve->isCurveVisible() ) visibleCurves++;
    }
    return visibleCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimGridCrossPlotDataSet::sampleCount() const
{
    size_t sampleCount = 0;
    for ( auto curve : m_crossPlotCurves )
    {
        if ( curve && curve->isCurveVisible() ) sampleCount += curve->sampleCount();
    }
    return sampleCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotDataSet::createGroupName( size_t groupIndex ) const
{
    if ( groupingByCategoryResult() )
    {
        return legendConfig()->categoryNameFromCategoryValue( groupIndex );
    }
    else
    {
        std::vector<double> tickValues;
        legendConfig()->scalarMapper()->majorTickValues( &tickValues );
        double lowerLimit = tickValues[groupIndex];
        double upperLimit = groupIndex + 1u < tickValues.size() ? tickValues[groupIndex + 1u]
                                                                : std::numeric_limits<double>::infinity();
        return QString( "%1 [%2, %3]" ).arg( groupParameter() ).arg( lowerLimit ).arg( upperLimit );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, cvf::UByteArray> RimGridCrossPlotDataSet::calculateCellVisibility( RimEclipseCase* eclipseCase ) const
{
    std::map<int, cvf::UByteArray> timeStepCellVisibilityMap;
    if ( m_cellFilterView )
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( m_cellFilterView() );
        if ( eclipseView )
        {
            std::set<int> timeSteps;
            if ( m_timeStep() == -1 )
            {
                for ( int i = 0; i < (int)eclipseCase->timeStepDates().size(); ++i )
                {
                    timeSteps.insert( i );
                }
            }
            else
            {
                timeSteps.insert( m_timeStep() );
            }
            for ( int i : timeSteps )
            {
                eclipseView->calculateCurrentTotalCellVisibility( &timeStepCellVisibilityMap[i], i );
            }
        }
    }
    else if ( m_plotCellFilterCollection->cellFilterCount() > 0 )
    {
        std::set<int> timeSteps;
        if ( m_timeStep() == -1 )
        {
            for ( int i = 0; i < (int)eclipseCase->timeStepDates().size(); ++i )
            {
                timeSteps.insert( i );
            }
        }
        else
        {
            timeSteps.insert( m_timeStep() );
        }

        RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
        if ( eclipseCaseData )
        {
            RiaDefines::PorosityModelType porosityModel = RiaDefines::MATRIX_MODEL;

            RigCaseCellResultsData* cellResultsData = eclipseCaseData->results( porosityModel );
            if ( cellResultsData )
            {
                const RigActiveCellInfo* actCellInfo = cellResultsData->activeCellInfo();
                size_t                   cellCount   = actCellInfo->reservoirCellCount();

                for ( int i : timeSteps )
                {
                    cvf::UByteArray* cellVisibility = &timeStepCellVisibilityMap[i];
                    cellVisibility->resize( cellCount );
                    cellVisibility->setAll( true );

                    m_plotCellFilterCollection->computeCellVisibilityFromFilter( i, cellVisibility );
                }
            }
        }
    }

    return timeStepCellVisibilityMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_case );
    if ( m_case )
    {
        uiOrdering.add( &m_timeStep );
        uiOrdering.add( &m_cellFilterView );
        uiOrdering.add( &m_grouping );

        CVF_ASSERT( m_xAxisProperty && m_yAxisProperty && m_groupingProperty &&
                    "All property objects should always be created" );

        if ( m_grouping() == GROUP_BY_TIME &&
             !( m_xAxisProperty->hasDynamicResult() || m_yAxisProperty->hasDynamicResult() ) )
        {
            m_grouping = NO_GROUPING;
        }

        if ( m_grouping() == GROUP_BY_RESULT )
        {
            caf::PdmUiGroup* dataGroupingGroup = uiOrdering.addNewGroup( "Data Grouping Property" );
            m_groupingProperty->uiOrdering( "AddLegendLevels", *dataGroupingGroup );
        }

        caf::PdmUiGroup* invisibleFullWidthGroup = uiOrdering.addNewGroup( "Property Group" );
        invisibleFullWidthGroup->setEnableFrame( false );

        caf::PdmUiGroup* xAxisGroup = invisibleFullWidthGroup->addNewGroup( "X-Axis Property" );
        m_xAxisProperty->uiOrdering( uiConfigName, *xAxisGroup );

        caf::PdmUiGroup* yAxisGroup = invisibleFullWidthGroup->addNewGroup( "Y-Axis Property", false );
        m_yAxisProperty->uiOrdering( uiConfigName, *yAxisGroup );
    }

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Name Configuration" );
    m_nameConfig->uiOrdering( uiConfigName, *nameGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                const QVariant&            oldValue,
                                                const QVariant&            newValue )
{
    if ( changedField == &m_case )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
        if ( eclipseCase )
        {
            m_xAxisProperty->setEclipseCase( eclipseCase );
            m_yAxisProperty->setEclipseCase( eclipseCase );
            m_groupingProperty->setEclipseCase( eclipseCase );
            // TODO: Do we need all these??
            m_xAxisProperty->updateConnectedEditors();
            m_yAxisProperty->updateConnectedEditors();
            m_groupingProperty->updateConnectedEditors();

            if ( m_grouping == GROUP_BY_FORMATION && !eclipseCase->activeFormationNames() )
            {
                m_grouping = NO_GROUPING;
            }

            destroyCurves();
            loadDataAndUpdate( true );
        }
    }
    else if ( changedField == &m_timeStep )
    {
        if ( m_timeStep != -1 && m_grouping == GROUP_BY_TIME )
        {
            m_grouping = NO_GROUPING;
        }

        destroyCurves();
        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_grouping )
    {
        if ( m_grouping == GROUP_BY_TIME )
        {
            legendConfig()->setColorRange( RimRegularLegendConfig::NORMAL );
            legendConfig()->setMappingMode( RimRegularLegendConfig::CATEGORY_INTEGER );
        }
        else if ( groupingByCategoryResult() )
        {
            legendConfig()->setColorRange( RimRegularLegendConfig::CATEGORY );
            legendConfig()->setMappingMode( RimRegularLegendConfig::CATEGORY_INTEGER );
        }
        else
        {
            legendConfig()->setColorRange( RimRegularLegendConfig::NORMAL );
            legendConfig()->setMappingMode( RimRegularLegendConfig::LINEAR_DISCRETE );
        }

        destroyCurves();
        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_cellFilterView )
    {
        m_groupingProperty->setReservoirView( dynamic_cast<RimEclipseView*>( m_cellFilterView() ) );

        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_isChecked )
    {
        updateLegendRange();
        triggerPlotNameUpdateAndReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    if ( changedChildField == &m_yAxisProperty )
    {
        if ( m_yAxisProperty->resultVariable() == "DEPTH" )
        {
            RimGridCrossPlot* plot;
            this->firstAncestorOrThisOfTypeAsserted( plot );
            plot->setYAxisInverted( true );
            triggerPlotNameUpdateAndReplot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimGridCrossPlotDataSet::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_case )
    {
        RimTools::eclipseCaseOptionItems( &options );
        if ( options.empty() )
        {
            options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
        }
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        QStringList timeStepNames;

        if ( m_case )
        {
            timeStepNames = m_case->timeStepStrings();
        }
        options.push_back( caf::PdmOptionItemInfo( "All Time Steps", -1 ) );
        for ( int i = 0; i < timeStepNames.size(); i++ )
        {
            options.push_back( caf::PdmOptionItemInfo( timeStepNames[i], i ) );
        }
    }
    else if ( fieldNeedingOptions == &m_cellFilterView )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
        if ( eclipseCase )
        {
            options.push_back( caf::PdmOptionItemInfo( "Disabled", nullptr ) );
            for ( RimEclipseView* view : eclipseCase->reservoirViews.childObjects() )
            {
                CVF_ASSERT( view && "Really always should have a valid view pointer in ReservoirViews" );
                options.push_back( caf::PdmOptionItemInfo( view->name(), view, false, view->uiIconProvider() ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_grouping )
    {
        std::set<RigGridCrossPlotCurveGrouping> validOptions = {NO_GROUPING,
                                                                GROUP_BY_TIME,
                                                                GROUP_BY_FORMATION,
                                                                GROUP_BY_RESULT};
        if ( !hasMultipleTimeSteps() )
        {
            validOptions.erase( GROUP_BY_TIME );
        }
        {
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
            if ( !( eclipseCase && eclipseCase->activeFormationNames() ) )
            {
                validOptions.erase( GROUP_BY_FORMATION );
            }
        }
        for ( auto optionItem : validOptions )
        {
            options.push_back( caf::PdmOptionItemInfo( CurveGroupingEnum::uiText( optionItem ), optionItem ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::updateLegendRange()
{
    legendConfig()->setTitle( groupParameter() );
    legendConfig()->disableAllTimeStepsRange( !hasMultipleTimeSteps() );

    RimGridCrossPlot* parent;
    this->firstAncestorOrThisOfTypeAsserted( parent );
    if ( parent->viewer() )
    {
        if ( groupingEnabled() && m_case() && isChecked() && legendConfig()->showLegend() )
        {
            if ( m_grouping() == GROUP_BY_FORMATION )
            {
                RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
                if ( eclipseCase )
                {
                    RigFormationNames* formationNames = eclipseCase->eclipseCaseData()->activeFormationNames();
                    if ( formationNames )
                    {
                        const std::vector<QString>& categoryNames = formationNames->formationNames();
                        if ( !categoryNames.empty() )
                        {
                            legendConfig()->setNamedCategoriesInverse( categoryNames );
                            legendConfig()->setAutomaticRanges( 0, categoryNames.size() - 1, 0, categoryNames.size() - 1 );
                        }
                    }
                }
            }
            else if ( m_grouping() == GROUP_BY_TIME )
            {
                QStringList          timeStepNames = m_case->timeStepStrings();
                std::vector<QString> categoryNames;
                for ( auto name : timeStepNames )
                {
                    categoryNames.push_back( name );
                }
                if ( !categoryNames.empty() )
                {
                    legendConfig()->setNamedCategories( categoryNames );
                    legendConfig()->setAutomaticRanges( 0, categoryNames.size() - 1, 0, categoryNames.size() - 1 );
                }
            }
            else if ( m_groupingProperty->eclipseResultAddress().isValid() )
            {
                RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
                if ( eclipseCase )
                {
                    m_groupingProperty->updateLegendData( eclipseCase, m_timeStep() );
                }
            }
            parent->addOrUpdateDataSetLegend( this );
        }
        else
        {
            parent->removeDataSetLegend( this );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::updateLegendIcons()
{
    for ( auto curve : m_crossPlotCurves )
    {
        curve->determineLegendIcon();
        curve->setBlackAndWhiteLegendIcons( groupingEnabled() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlotDataSet::groupingByCategoryResult() const
{
    if ( m_grouping == GROUP_BY_FORMATION || m_grouping == GROUP_BY_TIME )
    {
        return true;
    }
    else if ( m_grouping == GROUP_BY_RESULT )
    {
        return m_groupingProperty->hasCategoryResult();
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlotDataSet::groupingEnabled() const
{
    if ( m_grouping != NO_GROUPING )
    {
        if ( m_grouping == GROUP_BY_RESULT && !m_groupingProperty->eclipseResultAddress().isValid() )
        {
            return false;
        }
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::swapAxisProperties( bool updatePlot )
{
    RimEclipseResultDefinition* xAxisProperties = m_xAxisProperty();
    RimEclipseResultDefinition* yAxisProperties = m_yAxisProperty();

    m_xAxisProperty.removeChildObject( xAxisProperties );
    m_yAxisProperty.removeChildObject( yAxisProperties );
    m_yAxisProperty = xAxisProperties;
    m_xAxisProperty = yAxisProperties;

    updateConnectedEditors();
    loadDataAndUpdate( updatePlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::exportFormattedData( RifTextDataTableFormatter& formatter ) const
{
    if ( m_groupedResults.empty() ) return;

    QString xTitle = QString( "%1" ).arg( m_xAxisProperty->resultVariableUiShortName() );
    QString yTitle = QString( "%1" ).arg( m_yAxisProperty->resultVariableUiShortName() );

    if ( m_grouping != NO_GROUPING )
    {
        std::vector<RifTextDataTableColumn> header = {RifTextDataTableColumn( xTitle ),
                                                      RifTextDataTableColumn( yTitle ),
                                                      RifTextDataTableColumn( "Group Index" ),
                                                      RifTextDataTableColumn( "Group Description" )};

        formatter.header( header );
    }
    else
    {
        std::vector<RifTextDataTableColumn> header = {RifTextDataTableColumn( xTitle ), RifTextDataTableColumn( yTitle )};
        formatter.header( header );
    }

    caf::ProgressInfo progress( m_groupedResults.size(), "Gathering Data Points" );
    for ( auto it = m_groupedResults.begin(); it != m_groupedResults.end(); ++it )
    {
        auto task = progress.task( QString( "Exporting Group %1" ).arg( it->first ) );

        RigEclipseCrossPlotResult res = it->second;

        for ( size_t i = 0; i < it->second.xValues.size(); ++i )
        {
            if ( m_grouping() == NO_GROUPING )
            {
                formatter.add( res.xValues[i] );
                formatter.add( res.yValues[i] );
            }
            else
            {
                int     groupIndex = it->first;
                QString groupName  = createGroupName( groupIndex );
                formatter.add( res.xValues[i] );
                formatter.add( res.yValues[i] );
                formatter.add( groupIndex );
                formatter.add( groupName );
            }
            formatter.rowCompleted();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlotDataSet::isXAxisLogarithmic() const
{
    RimGridCrossPlot* parent = nullptr;
    firstAncestorOrThisOfTypeAsserted( parent );
    return parent->isXAxisLogarithmic();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlotDataSet::isYAxisLogarithmic() const
{
    RimGridCrossPlot* parent = nullptr;
    firstAncestorOrThisOfTypeAsserted( parent );
    return parent->isYAxisLogarithmic();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::configureForPressureSaturationCurves( RimEclipseResultCase* eclipseCase,
                                                                    const QString&        dynamicResultName )
{
    m_case = eclipseCase;

    m_xAxisProperty->setEclipseCase( eclipseCase );
    m_xAxisProperty->setResultType( RiaDefines::DYNAMIC_NATIVE );
    m_xAxisProperty->setResultVariable( dynamicResultName );

    m_yAxisProperty->setEclipseCase( eclipseCase );
    m_yAxisProperty->setResultType( RiaDefines::STATIC_NATIVE );
    m_yAxisProperty->setResultVariable( "DEPTH" );

    m_grouping = NO_GROUPING;

    m_nameConfig->setCustomName( dynamicResultName );

    m_nameConfig->addCaseName      = false;
    m_nameConfig->addAxisVariables = false;
    m_nameConfig->addTimestep      = false;
    m_nameConfig->addGrouping      = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::addCellFilter( RimPlotCellFilter* cellFilter )
{
    m_plotCellFilterCollection->addCellFilter( cellFilter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::setCustomColor( const cvf::Color3f color )
{
    m_useCustomColor = true;
    m_customColor    = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::triggerPlotNameUpdateAndReplot()
{
    RimGridCrossPlot* parent;
    this->firstAncestorOrThisOfType( parent );
    if ( parent )
    {
        parent->updateCurveNamesAndPlotTitle();
        parent->reattachAllCurves();
        parent->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::updateCurveNames( size_t dataSetIndex, size_t dataSetCount )
{
    for ( size_t i = 0; i < m_crossPlotCurves.size(); ++i )
    {
        QString dataSetName = createAutoName();
        if ( dataSetName.isEmpty() )
        {
            if ( dataSetCount > 1u )
                dataSetName = QString( "DataSet #%1" ).arg( dataSetIndex + 1 );
            else
                dataSetName = "Data Set";
        }

        auto curve = m_crossPlotCurves[i];
        if ( groupingEnabled() )
        {
            QString curveGroupName = createGroupName( curve->groupIndex() );
            curve->setCustomName( curveGroupName );
            curve->setLegendEntryText( dataSetName );
        }
        else
        {
            curve->setCustomName( dataSetName );
        }
        curve->updateCurveNameAndUpdatePlotLegendAndTitle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::updateDataSetName()
{
    this->setName( createAutoName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::performAutoNameUpdate()
{
    updateDataSetName();
    triggerPlotNameUpdateAndReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::setDefaults()
{
    RimProject* project = RiaApplication::instance()->project();
    if ( project )
    {
        if ( !project->eclipseCases().empty() )
        {
            RimEclipseCase* eclipseCase = project->eclipseCases().front();
            m_case                      = eclipseCase;
            m_xAxisProperty->setEclipseCase( eclipseCase );
            m_yAxisProperty->setEclipseCase( eclipseCase );
            m_groupingProperty->setEclipseCase( eclipseCase );

            m_xAxisProperty->setResultType( RiaDefines::STATIC_NATIVE );
            m_xAxisProperty->setResultVariable( "PORO" );

            m_yAxisProperty->setResultType( RiaDefines::STATIC_NATIVE );
            m_yAxisProperty->setResultVariable( "PERMX" );

            m_grouping = NO_GROUPING;
            if ( eclipseCase->activeFormationNames() )
            {
                m_grouping = GROUP_BY_FORMATION;
                m_groupingProperty->legendConfig()->setColorRange( RimRegularLegendConfig::CATEGORY );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                     QString                    uiConfigName,
                                                     caf::PdmUiEditorAttribute* attribute )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                    QString                 uiConfigName /*= ""*/ )
{
    if ( groupingEnabled() )
    {
        m_groupingProperty->uiTreeOrdering( uiTreeOrdering, uiConfigName );
    }

    for ( auto curve : m_crossPlotCurves() )
    {
        uiTreeOrdering.add( curve );
    }

    uiTreeOrdering.add( &m_plotCellFilterCollection );

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlotDataSet::hasMultipleTimeSteps() const
{
    return m_timeStep() == -1 && ( m_xAxisProperty->hasDynamicResult() || m_yAxisProperty->hasDynamicResult() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSet::filterInvalidCurveValues( RigEclipseCrossPlotResult* result )
{
    bool xLog = isXAxisLogarithmic();
    bool yLog = isYAxisLogarithmic();

    if ( xLog || yLog )
    {
        RigEclipseCrossPlotResult validResult;
        for ( size_t i = 0; i < result->xValues.size(); ++i )
        {
            double xValue  = result->xValues[i];
            double yValue  = result->yValues[i];
            bool   invalid = ( xLog && xValue <= 0.0 ) || ( yLog && yValue <= 0.0 );
            if ( !invalid )
            {
                validResult.xValues.push_back( xValue );
                validResult.yValues.push_back( yValue );
                if ( i < result->groupValuesContinuous.size() )
                {
                    validResult.groupValuesContinuous.push_back( result->groupValuesContinuous[i] );
                }
                if ( i < result->groupValuesDiscrete.size() )
                {
                    validResult.groupValuesDiscrete.push_back( result->groupValuesDiscrete[i] );
                }
            }
        }

        *result = validResult;
    }
}

CAF_PDM_SOURCE_INIT( RimGridCrossPlotDataSetNameConfig, "RimGridCrossPlotCurveSetNameConfig" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotDataSetNameConfig::RimGridCrossPlotDataSetNameConfig()
    : RimNameConfig()
{
    CAF_PDM_InitObject( "Cross Plot Data Set NameGenerator", "", "", "" );

    CAF_PDM_InitField( &addCaseName, "AddCaseName", true, "Add Case Name", "", "", "" );
    CAF_PDM_InitField( &addAxisVariables, "AddAxisVariables", true, "Add Axis Variables", "", "", "" );
    CAF_PDM_InitField( &addTimestep, "AddTimeStep", true, "Add Time Step", "", "", "" );
    CAF_PDM_InitField( &addGrouping, "AddGrouping", true, "Add Data Group", "", "", "" );

    setCustomName( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotDataSetNameConfig::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &addCaseName );
    uiOrdering.add( &addAxisVariables );
    uiOrdering.add( &addTimestep );
    uiOrdering.add( &addGrouping );
}
