/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RimSummaryMultiPlot.h"

#include "RiaApplication.h"
#include "RiaPlotDefines.h"
#include "RiaSummaryAddressAnalyzer.h"
#include "RiaSummaryStringTools.h"

#include "PlotBuilderCommands/RicAppendSummaryPlotsForObjectsFeature.h"
#include "PlotBuilderCommands/RicAppendSummaryPlotsForSummaryAddressesFeature.h"
#include "PlotBuilderCommands/RicAppendSummaryPlotsForSummaryCasesFeature.h"
#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"

#include "RifEclEclipseSummary.h"
#include "RifEclipseRftAddress.h"
#include "RifEclipseSummaryAddress.h"

#include "RimEnsembleCurveSet.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlotCollection.h"
#include "RimMultipleSummaryPlotNameHelper.h"
#include "RimPlotAxisProperties.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryAddressModifier.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotControls.h"
#include "RimSummaryPlotNameHelper.h"
#include "RimSummaryPlotSourceStepping.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuSummaryMultiPlotBook.h"
#include "RiuSummaryVectorSelectionUi.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include "qwt_scale_engine.h"

#include <QKeyEvent>

#include <cmath>

namespace caf
{
template <>
void AppEnum<RimSummaryMultiPlot::AxisRangeAggregation>::setUp()
{
    addItem( RimSummaryMultiPlot::AxisRangeAggregation::INDIVIDUAL, "INDIVIDUAL", "Individual" );
    addItem( RimSummaryMultiPlot::AxisRangeAggregation::SUB_PLOTS, "SUB_PLOTS", "All Sub Plots" );
    addItem( RimSummaryMultiPlot::AxisRangeAggregation::WELLS, "WELLS", "All Wells" );
    addItem( RimSummaryMultiPlot::AxisRangeAggregation::REGIONS, "REGIONS", "All Regions" );
    addItem( RimSummaryMultiPlot::AxisRangeAggregation::REALIZATIONS, "REALIZATIONS", "All Realizations" );
    setDefault( RimSummaryMultiPlot::AxisRangeAggregation::INDIVIDUAL );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimSummaryMultiPlot, "MultiSummaryPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::setLayoutInfo( RimSummaryPlot* summaryPlot, int row, int col )
{
    m_gridLayoutInfo[summaryPlot] = std::make_pair( row, col );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::clearLayoutInfo()
{
    m_gridLayoutInfo.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot::RimSummaryMultiPlot()
    : duplicatePlot( this )
{
    CAF_PDM_InitObject( "Multi Summary Plot", ":/SummaryPlotLight16x16.png" );
    this->setDeletable( true );

    CAF_PDM_InitField( &m_autoPlotTitles, "AutoPlotTitles", true, "Auto Plot Titles" );
    CAF_PDM_InitField( &m_autoPlotTitlesOnSubPlots, "AutoPlotTitlesSubPlots", true, "Auto Plot Titles Sub Plots" );

    CAF_PDM_InitField( &m_createPlotDuplicate, "DuplicatePlot", false, "", "", "Duplicate Plot" );
    m_createPlotDuplicate.xmlCapability()->disableIO();
    m_createPlotDuplicate.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_createPlotDuplicate.uiCapability()->setUiIconFromResourceString( ":/Copy.svg" );

    CAF_PDM_InitField( &m_disableWheelZoom, "DisableWheelZoom", true, "", "", "Disable Mouse Wheel Zooming in Multi Summary Plot" );
    m_disableWheelZoom.xmlCapability()->disableIO();
    m_disableWheelZoom.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_disableWheelZoom.uiCapability()->setUiIconFromResourceString( ":/DisableZoom.png" );

    CAF_PDM_InitField( &m_appendNextPlot, "AppendNextPlot", false, "", "", "Step Next and Add to New Plot" );
    m_appendNextPlot.xmlCapability()->disableIO();
    m_appendNextPlot.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_appendNextPlot.uiCapability()->setUiIconFromResourceString( ":/AppendNext.png" );

    CAF_PDM_InitField( &m_appendPrevPlot, "AppendPrevPlot", false, "", "", "Step Previous and Add to New Plot" );
    m_appendPrevPlot.xmlCapability()->disableIO();
    m_appendPrevPlot.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_appendPrevPlot.uiCapability()->setUiIconFromResourceString( ":/AppendPrev.png" );

    CAF_PDM_InitField( &m_appendNextCurve, "AppendNextCurve", false, "", "", "Step Next and Add Curve to Plot" );
    m_appendNextCurve.xmlCapability()->disableIO();
    m_appendNextCurve.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_appendNextCurve.uiCapability()->setUiIconFromResourceString( ":/AppendNextCurve.png" );

    CAF_PDM_InitField( &m_appendPrevCurve, "AppendPrevCurve", false, "", "", "Step Previous and Add Curve to Plot" );
    m_appendPrevCurve.xmlCapability()->disableIO();
    m_appendPrevCurve.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_appendPrevCurve.uiCapability()->setUiIconFromResourceString( ":/AppendPrevCurve.png" );

    CAF_PDM_InitField( &m_linkSubPlotAxes, "LinkSubPlotAxes", true, "Link Sub Plot Axes" );
    CAF_PDM_InitField( &m_autoAdjustAppearance, "AutoAdjustAppearance", true, "Auto Adjust Appearance" );

    CAF_PDM_InitFieldNoDefault( &m_axisRangeAggregation, "AxisRangeAggregation", "Axis Range Aggregation" );

    CAF_PDM_InitFieldNoDefault( &m_sourceStepping, "SourceStepping", "" );

    m_sourceStepping = new RimSummaryPlotSourceStepping;
    m_sourceStepping->setSourceSteppingType( RimSummaryDataSourceStepping::Axis::Y_AXIS );
    m_sourceStepping->setSourceSteppingObject( this );
    m_sourceStepping.uiCapability()->setUiTreeHidden( true );
    m_sourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_sourceStepping.xmlCapability()->disableIO();

    m_nameHelper = std::make_unique<RimSummaryPlotNameHelper>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot::~RimSummaryMultiPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::addPlot( RimPlot* plot )
{
    auto* sumPlot = dynamic_cast<RimSummaryPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        RimMultiPlot::addPlot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::insertPlot( RimPlot* plot, size_t index )
{
    auto* sumPlot = dynamic_cast<RimSummaryPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        sumPlot->axisChanged.connect( this, &RimSummaryMultiPlot::onSubPlotAxisChanged );
        sumPlot->curvesChanged.connect( this, &RimSummaryMultiPlot::onSubPlotChanged );
        RimMultiPlot::insertPlot( plot, index );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects )
{
    if ( objects.empty() ) return;

    std::vector<RimSummaryAddress*>           addresses;
    std::vector<RimSummaryAddressCollection*> addressCollections;
    std::vector<RimSummaryCase*>              cases;

    for ( auto o : objects )
    {
        auto address = dynamic_cast<RimSummaryAddress*>( o );
        if ( address ) addresses.push_back( address );

        auto adrColl = dynamic_cast<RimSummaryAddressCollection*>( o );
        if ( adrColl ) addressCollections.push_back( adrColl );

        auto summaryCase = dynamic_cast<RimSummaryCase*>( o );
        if ( summaryCase ) cases.push_back( summaryCase );
    }

    RicAppendSummaryPlotsForSummaryAddressesFeature::appendPlotsForAddresses( this, addresses );
    RicAppendSummaryPlotsForObjectsFeature::appendPlots( this, addressCollections );
    RicAppendSummaryPlotsForSummaryCasesFeature::appendPlotsForCases( this, cases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::removePlot( RimPlot* plot )
{
    auto* sumPlot = dynamic_cast<RimSummaryPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        RimMultiPlot::removePlot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::removePlotNoUpdate( RimPlot* plot )
{
    auto* sumPlot = dynamic_cast<RimSummaryPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        RimMultiPlot::removePlotNoUpdate( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updateAfterPlotRemove()
{
    onPlotAdditionOrRemoval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryDataSourceStepping::Axis> RimSummaryMultiPlot::availableAxes() const
{
    return { RimSummaryDataSourceStepping::Axis::X_AXIS };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryMultiPlot::curvesForStepping( RimSummaryDataSourceStepping::Axis axis ) const
{
    std::vector<RimSummaryCurve*> curves;

    for ( auto summaryPlot : summaryPlots() )
    {
        for ( auto curve : summaryPlot->curvesForStepping( axis ) )
        {
            curves.push_back( curve );
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleCurveSet*> RimSummaryMultiPlot::curveSets() const
{
    std::vector<RimEnsembleCurveSet*> curveSets;

    for ( auto summaryPlot : summaryPlots() )
    {
        for ( auto curveSet : summaryPlot->curveSets() )
        {
            curveSets.push_back( curveSet );
        }
    }

    return curveSets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryMultiPlot::allCurves( RimSummaryDataSourceStepping::Axis axis ) const
{
    std::vector<RimSummaryCurve*> curves;

    for ( auto summaryPlot : summaryPlots() )
    {
        for ( auto curve : summaryPlot->allCurves( axis ) )
        {
            curves.push_back( curve );
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::populateNameHelper( RimSummaryPlotNameHelper* nameHelper )
{
    nameHelper->clear();

    std::vector<RifEclipseSummaryAddress>  addresses;
    std::vector<RimSummaryCase*>           sumCases;
    std::vector<RimSummaryCaseCollection*> ensembleCases;

    for ( RimSummaryCurve* curve : allCurves( RimSummaryDataSourceStepping::Axis::Y_AXIS ) )
    {
        addresses.push_back( curve->summaryAddressY() );
        sumCases.push_back( curve->summaryCaseY() );
    }

    for ( auto curveSet : curveSets() )
    {
        addresses.push_back( curveSet->summaryAddress() );
        ensembleCases.push_back( curveSet->summaryCaseCollection() );
    }

    nameHelper->appendAddresses( addresses );
    nameHelper->setSummaryCases( sumCases );
    nameHelper->setEnsembleCases( ensembleCases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto axesGroup = uiOrdering.addNewGroup( "Axes" );
    axesGroup->add( &m_axisRangeAggregation );
    axesGroup->add( &m_linkSubPlotAxes );
    axesGroup->add( &m_autoAdjustAppearance );

    m_linkSubPlotAxes.uiCapability()->setUiReadOnly( m_autoAdjustAppearance() );
    if ( m_autoAdjustAppearance() ) m_linkSubPlotAxes = false;

    auto dataSourceGroup = uiOrdering.addNewGroup( "Data Source" );
    m_sourceStepping()->uiOrdering( uiConfigName, *dataSourceGroup );

    auto titlesGroup = uiOrdering.addNewGroup( "Main Plot Settings" );
    titlesGroup->setCollapsedByDefault( true );
    titlesGroup->add( &m_autoPlotTitles );
    titlesGroup->add( &m_showPlotWindowTitle );
    titlesGroup->add( &m_plotWindowTitle );
    titlesGroup->add( &m_titleFontSize );

    auto subPlotSettingsGroup = uiOrdering.addNewGroup( "Sub Plot Settings" );
    subPlotSettingsGroup->setCollapsedByDefault( true );
    subPlotSettingsGroup->add( &m_autoPlotTitlesOnSubPlots );
    subPlotSettingsGroup->add( &m_showIndividualPlotTitles );
    subPlotSettingsGroup->add( &m_subTitleFontSize );

    auto legendsGroup = uiOrdering.addNewGroup( "Legends" );
    legendsGroup->setCollapsedByDefault( true );
    legendsGroup->add( &m_showPlotLegends );
    legendsGroup->add( &m_plotLegendsHorizontal );
    legendsGroup->add( &m_legendFontSize );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
    if ( changedField == &m_autoPlotTitles || changedField == &m_autoPlotTitlesOnSubPlots )
    {
        onLoadDataAndUpdate();
        updateLayout();
    }
    else if ( changedField == &m_linkSubPlotAxes || changedField == &m_axisRangeAggregation )
    {
        syncAxisRanges();
    }
    else if ( changedField == &m_createPlotDuplicate )
    {
        m_createPlotDuplicate = false;
        duplicate();
    }
    else if ( changedField == &m_appendNextPlot )
    {
        m_appendNextPlot  = false;
        int stepDirection = 1;
        appendSubPlotByStepping( stepDirection );
    }
    else if ( changedField == &m_appendPrevPlot )
    {
        m_appendPrevPlot  = false;
        int stepDirection = -1;
        appendSubPlotByStepping( stepDirection );
    }
    else if ( changedField == &m_appendNextCurve )
    {
        m_appendNextCurve = false;
        int stepDirection = 1;
        appendCurveByStepping( stepDirection );
    }
    else if ( changedField == &m_appendPrevCurve )
    {
        m_appendPrevCurve = false;
        int stepDirection = -1;
        appendCurveByStepping( stepDirection );
    }
    else if ( changedField == &m_autoAdjustAppearance )
    {
        checkAndApplyAutoAppearance();
    }
    else
    {
        RimMultiPlot::fieldChangedByUi( changedField, oldValue, newValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updatePlotWindowTitle()
{
    if ( m_autoPlotTitles )
    {
        populateNameHelper( m_nameHelper.get() );

        auto title = m_nameHelper->plotTitle();
        if ( title.isEmpty() ) title = "Empty Plot";
        setMultiPlotTitle( title );
    }

    if ( m_autoPlotTitlesOnSubPlots )
    {
        for ( auto plot : summaryPlots() )
        {
            auto subPlotNameHelper = plot->plotTitleHelper();

            // Disable auto plot title, as this is required to be able to include the information in the multi plot title
            plot->enableAutoPlotTitle( false );

            auto plotName = subPlotNameHelper->aggregatedPlotTitle( *m_nameHelper );
            plot->setPlotTitleVisible( true );
            plot->setDescription( plotName );
            plot->updatePlotTitle();
        }

        if ( !m_viewer.isNull() ) m_viewer->scheduleTitleUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimSummaryNameHelper* RimSummaryMultiPlot::nameHelper() const
{
    return m_nameHelper.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::setAutoTitlePlot( bool enable )
{
    m_autoPlotTitles = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::setAutoTitleGraphs( bool enable )
{
    m_autoPlotTitlesOnSubPlots = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RimSummaryMultiPlot::summaryPlots() const
{
    std::vector<RimSummaryPlot*> typedPlots;

    for ( auto plot : plots() )
    {
        auto summaryPlot = dynamic_cast<RimSummaryPlot*>( plot );
        if ( summaryPlot ) typedPlots.push_back( summaryPlot );
    }

    return typedPlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RimSummaryMultiPlot::visibleSummaryPlots() const
{
    std::vector<RimSummaryPlot*> visiblePlots;

    for ( auto plot : summaryPlots() )
    {
        if ( plot->showWindow() ) visiblePlots.push_back( plot );
    }

    return visiblePlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryMultiPlot::fieldsToShowInToolbar()
{
    std::vector<caf::PdmFieldHandle*> toolBarFields;

    toolBarFields.push_back( &m_disableWheelZoom );
    toolBarFields.push_back( &m_createPlotDuplicate );

    auto& sourceObject = m_sourceStepping();
    if ( sourceObject )
    {
        auto fields = sourceObject->fieldsToShowInToolbar();
        toolBarFields.insert( std::end( toolBarFields ), std::begin( fields ), std::end( fields ) );
    }

    toolBarFields.push_back( &m_appendPrevPlot );
    toolBarFields.push_back( &m_appendNextPlot );

    toolBarFields.push_back( &m_appendPrevCurve );
    toolBarFields.push_back( &m_appendNextCurve );

    auto multiFields = RimMultiPlot::fieldsToShowInToolbar();
    toolBarFields.insert( std::end( toolBarFields ), std::begin( multiFields ), std::end( multiFields ) );

    return toolBarFields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryMultiPlot::handleGlobalKeyEvent( QKeyEvent* keyEvent )
{
    if ( !RimSummaryPlotControls::handleKeyEvents( m_sourceStepping(), keyEvent ) )
    {
        if ( isMouseCursorInsidePlot() )
        {
            if ( keyEvent->key() == Qt::Key_PageUp )
            {
                m_viewer->goToPrevPage();
                return true;
            }
            else if ( keyEvent->key() == Qt::Key_PageDown )
            {
                m_viewer->goToNextPage();
                return true;
            }
        }
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryMultiPlot::handleGlobalWheelEvent( QWheelEvent* wheelEvent )
{
    if ( m_disableWheelZoom )
    {
        if ( isMouseCursorInsidePlot() )
        {
            if ( wheelEvent->angleDelta().y() > 0 )
            {
                m_viewer->goToPrevPage();
            }
            else if ( wheelEvent->angleDelta().y() < 0 )
            {
                m_viewer->goToNextPage();
            }

            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::initAfterRead()
{
    RimMultiPlot::initAfterRead();

    for ( auto plot : summaryPlots() )
    {
        plot->axisChanged.connect( this, &RimSummaryMultiPlot::onSubPlotAxisChanged );
        plot->curvesChanged.connect( this, &RimSummaryMultiPlot::onSubPlotChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::onLoadDataAndUpdate()
{
    RimMultiPlot::onLoadDataAndUpdate();
    updatePlotWindowTitle();

    checkAndApplyAutoAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::zoomAll()
{
    syncAxisRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::setDefaultRangeAggregationSteppingDimension()
{
    RiaSummaryAddressAnalyzer analyzer;

    for ( auto p : summaryPlots() )
    {
        auto addresses = RimSummaryAddressModifier::createEclipseSummaryAddress( p );
        analyzer.appendAddresses( addresses );
    }

    auto rangeAggregation = AxisRangeAggregation::SUB_PLOTS;

    if ( !analyzer.wellNames().empty() )
    {
        rangeAggregation = AxisRangeAggregation::WELLS;
    }
    else if ( !analyzer.groupNames().empty() )
    {
        rangeAggregation = AxisRangeAggregation::SUB_PLOTS;
    }
    else if ( !analyzer.regionNumbers().empty() )
    {
        rangeAggregation = AxisRangeAggregation::REGIONS;
    }
    else if ( !analyzer.aquifers().empty() )
    {
        rangeAggregation = AxisRangeAggregation::SUB_PLOTS;
    }
    else if ( !analyzer.blocks().empty() )
    {
        rangeAggregation = AxisRangeAggregation::SUB_PLOTS;
    }

    auto stepDimension = RimSummaryPlotSourceStepping::SourceSteppingDimension::VECTOR;
    if ( analyzer.wellNames().size() == 1 )
    {
        stepDimension = RimSummaryPlotSourceStepping::SourceSteppingDimension::WELL;
    }
    else if ( analyzer.groupNames().size() == 1 )
    {
        stepDimension = RimSummaryPlotSourceStepping::SourceSteppingDimension::GROUP;
    }
    else if ( analyzer.regionNumbers().size() == 1 )
    {
        stepDimension = RimSummaryPlotSourceStepping::SourceSteppingDimension::REGION;
    }
    else if ( analyzer.aquifers().size() == 1 )
    {
        stepDimension = RimSummaryPlotSourceStepping::SourceSteppingDimension::AQUIFER;
    }
    else if ( analyzer.blocks().size() == 1 )
    {
        stepDimension = RimSummaryPlotSourceStepping::SourceSteppingDimension::BLOCK;
    }

    m_axisRangeAggregation = rangeAggregation;
    m_sourceStepping->setStepDimension( stepDimension );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::checkAndApplyAutoAppearance()
{
    if ( m_autoAdjustAppearance )
    {
        analyzePlotsAndAdjustAppearanceSettings();
        syncAxisRanges();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::syncAxisRanges()
{
    // Reset zoom to make sure the complete range for min/max is available
    RimMultiPlot::zoomAll();

    if ( m_axisRangeAggregation() == AxisRangeAggregation::INDIVIDUAL )
    {
        return;
    }
    else if ( m_axisRangeAggregation() == AxisRangeAggregation::SUB_PLOTS )
    {
        std::map<RiuPlotAxis, std::pair<double, double>> axisRanges;

        // gather current min/max values for each category (axis label)
        for ( auto plot : summaryPlots() )
        {
            for ( auto axis : plot->plotAxes() )
            {
                double minVal = axis->visibleRangeMin();
                double maxVal = axis->visibleRangeMax();

                if ( axisRanges.count( axis->plotAxisType() ) == 0 )
                {
                    axisRanges[axis->plotAxisType()] = std::make_pair( axis->visibleRangeMin(), axis->visibleRangeMax() );
                }
                else
                {
                    auto& [currentMin, currentMax] = axisRanges[axis->plotAxisType()];
                    axisRanges[axis->plotAxisType()] =
                        std::make_pair( std::min( currentMin, minVal ), std::max( currentMax, maxVal ) );
                }
            }
        }

        // set all plots to use the global min/max values for each category
        for ( auto plot : summaryPlots() )
        {
            for ( auto axis : plot->plotAxes() )
            {
                const auto& [minVal, maxVal] = axisRanges[axis->plotAxisType()];
                axis->setAutoZoom( false );
                axis->setVisibleRangeMin( minVal );
                axis->setVisibleRangeMax( maxVal );
            }

            plot->updateAxes();
        }
    }
    else
    {
        computeAggregatedAxisRange();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::computeAggregatedAxisRange()
{
    auto readValues = []( RimSummaryCase* summaryCase, RifEclipseSummaryAddress addr ) {
        std::vector<double> values;
        if ( summaryCase && summaryCase->summaryReader() )
        {
            RifSummaryReaderInterface* reader = summaryCase->summaryReader();
            reader->values( addr, &values );
        }

        return values;
    };

    auto findMinMaxForSummaryCase = [readValues]( RimSummaryCase* summaryCase, RifEclipseSummaryAddress addr ) {
        auto values = readValues( summaryCase, addr );
        if ( values.empty() ) return std::make_pair( HUGE_VAL, -HUGE_VAL );

        auto   minMaxPair  = std::minmax_element( values.begin(), values.end() );
        double caseMinimum = *minMaxPair.first;
        double caseMaximum = *minMaxPair.second;

        return std::make_pair( caseMinimum, caseMaximum );
    };

    auto summaryCasesForCurve = []( RimSummaryCurve* curve, AxisRangeAggregation axisRangeAggregation ) {
        std::vector<RimSummaryCase*> summaryCases;

        if ( axisRangeAggregation == AxisRangeAggregation::REALIZATIONS )
        {
            if ( curve->summaryCaseY() )
            {
                auto ensemble = curve->summaryCaseY()->ensemble();
                if ( ensemble )
                {
                    summaryCases = ensemble->allSummaryCases();
                }
                else
                {
                    summaryCases.push_back( curve->summaryCaseY() );
                }
            }
        }
        else if ( axisRangeAggregation == AxisRangeAggregation::WELLS ||
                  axisRangeAggregation == AxisRangeAggregation::REGIONS )
        {
            // Use only the current summary case when aggregation across wells/regions
            summaryCases.push_back( curve->summaryCaseY() );
        }

        return summaryCases;
    };

    auto addressesForCurve = []( RimSummaryCurve* curve, AxisRangeAggregation axisRangeAggregation ) {
        std::vector<RifEclipseSummaryAddress> addresses;

        if ( axisRangeAggregation == AxisRangeAggregation::REALIZATIONS )
        {
            RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fieldAddress( curve->summaryAddressY().vectorName() );
            addresses                     = { addr };
        }
        else if ( axisRangeAggregation == AxisRangeAggregation::WELLS ||
                  axisRangeAggregation == AxisRangeAggregation::REGIONS )
        {
            RiaSummaryAddressAnalyzer analyzer;
            auto                      ensemble = curve->summaryCaseY()->ensemble();
            if ( ensemble )
            {
                analyzer.appendAddresses( ensemble->ensembleSummaryAddresses() );
            }
            else
            {
                analyzer.appendAddresses( curve->summaryCaseY()->summaryReader()->allResultAddresses() );
            }

            if ( axisRangeAggregation == AxisRangeAggregation::WELLS )
            {
                for ( const auto& wellName : analyzer.wellNames() )
                {
                    addresses.push_back(
                        RifEclipseSummaryAddress::wellAddress( curve->summaryAddressY().vectorName(), wellName ) );
                }
            }

            if ( axisRangeAggregation == AxisRangeAggregation::REGIONS )
            {
                for ( auto regionNumber : analyzer.regionNumbers() )
                {
                    addresses.push_back( RifEclipseSummaryAddress::regionAddress( curve->summaryAddressY().vectorName(),
                                                                                  regionNumber ) );
                }
            }
        }

        return addresses;
    };

    auto findMinMaxForAddressesInSummaryCases =
        [findMinMaxForSummaryCase]( const std::vector<RifEclipseSummaryAddress>& addresses,
                                    const std::vector<RimSummaryCase*>&          summaryCases ) {
            double minimum = HUGE_VAL;
            double maximum = -HUGE_VAL;
            for ( auto summaryCase : summaryCases )
            {
                for ( const auto& addr : addresses )
                {
                    auto [caseMinimum, caseMaximum] = findMinMaxForSummaryCase( summaryCase, addr );
                    minimum                         = std::min( minimum, caseMinimum );
                    maximum                         = std::max( maximum, caseMaximum );
                }
            }

            return std::make_pair( minimum, maximum );
        };

    // gather current min/max values for each category (axis label)
    for ( auto plot : summaryPlots() )
    {
        std::map<RiuPlotAxis, std::pair<double, double>> axisRanges;

        for ( auto axis : plot->plotAxes() )
        {
            for ( auto curve : plot->summaryAndEnsembleCurves() )
            {
                if ( curve->axisY() == axis->plotAxisType() )
                {
                    std::vector<RimSummaryCase*> summaryCases = summaryCasesForCurve( curve, m_axisRangeAggregation() );
                    std::vector<RifEclipseSummaryAddress> addresses = addressesForCurve( curve, m_axisRangeAggregation() );

                    auto [minimum, maximum] = findMinMaxForAddressesInSummaryCases( addresses, summaryCases );

                    if ( axisRanges.count( axis->plotAxisType() ) == 0 )
                    {
                        axisRanges[axis->plotAxisType()] = std::make_pair( minimum, maximum );
                    }
                    else
                    {
                        auto& [currentMin, currentMax] = axisRanges[axis->plotAxisType()];
                        axisRanges[axis->plotAxisType()] =
                            std::make_pair( std::min( currentMin, minimum ), std::max( currentMax, maximum ) );
                    }
                }
            }
        }

        // set all plots to use the global min/max values for each category
        for ( auto axis : plot->plotAxes() )
        {
            QwtLinearScaleEngine scaleEngine;

            const auto& [minVal, maxVal] = axisRanges[axis->plotAxisType()];
            if ( axis->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT && minVal <= maxVal )
            {
                axis->setAutoZoom( false );

                auto adjustedMin = minVal;
                auto adjustedMax = maxVal;

                if ( !axis->isLogarithmicScaleEnabled() )
                {
                    int    maxMajorTickIntervalCount = 8;
                    double stepSize                  = 0.0;
                    scaleEngine.autoScale( maxMajorTickIntervalCount, adjustedMin, adjustedMax, stepSize );
                }

                axis->setVisibleRangeMin( adjustedMin );
                axis->setVisibleRangeMax( adjustedMax );
            }
        }

        plot->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::summaryPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const
{
    for ( RimSummaryPlot* plot : summaryPlots() )
    {
        QString displayName = plot->description();
        optionInfos->push_back( caf::PdmOptionItemInfo( displayName, plot, false, plot->uiCapability()->uiIconProvider() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::duplicate()
{
    duplicatePlot.send( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::analyzePlotsAndAdjustAppearanceSettings()
{
    RiaSummaryAddressAnalyzer analyzer;

    for ( auto p : summaryPlots() )
    {
        auto addresses = RimSummaryAddressModifier::createEclipseSummaryAddress( p );
        analyzer.appendAddresses( addresses );
    }

    bool hasOnlyOneQuantity = analyzer.quantities().size() == 1;

    for ( auto p : summaryPlots() )
    {
        auto timeAxisProp = p->timeAxisProperties();

        if ( columnCount() < 3 )
            timeAxisProp->setMajorTickmarkCount( RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_DEFAULT );
        else
            timeAxisProp->setMajorTickmarkCount( RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_FEW );

        for ( RimPlotAxisPropertiesInterface* axisInterface : p->plotAxes() )
        {
            auto axisProp = dynamic_cast<RimPlotAxisProperties*>( axisInterface );

            if ( !axisProp ) continue;

            if ( rowsPerPage() == 1 )
                axisProp->setMajorTickmarkCount( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_DEFAULT );
            else
                axisProp->setMajorTickmarkCount( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_FEW );

            axisProp->computeAndSetScaleFactor();

            if ( hasOnlyOneQuantity )
            {
                // Disable sub plot linking to be able to configure individually
                setSubPlotAxesLinked( false );

                axisProp->setShowDescription( false );

                auto [row, col] = gridLayoutInfoForSubPlot( p );
                if ( col == 0 )
                    axisProp->setShowUnitText( true );
                else
                    axisProp->setShowUnitText( false );
            }
        }

        p->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::makeSureIsVisible( RimSummaryPlot* summaryPlot )
{
    if ( summaryPlot->plotWidget() && !m_viewer.isNull() ) m_viewer->scrollToPlot( summaryPlot->plotWidget() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::setSubPlotAxesLinked( bool enable )
{
    m_linkSubPlotAxes = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryMultiPlot::isSubPlotAxesLinked() const
{
    return m_linkSubPlotAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RimSummaryMultiPlot::gridLayoutInfoForSubPlot( RimSummaryPlot* summaryPlot ) const
{
    auto it = m_gridLayoutInfo.find( summaryPlot );
    if ( it != m_gridLayoutInfo.end() ) return it->second;

    return { -1, -1 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::onSubPlotChanged( const caf::SignalEmitter* emitter )
{
    updatePlotWindowTitle();
    applyPlotWindowTitleToWidgets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::onSubPlotAxisChanged( const caf::SignalEmitter* emitter, RimSummaryPlot* summaryPlot )
{
    if ( !m_linkSubPlotAxes() ) return;

    for ( auto plot : summaryPlots() )
    {
        if ( plot != summaryPlot )
        {
            plot->copyMatchingAxisPropertiesFromOther( *summaryPlot );
            plot->updateAll();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimSummaryMultiPlot::createViewWidget( QWidget* mainWindowParent )
{
    if ( m_viewer.isNull() )
    {
        m_viewer = new RiuSummaryMultiPlotBook( this, mainWindowParent );
    }
    recreatePlotWidgets();

    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::appendSubPlotByStepping( int direction )
{
    if ( summaryPlots().empty() ) return;

    auto newPlots = RicSummaryPlotBuilder::duplicatePlots( { summaryPlots().back() } );
    if ( newPlots.empty() ) return;

    RimSummaryPlot* newPlot = dynamic_cast<RimSummaryPlot*>( newPlots[0] );
    if ( newPlot == nullptr ) return;

    if ( m_sourceStepping()->stepDimension() == RimSummaryPlotSourceStepping::SourceSteppingDimension::SUMMARY_CASE )
    {
        newPlot->resolveReferencesRecursively();

        RimSummaryCase* newCase = m_sourceStepping()->stepCase( direction );
        for ( auto curve : newPlot->allCurves( RimSummaryDataSourceStepping::Axis::Y_AXIS ) )
        {
            curve->setSummaryCaseX( newCase );
            curve->setSummaryCaseY( newCase );
        }
    }
    else if ( m_sourceStepping()->stepDimension() == RimSummaryPlotSourceStepping::SourceSteppingDimension::ENSEMBLE )
    {
        newPlot->resolveReferencesRecursively();

        RimSummaryCaseCollection* newEnsemble = m_sourceStepping()->stepEnsemble( direction );
        for ( auto curveSet : newPlot->curveSets() )
        {
            curveSet->setSummaryCaseCollection( newEnsemble );
        }
    }
    else
    {
        auto mods = RimSummaryAddressModifier::createAddressModifiersForPlot( newPlot );
        for ( auto& mod : mods )
        {
            auto modifiedAdr = m_sourceStepping()->stepAddress( mod.address(), direction );
            mod.setAddress( modifiedAdr );
        }
    }

    addPlot( newPlot );

    newPlot->resolveReferencesRecursively();
    newPlot->loadDataAndUpdate();

    updatePlotWindowTitle();
    updateConnectedEditors();

    RiuPlotMainWindowTools::selectAsCurrentItem( newPlot, true );

    updateSourceStepper();
    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::appendCurveByStepping( int direction )
{
    for ( auto plot : summaryPlots() )
    {
        std::vector<caf::PdmObjectHandle*> addresses;

        for ( auto curve : plot->allCurves( RimSummaryDataSourceStepping::Axis::Y_AXIS ) )
        {
            auto address = curve->summaryAddressY();
            auto sumCase = curve->summaryCaseY();
            address      = m_sourceStepping->stepAddress( address, direction );
            addresses.push_back( RimSummaryAddress::wrapFileReaderAddress( address, sumCase->caseId() ) );
        }

        for ( auto curveSet : plot->curveSets() )
        {
            auto address = curveSet->summaryAddress();
            auto sumEns  = curveSet->summaryCaseCollection();
            address      = m_sourceStepping->stepAddress( address, direction );
            addresses.push_back( RimSummaryAddress::wrapFileReaderAddress( address, -1, sumEns->ensembleId() ) );
        }

        plot->handleDroppedObjects( addresses );

        for ( auto adr : addresses )
        {
            delete adr;
        }
    }

    m_sourceStepping->updateStepIndex( direction );

    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updateSourceStepper()
{
    if ( summaryPlots().empty() ) return;

    RimSummaryPlot* plot = summaryPlots().back();

    auto sourceStepper = plot->sourceStepper();
    if ( sourceStepper == nullptr ) return;

    m_sourceStepping->syncWithStepper( sourceStepper );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::keepVisiblePageAfterUpdate( bool keepPage )
{
    if ( !m_viewer ) return;

    if ( keepPage ) m_viewer->keepCurrentPageAfterUpdate();
}
