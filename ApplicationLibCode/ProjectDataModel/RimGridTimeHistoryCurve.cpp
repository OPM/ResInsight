/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimGridTimeHistoryCurve.h"

#include "Tools/Summary/RiaSummaryTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFemResultAddress.h"
#include "RigMainGrid.h"
#include "RigTimeHistoryResultAccessor.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "RimEclipseCase.h"
#include "RimEclipseGeometrySelectionItem.h"
#include "RimEclipseResultDefinition.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechGeometrySelectionItem.h"
#include "RimGeoMechResultDefinition.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTimeAxisProperties.h"
#include "RimTools.h"
#include "Tools/RimAutomationSettings.h"

#include "Riu3dSelectionManager.h"
#include "RiuFemTimeHistoryResultAccessor.h"
#include "RiuPlotCurve.h"
#include "RiuPlotMainWindowTools.h"

#include "cafPdmUiTreeAttributes.h"

CAF_PDM_SOURCE_INIT( RimGridTimeHistoryCurve, "GridTimeHistoryCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridTimeHistoryCurve::RimGridTimeHistoryCurve()
{
    CAF_PDM_InitObject( "Grid Time History Curve", ":/SummaryCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_geometrySelectionText, "GeometrySelectionText", "Cell Reference" );
    m_geometrySelectionText.registerGetMethod( this, &RimGridTimeHistoryCurve::geometrySelectionText );
    m_geometrySelectionText.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_isLocked, "IsLocked", false, "Lock Curve" );

    CAF_PDM_InitFieldNoDefault( &m_eclipseResultDefinition, "EclipseResultDefinition", "Eclipse Result Definition" );
    m_eclipseResultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_eclipseResultDefinition = new RimEclipseResultDefinition;

    CAF_PDM_InitFieldNoDefault( &m_geoMechResultDefinition, "GeoMechResultDefinition", "GeoMech Result Definition" );
    m_geoMechResultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_geoMechResultDefinition = new RimGeoMechResultDefinition;

    CAF_PDM_InitFieldNoDefault( &m_eclipseDataSource, "EclipseDataSource", "Eclipse Data Source" );
    m_eclipseDataSource.uiCapability()->setUiTreeChildrenHidden( true );
    m_eclipseDataSource = new RimEclipseGeometrySelectionItem;

    CAF_PDM_InitFieldNoDefault( &m_geoMechDataSource, "GeoMechDataSource", "Geomechanical Data Source" );
    m_geoMechDataSource.uiCapability()->setUiTreeChildrenHidden( true );
    m_geoMechDataSource = new RimGeoMechGeometrySelectionItem;

    CAF_PDM_InitField( &m_plotAxis, "PlotAxis", caf::AppEnum<RiaDefines::PlotAxis>( RiaDefines::PlotAxis::PLOT_AXIS_LEFT ), "Axis" );

    CAF_PDM_InitFieldNoDefault( &m_geometrySelectionItem_OBSOLETE, "GeometrySelectionItem", "Geometry Selection" );
    m_geometrySelectionItem_OBSOLETE.uiCapability()->setUiTreeChildrenHidden( true );
    m_geometrySelectionItem_OBSOLETE.xmlCapability()->setIOWritable( false );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridTimeHistoryCurve::~RimGridTimeHistoryCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::setFromSelectionItem( const RiuSelectionItem* selectionItem, bool updateResultDefinition )
{
    if ( const RiuEclipseSelectionItem* eclSelectionItem = dynamic_cast<const RiuEclipseSelectionItem*>( selectionItem ) )
    {
        m_eclipseDataSource->setFromSelectionItem( eclSelectionItem );

        if ( updateResultDefinition && eclSelectionItem->m_resultDefinition )
        {
            m_eclipseResultDefinition->simpleCopy( eclSelectionItem->m_resultDefinition );
        }
    }

    if ( const RiuGeoMechSelectionItem* geoMechSelectionItem = dynamic_cast<const RiuGeoMechSelectionItem*>( selectionItem ) )
    {
        m_geoMechDataSource->setFromSelectionItem( geoMechSelectionItem );

        if ( updateResultDefinition && geoMechSelectionItem->m_resultDefinition )
        {
            m_geoMechResultDefinition->setGeoMechCase( geoMechSelectionItem->m_resultDefinition->geoMechCase() );
            m_geoMechResultDefinition->setResultAddress( geoMechSelectionItem->m_resultDefinition->resultAddress() );
        }
    }

    updateResultDefinitionFromCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::setFromEclipseCellAndResult( RimEclipseCase*                eclCase,
                                                           size_t                         gridIdx,
                                                           size_t                         i,
                                                           size_t                         j,
                                                           size_t                         k,
                                                           const RigEclipseResultAddress& resAddr )
{
    m_eclipseResultDefinition->setEclipseCase( eclCase );
    m_eclipseResultDefinition->setFromEclipseResultAddress( resAddr );

    m_eclipseDataSource->setFromCaseGridAndIJK( eclCase, gridIdx, i, j, k );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimGridTimeHistoryCurve::yAxis() const
{
    return RiuPlotAxis( m_plotAxis() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::setYAxis( RiaDefines::PlotAxis plotAxis )
{
    m_plotAxis = plotAxis;

    updateQwtPlotAxis();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGridTimeHistoryCurve::yValues() const
{
    std::vector<double> values;

    RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
    if ( eclTopItem && eclTopItem->eclipseCase() )
    {
        size_t cellIndex = eclTopItem->cellIndex();
        size_t gridIndex = eclTopItem->gridIndex();

        CVF_ASSERT( m_eclipseResultDefinition() );
        m_eclipseResultDefinition->loadResult();

        RigCaseCellResultsData* cellResultsData = m_eclipseResultDefinition->currentGridCellResults();

        if ( cellResultsData )
        {
            std::vector<QDateTime> timeStepDates = cellResultsData->timeStepDates();

            values = RigTimeHistoryResultAccessor::timeHistoryValues( eclTopItem->eclipseCase()->eclipseCaseData(),
                                                                      m_eclipseResultDefinition(),
                                                                      gridIndex,
                                                                      cellIndex,
                                                                      timeStepDates.size() );
        }
    }

    if ( geoMechGeomSelectionItem() && geoMechGeomSelectionItem()->geoMechCase() )
    {
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor = femTimeHistoryResultAccessor();

        if ( timeHistResultAccessor )
        {
            values = timeHistResultAccessor->timeHistoryValues();
        }
    }

    auto plot         = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
    bool isNormalized = plot->isNormalizationEnabled();
    if ( isNormalized )
    {
        auto   minMaxPair = std::minmax_element( values.begin(), values.end() );
        double min        = *minMaxPair.first;
        double max        = *minMaxPair.second;
        double range      = max - min;

        for ( double& v : values )
        {
            v = ( v - min ) / range;
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::quantityName() const
{
    RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
    if ( eclTopItem )
    {
        CVF_ASSERT( m_eclipseResultDefinition() );

        return m_eclipseResultDefinition->resultVariableUiName();
    }

    if ( geoMechGeomSelectionItem() )
    {
        CVF_ASSERT( m_geoMechResultDefinition() );

        RimGeoMechGeometrySelectionItem*                 geoMechTopItem         = geoMechGeomSelectionItem();
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor = femTimeHistoryResultAccessor();

        QString text;

        caf::AppEnum<RigFemResultPosEnum> resPosAppEnum = m_geoMechResultDefinition()->resultPositionType();
        text.append( resPosAppEnum.uiText() + ", " );
        text.append( m_geoMechResultDefinition()->resultFieldUiName() + ", " );
        text.append( m_geoMechResultDefinition()->resultComponentUiName() + " " );

        if ( resPosAppEnum == RIG_ELEMENT_NODAL_FACE )
        {
            if ( geoMechTopItem->m_elementFace >= 0 )
            {
                text.append( ", " + caf::AppEnum<cvf::StructGridInterface::FaceType>::textFromIndex( geoMechTopItem->m_elementFace ) );
            }
            else
            {
                text.append( ", from N[" + QString::number( timeHistResultAccessor->closestNodeId() ) + "] transformed onto intersection" );
            }
        }

        return text;
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::caseName() const
{
    RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
    if ( eclTopItem && eclTopItem->eclipseCase() )
    {
        return eclTopItem->eclipseCase()->caseUserDescription();
    }

    RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
    if ( geoMechTopItem && geoMechTopItem->geoMechCase() )
    {
        return geoMechTopItem->geoMechCase()->caseUserDescription();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimGridTimeHistoryCurve::gridCase() const
{
    RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
    if ( eclTopItem && eclTopItem->eclipseCase() )
    {
        return eclTopItem->eclipseCase();
    }

    RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
    if ( geoMechTopItem && geoMechTopItem->geoMechCase() )
    {
        return geoMechTopItem->geoMechCase();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::setLocked( bool locked )
{
    m_isLocked = locked;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridTimeHistoryCurve::isLocked() const
{
    return m_isLocked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::createCurveFromSelectionItem( const RiuSelectionItem* selectionItem, RimSummaryPlot* plot )
{
    if ( !selectionItem || !plot ) return;

    auto newCurve               = new RimGridTimeHistoryCurve();
    bool updateResultDefinition = true;
    newCurve->setFromSelectionItem( selectionItem, updateResultDefinition );
    newCurve->setLineThickness( 2 );

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable( plot->curveCount() );
    newCurve->setColor( curveColor );

    plot->addGridTimeHistoryCurve( newCurve );

    newCurve->loadDataAndUpdate( true );

    plot->updateConnectedEditors();

    RiuPlotMainWindowTools::showPlotMainWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( newCurve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PhaseType RimGridTimeHistoryCurve::phaseType() const
{
    if ( m_eclipseResultDefinition )
    {
        return m_eclipseResultDefinition->resultPhaseType();
    }

    return RiaDefines::PhaseType::PHASE_NOT_APPLICABLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::createCurveAutoName()
{
    QString text;

    text += quantityName();

    QString topoText = geometrySelectionText();

    if ( !topoText.isEmpty() )
    {
        text += ", ";
        text += topoText;
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::updateZoomInParentPlot()
{
    auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();

    plot->updatePlotWidgetFromAxisRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    RimPlotCurve::updateCurvePresentation( updateParentPlot );

    if ( isChecked() && m_plotCurve )
    {
        updateResultDefinitionFromCase();

        std::vector<double> values;

        RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
        if ( eclTopItem && eclTopItem->eclipseCase() )
        {
            m_eclipseResultDefinition->loadResult();
        }

        RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
        if ( geoMechTopItem && geoMechTopItem->geoMechCase() )
        {
            m_geoMechResultDefinition->loadResult();
        }

        values = yValues();

        auto plot                = firstAncestorOrThisOfType<RimSummaryPlot>();
        bool useLogarithmicScale = plot->isLogarithmicScaleEnabled( yAxis() );

        if ( plot->timeAxisProperties()->timeMode() == RimSummaryTimeAxisProperties::DATE )
        {
            std::vector<time_t> dateTimes = timeStepValues();
            if ( !dateTimes.empty() && dateTimes.size() == values.size() )
            {
                m_plotCurve->setSamplesFromTimeTAndYValues( dateTimes, values, useLogarithmicScale );
            }
            else
            {
                m_plotCurve->setSamplesFromTimeTAndYValues( std::vector<time_t>(), std::vector<double>(), useLogarithmicScale );
            }
        }
        else
        {
            std::vector<double> days = daysSinceSimulationStart();
            if ( !days.empty() && days.size() == values.size() )
            {
                double timeScale = plot->timeAxisProperties()->fromDaysToDisplayUnitScale();

                std::vector<double> times;
                for ( double day : days )
                {
                    times.push_back( timeScale * day );
                }

                m_plotCurve->setSamplesFromXValuesAndYValues( times, values, useLogarithmicScale );
            }
            else
            {
                m_plotCurve->setSamplesFromTimeTAndYValues( std::vector<time_t>(), std::vector<double>(), useLogarithmicScale );
            }
        }

        plot->zoomAll();
        if ( m_parentPlot ) m_parentPlot->replot();

        updateQwtPlotAxis();
        plot->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimGridTimeHistoryCurve::timeStepValues() const
{
    std::vector<time_t> dateTimes;

    RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
    if ( eclTopItem && eclTopItem->eclipseCase() )
    {
        RigCaseCellResultsData* cellResultsData = m_eclipseResultDefinition->currentGridCellResults();

        if ( cellResultsData )
        {
            std::vector<QDateTime> timeStepDates = cellResultsData->timeStepDates();

            for ( QDateTime dt : timeStepDates )
            {
                dateTimes.push_back( dt.toSecsSinceEpoch() );
            }
        }
    }

    RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
    if ( geoMechTopItem && geoMechTopItem->geoMechCase() )
    {
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor = femTimeHistoryResultAccessor();
        if ( timeHistResultAccessor )
        {
            std::vector<double> values = timeHistResultAccessor->timeHistoryValues();

            std::vector<QDateTime> dates = geoMechTopItem->geoMechCase()->timeStepDates();
            if ( dates.size() == values.size() )
            {
                for ( QDateTime dt : dates )
                {
                    dateTimes.push_back( dt.toSecsSinceEpoch() );
                }
            }
            else
            {
                for ( size_t i = 0; i < values.size(); i++ )
                {
                    dateTimes.push_back( i );
                }
            }
        }
    }

    return dateTimes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGridTimeHistoryCurve::daysSinceSimulationStart() const
{
    std::vector<double>              daysSinceSimulationStart;
    RimEclipseGeometrySelectionItem* eclTopItem = eclipseGeomSelectionItem();
    if ( eclTopItem && eclTopItem->eclipseCase() )
    {
        RigCaseCellResultsData* cellResultsData = m_eclipseResultDefinition->currentGridCellResults();

        if ( cellResultsData )
        {
            daysSinceSimulationStart = cellResultsData->daysSinceSimulationStart();
        }
    }

    RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
    if ( geoMechTopItem && geoMechTopItem->geoMechCase() )
    {
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor = femTimeHistoryResultAccessor();
        if ( timeHistResultAccessor )
        {
            std::vector<double> values = timeHistResultAccessor->timeHistoryValues();

            std::vector<QDateTime> dates = geoMechTopItem->geoMechCase()->timeStepDates();
            if ( dates.size() == values.size() )
            {
                if ( !dates.empty() )
                {
                    time_t startDate               = dates[0].toSecsSinceEpoch();
                    double secondsToDaysConversion = ( 24.0 * 60.0 * 60.0 );
                    for ( QDateTime dt : dates )
                    {
                        double timeDifference = static_cast<double>( dt.toSecsSinceEpoch() - startDate );
                        daysSinceSimulationStart.push_back( timeDifference / secondsToDaysConversion );
                    }
                }
            }
            else
            {
                for ( size_t i = 0; i < values.size(); i++ )
                {
                    daysSinceSimulationStart.push_back( i );
                }
            }
        }
    }

    return daysSinceSimulationStart;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    uiOrdering.add( &m_isLocked );

    caf::PdmUiGroup* dataSource = uiOrdering.addNewGroup( "Data Source" );
    dataSource->add( &m_geometrySelectionText );
    eclipseGeomSelectionItem()->uiOrdering( uiConfigName, *dataSource );

    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup( "Result" );
    if ( eclipseGeomSelectionItem() && m_eclipseResultDefinition->eclipseCase() )
    {
        m_eclipseResultDefinition->uiOrdering( uiConfigName, *group1 );
    }

    if ( geoMechGeomSelectionItem() && m_geoMechResultDefinition->geoMechCase() )
    {
        m_geoMechResultDefinition->uiOrdering( uiConfigName, *group1 );
    }

    uiOrdering.add( &m_plotAxis );

    caf::PdmUiGroup* stackingGroup = uiOrdering.addNewGroup( "Stacking" );
    RimStackablePlotCurve::stackingUiOrdering( *stackingGroup );

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );
    appearanceGroup->setCollapsedByDefault();

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->setCollapsedByDefault();
    nameGroup->add( &m_showLegend );
    RimPlotCurve::curveNameUiOrdering( *nameGroup );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::initAfterRead()
{
    RimPlotCurve::initAfterRead();

    // Convert old geometry selection item to new data source
    // This change was introduced in 2025.03
    // When this class is removed, the inheritance of RimGeometrySelectionItem_OBSOLETE should be removed as well
    // The project files will then work as expected
    RimGeometrySelectionItem_OBSOLETE* obj = m_geometrySelectionItem_OBSOLETE();
    if ( auto eclipseObject = dynamic_cast<RimEclipseGeometrySelectionItem*>( obj ) )
    {
        m_eclipseDataSource->setFromSelectionItem( eclipseObject );
    }
    else if ( auto geoMechObject = dynamic_cast<RimGeoMechGeometrySelectionItem*>( obj ) )
    {
        m_geoMechDataSource->setFromSelectionItem( geoMechObject );
    }

    updateResultDefinitionFromCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimStackablePlotCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_plotAxis )
    {
        updateQwtPlotAxis();

        auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();

        plot->updateAxes();
    }
    else
    {
        RimStackablePlotCurve::fieldChangedByUi( changedField, oldValue, newValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    onLoadDataAndUpdate( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    bool isUpdatedByAutomation = false;

    if ( auto parentPlot = RiaSummaryTools::parentSummaryPlot( this ) )
    {
        for ( auto plot : RimTools::automationSettings()->summaryPlots() )
        {
            if ( parentPlot == plot ) isUpdatedByAutomation = true;
        }
    }

    // The padlock icon is only shown if the curve is updated by automation
    if ( isUpdatedByAutomation )
    {
        QString iconResourceString = m_isLocked ? ":/padlock.svg" : ":/padlock-unlocked.svg";

        if ( auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute ) )
        {
            auto tag  = caf::PdmUiTreeViewItemAttribute::createTag();
            tag->icon = caf::IconProvider( iconResourceString );

            tag->clicked.connect( this, &RimGridTimeHistoryCurve::onPadlockClicked );

            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimGridTimeHistoryCurve::mainGrid()
{
    if ( eclipseGeomSelectionItem() && eclipseGeomSelectionItem()->eclipseCase() &&
         eclipseGeomSelectionItem()->eclipseCase()->eclipseCaseData() )
    {
        return eclipseGeomSelectionItem()->eclipseCase()->eclipseCaseData()->mainGrid();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseGeometrySelectionItem* RimGridTimeHistoryCurve::eclipseGeomSelectionItem() const
{
    return m_eclipseDataSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechGeometrySelectionItem* RimGridTimeHistoryCurve::geoMechGeomSelectionItem() const
{
    return m_geoMechDataSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::updateResultDefinitionFromCase()
{
    if ( eclipseGeomSelectionItem() && m_eclipseResultDefinition() )
    {
        m_eclipseResultDefinition->setEclipseCase( eclipseGeomSelectionItem()->eclipseCase() );
    }

    if ( geoMechGeomSelectionItem() && m_geoMechResultDefinition() )
    {
        m_geoMechResultDefinition->setGeoMechCase( geoMechGeomSelectionItem()->geoMechCase() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridTimeHistoryCurve::geometrySelectionText() const
{
    QString text;

    if ( eclipseGeomSelectionItem() )
    {
        text = eclipseGeomSelectionItem()->geometrySelectionText();
    }
    else if ( geoMechGeomSelectionItem() )
    {
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor = femTimeHistoryResultAccessor();
        if ( timeHistResultAccessor )
        {
            text = timeHistResultAccessor->geometrySelectionText();
        }
    }
    else
    {
        text = "No Geometry";
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::updateQwtPlotAxis()
{
    if ( m_plotCurve ) updateYAxisInPlot( yAxis() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridTimeHistoryCurve::onPadlockClicked( const SignalEmitter* emitter, size_t index )
{
    m_isLocked = !m_isLocked();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RiuFemTimeHistoryResultAccessor> RimGridTimeHistoryCurve::femTimeHistoryResultAccessor() const
{
    std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor;

    if ( geoMechGeomSelectionItem() && geoMechGeomSelectionItem()->geoMechCase() && geoMechGeomSelectionItem()->geoMechCase()->geoMechData() )
    {
        RimGeoMechGeometrySelectionItem* geoMechTopItem = geoMechGeomSelectionItem();
        if ( geoMechTopItem->m_hasIntersectionTriangle )
        {
            std::array<cvf::Vec3f, 3> intersectionTriangle;
            intersectionTriangle[0] = cvf::Vec3f( geoMechTopItem->m_intersectionTriangle_0() );
            intersectionTriangle[1] = cvf::Vec3f( geoMechTopItem->m_intersectionTriangle_1() );
            intersectionTriangle[2] = cvf::Vec3f( geoMechTopItem->m_intersectionTriangle_2() );

            timeHistResultAccessor = std::make_unique<RiuFemTimeHistoryResultAccessor>( geoMechTopItem->geoMechCase()->geoMechData(),
                                                                                        m_geoMechResultDefinition()->resultAddress(),
                                                                                        geoMechTopItem->m_gridIndex,
                                                                                        static_cast<int>( geoMechTopItem->m_cellIndex ),
                                                                                        geoMechTopItem->m_elementFace,
                                                                                        geoMechTopItem->m_localIntersectionPoint,
                                                                                        intersectionTriangle );
        }
        else
        {
            timeHistResultAccessor = std::make_unique<RiuFemTimeHistoryResultAccessor>( geoMechTopItem->geoMechCase()->geoMechData(),
                                                                                        m_geoMechResultDefinition()->resultAddress(),
                                                                                        geoMechTopItem->m_gridIndex,
                                                                                        static_cast<int>( geoMechTopItem->m_cellIndex ),
                                                                                        geoMechTopItem->m_elementFace,
                                                                                        geoMechTopItem->m_localIntersectionPoint );
        }
    }

    return timeHistResultAccessor;
}
