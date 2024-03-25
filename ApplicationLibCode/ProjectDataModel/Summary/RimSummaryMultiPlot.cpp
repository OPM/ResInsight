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
#include "RiaNumericalTools.h"
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
#include "RimSummaryAddress.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryAddressModifier.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotControls.h"
#include "RimSummaryPlotNameHelper.h"
#include "RimSummaryPlotSourceStepping.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuSummaryMultiPlotBook.h"
#include "RiuSummaryVectorSelectionUi.h"

#include "cafPdmUiCheckBoxEditor.h"
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
    addItem( RimSummaryMultiPlot::AxisRangeAggregation::NONE, "NONE", "Per Sub Plot" );
    addItem( RimSummaryMultiPlot::AxisRangeAggregation::SUB_PLOTS, "SUB_PLOTS", "All Sub Plots" );
    addItem( RimSummaryMultiPlot::AxisRangeAggregation::WELLS, "WELLS", "All Wells" );
    addItem( RimSummaryMultiPlot::AxisRangeAggregation::REGIONS, "REGIONS", "All Regions" );
    addItem( RimSummaryMultiPlot::AxisRangeAggregation::REALIZATIONS, "REALIZATIONS", "All Realizations" );
    setDefault( RimSummaryMultiPlot::AxisRangeAggregation::NONE );
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
    setDeletable( true );

    CAF_PDM_InitField( &m_autoPlotTitle, "AutoPlotTitle", true, "Auto Plot Title" );
    CAF_PDM_InitField( &m_autoSubPlotTitle, "AutoSubPlotTitle", true, "Auto Sub Plot Title" );

    CAF_PDM_InitField( &m_createPlotDuplicate, "DuplicatePlot", false, "", "", "Duplicate Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_createPlotDuplicate );
    m_createPlotDuplicate.uiCapability()->setUiIconFromResourceString( ":/Copy.svg" );

    CAF_PDM_InitField( &m_disableWheelZoom, "DisableWheelZoom", true, "", "", "Disable Mouse Wheel Zooming in Multi Summary Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_disableWheelZoom );
    m_disableWheelZoom.uiCapability()->setUiIconFromResourceString( ":/DisableZoom.png" );

    CAF_PDM_InitField( &m_appendNextPlot, "AppendNextPlot", false, "", "", "Step Next and Add to New Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_appendNextPlot );
    m_appendNextPlot.uiCapability()->setUiIconFromResourceString( ":/AppendNext.png" );

    CAF_PDM_InitField( &m_appendPrevPlot, "AppendPrevPlot", false, "", "", "Step Previous and Add to New Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_appendPrevPlot );
    m_appendPrevPlot.uiCapability()->setUiIconFromResourceString( ":/AppendPrev.png" );

    CAF_PDM_InitField( &m_appendNextCurve, "AppendNextCurve", false, "", "", "Step Next and Add Curve to Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_appendNextCurve );
    m_appendNextCurve.uiCapability()->setUiIconFromResourceString( ":/AppendNextCurve.png" );

    CAF_PDM_InitField( &m_appendPrevCurve, "AppendPrevCurve", false, "", "", "Step Previous and Add Curve to Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_appendPrevCurve );
    m_appendPrevCurve.uiCapability()->setUiIconFromResourceString( ":/AppendPrevCurve.png" );

    CAF_PDM_InitField( &m_linkSubPlotAxes, "LinkSubPlotAxes", false, "Link Y Axes" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_linkSubPlotAxes );
    CAF_PDM_InitField( &m_linkTimeAxis, "LinkTimeAxis", true, "Link Time Axis" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_linkTimeAxis );
    CAF_PDM_InitField( &m_autoAdjustAppearance, "AutoAdjustAppearance", true, "Auto Plot Settings" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_autoAdjustAppearance );
    CAF_PDM_InitField( &m_allow3DSelectionLink, "Allow3DSelectionLink", true, "Allow Well Selection from 3D View" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_allow3DSelectionLink );

    CAF_PDM_InitFieldNoDefault( &m_axisRangeAggregation, "AxisRangeAggregation", "Y Axis Range" );

    CAF_PDM_InitField( &m_hidePlotsWithValuesBelow, "HidePlotsWithValuesBelow", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_hidePlotsWithValuesBelow );

    CAF_PDM_InitField( &m_plotFilterYAxisThreshold, "PlotFilterYAxisThreshold", 0.0, "Y-Axis Filter Threshold" );

    CAF_PDM_InitFieldNoDefault( &m_sourceStepping, "SourceStepping", "" );

    m_sourceStepping = new RimSummaryPlotSourceStepping;
    m_sourceStepping->setSourceSteppingObject( this );
    m_sourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_sourceStepping.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_defaultStepDimension, "DefaultStepDimension", "Default Step Dimension" );
    m_defaultStepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR;
    m_defaultStepDimension.uiCapability()->setUiHidden( true );

    m_nameHelper = std::make_unique<RimSummaryPlotNameHelper>();

    setBottomMargin( 40 );
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
        sumPlot->plotZoomedByUser.connect( this, &RimSummaryMultiPlot::onSubPlotZoomed );
        sumPlot->titleChanged.connect( this, &RimSummaryMultiPlot::onSubPlotChanged );
        sumPlot->axisChangedReloadRequired.connect( this, &RimSummaryMultiPlot::onSubPlotAxisReloadRequired );
        sumPlot->autoTitleChanged.connect( this, &RimSummaryMultiPlot::onSubPlotAutoTitleChanged );

        bool isMinMaxOverridden = m_axisRangeAggregation() != AxisRangeAggregation::NONE;
        setAutoValueStatesForPlot( sumPlot, isMinMaxOverridden, m_autoAdjustAppearance() );

        auto plots = summaryPlots();
        if ( !plots.empty() && m_linkTimeAxis )
        {
            sumPlot->copyAxisPropertiesFromOther( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, *plots.front() );
        }

        RimMultiPlot::insertPlot( plot, index );
    }

    if ( summaryPlots().size() == 1 ) m_disableWheelZoom = false;
    if ( summaryPlots().size() == 2 ) m_disableWheelZoom = true;
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
    std::vector<RimSummaryCaseCollection*>    ensembles;

    for ( auto o : objects )
    {
        auto address = dynamic_cast<RimSummaryAddress*>( o );
        if ( address ) addresses.push_back( address );

        auto adrColl = dynamic_cast<RimSummaryAddressCollection*>( o );
        if ( adrColl )
        {
            if ( objects.size() == 1 && adrColl->isFolder() )
            {
                // If a folder is selected, return all sub items in folder
                auto childObjects = adrColl->subFolders();
                addressCollections.insert( addressCollections.end(), childObjects.begin(), childObjects.end() );
            }
            else
                addressCollections.push_back( adrColl );
        }

        auto summaryCase = dynamic_cast<RimSummaryCase*>( o );
        if ( summaryCase ) cases.push_back( summaryCase );
        auto ensemble = dynamic_cast<RimSummaryCaseCollection*>( o );
        if ( ensemble ) ensembles.push_back( ensemble );
    }

    RicAppendSummaryPlotsForSummaryAddressesFeature::appendPlotsForAddresses( this, addresses );
    RicAppendSummaryPlotsForObjectsFeature::appendPlots( this, addressCollections );
    RicAppendSummaryPlotsForObjectsFeature::appendPlots( this, cases, ensembles );
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
std::vector<RimSummaryCurve*> RimSummaryMultiPlot::curvesForStepping() const
{
    std::vector<RimSummaryCurve*> curves;

    for ( auto summaryPlot : summaryPlots() )
    {
        for ( auto curve : summaryPlot->curvesForStepping() )
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
std::vector<RimSummaryCurve*> RimSummaryMultiPlot::allCurves() const
{
    std::vector<RimSummaryCurve*> curves;

    for ( auto summaryPlot : summaryPlots() )
    {
        for ( auto curve : summaryPlot->allCurves() )
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

    std::vector<RiaSummaryCurveAddress>    addresses;
    std::vector<RimSummaryCase*>           sumCases;
    std::vector<RimSummaryCaseCollection*> ensembleCases;

    for ( RimSummaryCurve* curve : allCurves() )
    {
        addresses.push_back( curve->curveAddress() );
        sumCases.push_back( curve->summaryCaseY() );
    }

    for ( auto curveSet : curveSets() )
    {
        addresses.push_back( curveSet->curveAddress() );
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
    axesGroup->add( &m_linkTimeAxis );
    axesGroup->add( &m_autoAdjustAppearance );

    auto plotVisibilityFilterGroup = uiOrdering.addNewGroup( "Plot Visibility Filter" );
    plotVisibilityFilterGroup->add( &m_plotFilterYAxisThreshold );
    plotVisibilityFilterGroup->add( &m_hidePlotsWithValuesBelow );

    auto dataSourceGroup = uiOrdering.addNewGroup( "Data Source" );
    dataSourceGroup->setCollapsedByDefault();
    m_sourceStepping()->uiOrdering( uiConfigName, *dataSourceGroup );

    if ( m_sourceStepping->stepDimension() == SourceSteppingDimension::WELL ) dataSourceGroup->add( &m_allow3DSelectionLink );

    auto titlesGroup = uiOrdering.addNewGroup( "Main Plot Settings" );
    titlesGroup->setCollapsedByDefault();

    // If a checkbox is first in the group, it is not responding to mouse clicks. Set title as first element.
    // https://github.com/OPM/ResInsight/issues/10321
    titlesGroup->add( &m_plotWindowTitle );
    titlesGroup->add( &m_autoPlotTitle );
    titlesGroup->add( &m_showPlotWindowTitle );
    titlesGroup->add( &m_titleFontSize );

    auto subPlotSettingsGroup = uiOrdering.addNewGroup( "Sub Plot Settings" );
    subPlotSettingsGroup->setCollapsedByDefault();
    subPlotSettingsGroup->add( &m_autoSubPlotTitle );
    subPlotSettingsGroup->add( &m_showIndividualPlotTitles );
    subPlotSettingsGroup->add( &m_subTitleFontSize );

    auto legendsGroup = uiOrdering.addNewGroup( "Legends" );
    legendsGroup->setCollapsedByDefault();
    legendsGroup->add( &m_showPlotLegends );
    legendsGroup->add( &m_plotLegendsHorizontal );
    legendsGroup->add( &m_legendPosition );
    legendsGroup->add( &m_legendFontSize );

    uiOrdering.skipRemainingFields( true );

    updateReadOnlyState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_autoPlotTitle || changedField == &m_autoSubPlotTitle )
    {
        onLoadDataAndUpdate();
        updateLayout();
    }
    else if ( changedField == &m_linkTimeAxis )
    {
        updateTimeAxisRangesFromFirstTimePlot();
    }
    else if ( changedField == &m_linkSubPlotAxes || changedField == &m_axisRangeAggregation || changedField == &m_linkTimeAxis )
    {
        setAutoValueStates();
        syncAxisRanges();
        analyzePlotsAndAdjustAppearanceSettings();
        zoomAll();
    }
    else if ( changedField == &m_hidePlotsWithValuesBelow )
    {
        m_hidePlotsWithValuesBelow = false;
        updatePlotVisibility();
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
        setAutoValueStates();
        analyzePlotsAndAdjustAppearanceSettings();
    }
    else if ( changedField == &m_plotWindowTitle )
    {
        // If the user has changed the plot title, disable the auto plot title
        // Workaround for https://github.com/OPM/ResInsight/issues/9681

        m_autoPlotTitle = false;
    }

    RimMultiPlot::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_hidePlotsWithValuesBelow == field )
    {
        auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Apply Filter";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updatePlotTitles()
{
    if ( m_autoPlotTitle )
    {
        populateNameHelper( m_nameHelper.get() );

        auto title = m_nameHelper->plotTitle();

        if ( title.isEmpty() )
        {
            auto collections = RimMainPlotCollection::current()->summaryMultiPlotCollection();

            size_t index = 0;
            for ( auto p : collections->multiPlots() )
            {
                index++;
                if ( p == this ) break;
            }

            title = QString( "Plot %1" ).arg( index );
        }

        setMultiPlotTitle( title );
    }

    for ( auto plot : summaryPlots() )
    {
        if ( m_autoSubPlotTitle )
        {
            auto subPlotNameHelper = plot->plotTitleHelper();

            // Disable auto plot title, as this is required to be able to include the information in the multi plot
            // title
            plot->enableAutoPlotTitle( false );

            auto plotName = subPlotNameHelper->aggregatedPlotTitle( *m_nameHelper );
            plot->setPlotTitleVisible( true );
            plot->setDescription( plotName );
        }
        plot->updatePlotTitle();
    }

    if ( !m_viewer.isNull() ) m_viewer->scheduleTitleUpdate();
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
void RimSummaryMultiPlot::setAutoPlotTitle( bool enable )
{
    m_autoPlotTitle = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::setAutoSubPlotTitle( bool enable )
{
    m_autoSubPlotTitle = enable;
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

    toolBarFields.push_back( &m_createPlotDuplicate );

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
    }
    return false;
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
        plot->plotZoomedByUser.connect( this, &RimSummaryMultiPlot::onSubPlotZoomed );
        plot->titleChanged.connect( this, &RimSummaryMultiPlot::onSubPlotChanged );
        plot->axisChangedReloadRequired.connect( this, &RimSummaryMultiPlot::onSubPlotAxisReloadRequired );
        plot->autoTitleChanged.connect( this, &RimSummaryMultiPlot::onSubPlotAutoTitleChanged );
    }
    updateStepDimensionFromDefault();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::onLoadDataAndUpdate()
{
    RimMultiPlot::onLoadDataAndUpdate();
    updatePlotTitles();

    analyzePlotsAndAdjustAppearanceSettings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::zoomAll()
{
    setAutoValueStates();

    if ( m_linkSubPlotAxes() )
    {
        // Reset zoom to make sure the complete range for min/max is available
        RimMultiPlot::zoomAll();

        if ( !summaryPlots().empty() )
        {
            onSubPlotAxisChanged( nullptr, summaryPlots().front() );
        }

        updateZoom();

        updateTimeAxisRangesFromFirstTimePlot();

        return;
    }

    // Reset zoom to make sure the complete range for min/max is available
    RimMultiPlot::zoomAll();

    syncAxisRanges();

    updateTimeAxisRangesFromFirstTimePlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updateTimeAxisRangesFromFirstTimePlot()
{
    if ( m_linkTimeAxis )
    {
        auto allPlots = summaryPlots();
        for ( auto plot : allPlots )
        {
            auto curves = plot->summaryAndEnsembleCurves();
            for ( auto curve : curves )
            {
                if ( curve->axisTypeX() == RiaDefines::HorizontalAxisType::TIME )
                {
                    setAutoScaleXEnabled( false );
                    syncTimeAxisRanges( plot );

                    return;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::setDefaultRangeAggregationSteppingDimension()
{
    RiaSummaryAddressAnalyzer analyzer;

    for ( auto p : summaryPlots() )
    {
        auto addresses = RimSummaryAddressModifier::allSummaryAddressesY( p );
        analyzer.appendAddresses( addresses );
    }

    auto stepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR;
    if ( analyzer.wellNames().size() == 1 )
    {
        auto wellName = *( analyzer.wellNames().begin() );

        if ( analyzer.wellSegmentNumbers( wellName ).size() == 1 )
        {
            stepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::WELL_SEGMENT;
        }
        else
        {
            stepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::WELL;
        }
    }
    else if ( analyzer.groupNames().size() == 1 )
    {
        stepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::GROUP;
    }
    else if ( analyzer.networkNames().size() == 1 )
    {
        stepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::NETWORK;
    }
    else if ( analyzer.regionNumbers().size() == 1 )
    {
        stepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::REGION;
    }
    else if ( analyzer.aquifers().size() == 1 )
    {
        stepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::AQUIFER;
    }
    else if ( analyzer.blocks().size() == 1 )
    {
        stepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::BLOCK;
    }

    m_sourceStepping->setStepDimension( stepDimension );

    // Previously, when the stepping dimension was set to 'well' for range aggregation, it was based on all wells. If one of the wells had
    // extreme values and was not visible, it would set the y-range to match the extreme value, making some curves invisible. We have now
    // changed the default setting to use visible subplots to determine the y-range aggregation.
    // https://github.com/OPM/ResInsight/issues/10543

    m_axisRangeAggregation = AxisRangeAggregation::SUB_PLOTS;

    setAutoValueStates();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::syncAxisRanges()
{
    if ( m_linkSubPlotAxes() )
    {
        return;
    }

    if ( m_axisRangeAggregation() == AxisRangeAggregation::NONE )
    {
        return;
    }

    // Reset zoom for axes with no custom range set to make sure the complete range for min/max is available

    for ( auto p : summaryPlots() )
    {
        for ( auto ax : p->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
        {
            ax->setAutoZoomIfNoCustomRangeIsSet();
        }
    }
    updateZoom();

    if ( m_axisRangeAggregation() == AxisRangeAggregation::SUB_PLOTS )
    {
        std::map<QString, std::pair<double, double>> axisRanges;

        // gather current min/max values for each category (axis label)
        for ( auto plot : summaryPlots() )
        {
            for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
            {
                double minVal = axis->visibleRangeMin();
                double maxVal = axis->visibleRangeMax();
                if ( axis->isAxisInverted() ) std::swap( minVal, maxVal );

                auto key = axis->objectName();
                if ( axisRanges.count( key ) == 0 )
                {
                    axisRanges[key] = std::make_pair( minVal, maxVal );
                }
                else
                {
                    auto& [currentMin, currentMax] = axisRanges[key];
                    axisRanges[key]                = std::make_pair( std::min( currentMin, minVal ), std::max( currentMax, maxVal ) );
                }
            }
        }

        // set all plots to use the global min/max values for each category
        for ( auto plot : summaryPlots() )
        {
            for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
            {
                auto [minVal, maxVal] = axisRanges[axis->objectName()];
                if ( axis->isAxisInverted() ) std::swap( minVal, maxVal );
                axis->setAutoZoom( false );

                axis->setAutoValueVisibleRangeMin( minVal );
                axis->setAutoValueVisibleRangeMax( maxVal );
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
void RimSummaryMultiPlot::syncTimeAxisRanges( RimSummaryPlot* sourceSummaryPlot )
{
    if ( sourceSummaryPlot && m_linkTimeAxis() )
    {
        for ( auto plot : summaryPlots() )
        {
            if ( plot != sourceSummaryPlot )
            {
                plot->copyAxisPropertiesFromOther( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, *sourceSummaryPlot );
                plot->updateAll();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::computeAggregatedAxisRange()
{
    auto readValues = []( RimSummaryCase* summaryCase, RifEclipseSummaryAddress addr )
    {
        if ( summaryCase && summaryCase->summaryReader() )
        {
            RifSummaryReaderInterface* reader = summaryCase->summaryReader();
            auto [isOk, values]               = reader->values( addr );
            return values;
        }

        return std::vector<double>();
    };

    auto findMinMaxForSummaryCase = [readValues]( RimSummaryCase* summaryCase, RifEclipseSummaryAddress addr, bool onlyPositiveValues )
    {
        auto values = readValues( summaryCase, addr );
        if ( onlyPositiveValues )
        {
            std::vector<double> positiveValues;

            for ( const auto& v : values )
            {
                if ( v > 0.0 ) positiveValues.push_back( v );
            }

            values = positiveValues;
        }
        if ( values.empty() ) return std::make_pair( HUGE_VAL, -HUGE_VAL );

        auto   minMaxPair  = std::minmax_element( values.begin(), values.end() );
        double caseMinimum = *minMaxPair.first;
        double caseMaximum = *minMaxPair.second;

        return std::make_pair( caseMinimum, caseMaximum );
    };

    auto summaryCasesForCurve = []( RimSummaryCurve* curve, AxisRangeAggregation axisRangeAggregation )
    {
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
        else if ( axisRangeAggregation == AxisRangeAggregation::WELLS || axisRangeAggregation == AxisRangeAggregation::REGIONS )
        {
            // Use only the current summary case when aggregation across wells/regions
            summaryCases.push_back( curve->summaryCaseY() );
        }

        return summaryCases;
    };

    auto addressesForCurve = []( RimSummaryCurve* curve, AxisRangeAggregation axisRangeAggregation )
    {
        std::vector<RifEclipseSummaryAddress> addresses;

        auto addr = curve->summaryAddressY();
        if ( axisRangeAggregation == AxisRangeAggregation::REALIZATIONS )
        {
            addresses = { RifEclipseSummaryAddress::fieldAddress( addr.vectorName(), addr.id() ) };
        }
        else if ( axisRangeAggregation == AxisRangeAggregation::WELLS || axisRangeAggregation == AxisRangeAggregation::REGIONS )
        {
            RiaSummaryAddressAnalyzer  fallbackAnalyzer;
            RiaSummaryAddressAnalyzer* analyzer = nullptr;

            if ( curve->summaryCaseY() )
            {
                auto ensemble = curve->summaryCaseY()->ensemble();
                if ( ensemble )
                {
                    analyzer = ensemble->addressAnalyzer();
                }
                else
                {
                    fallbackAnalyzer.appendAddresses( curve->summaryCaseY()->summaryReader()->allResultAddresses() );
                    analyzer = &fallbackAnalyzer;
                }
            }

            if ( analyzer )
            {
                if ( axisRangeAggregation == AxisRangeAggregation::WELLS )
                {
                    for ( const auto& wellName : analyzer->wellNames() )
                    {
                        addresses.push_back( RifEclipseSummaryAddress::wellAddress( addr.vectorName(), wellName, addr.id() ) );
                    }
                }

                if ( axisRangeAggregation == AxisRangeAggregation::REGIONS )
                {
                    for ( auto regionNumber : analyzer->regionNumbers() )
                    {
                        addresses.push_back( RifEclipseSummaryAddress::regionAddress( addr.vectorName(), regionNumber, addr.id() ) );
                    }
                }
            }
        }

        return addresses;
    };

    auto findMinMaxForAddressesInSummaryCases = [findMinMaxForSummaryCase]( const std::vector<RifEclipseSummaryAddress>& addresses,
                                                                            const std::vector<RimSummaryCase*>&          summaryCases,
                                                                            bool onlyPositiveValues )
    {
        double minimum = HUGE_VAL;
        double maximum = -HUGE_VAL;
        for ( auto summaryCase : summaryCases )
        {
            for ( const auto& addr : addresses )
            {
                auto [caseMinimum, caseMaximum] = findMinMaxForSummaryCase( summaryCase, addr, onlyPositiveValues );
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

        for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
        {
            for ( auto curve : plot->summaryCurves() )
            {
                if ( curve->axisY() == axis->plotAxis() )
                {
                    std::vector<RimSummaryCase*>          summaryCases = summaryCasesForCurve( curve, m_axisRangeAggregation() );
                    std::vector<RifEclipseSummaryAddress> addresses    = addressesForCurve( curve, m_axisRangeAggregation() );

                    bool onlyPositiveValues = axis->isLogarithmicScaleEnabled();

                    auto [minimum, maximum] = findMinMaxForAddressesInSummaryCases( addresses, summaryCases, onlyPositiveValues );

                    if ( axisRanges.count( axis->plotAxis() ) == 0 )
                    {
                        axisRanges[axis->plotAxis()] = std::make_pair( minimum, maximum );
                    }
                    else
                    {
                        auto& [currentMin, currentMax] = axisRanges[axis->plotAxis()];
                        axisRanges[axis->plotAxis()]   = std::make_pair( std::min( currentMin, minimum ), std::max( currentMax, maximum ) );
                    }
                }
            }

            for ( auto curveSet : plot->curveSets() )
            {
                if ( !curveSet->summaryCaseCollection() ) continue;

                if ( curveSet->axisY() == axis->plotAxis() )
                {
                    double minimum( std::numeric_limits<double>::infinity() );
                    double maximum( -std::numeric_limits<double>::infinity() );

                    auto curves = curveSet->curves();
                    if ( !curves.empty() )
                    {
                        // TODO: Use analyzer as input to addressesForCurve instead of curve

                        auto curve = curves.front();

                        std::vector<RifEclipseSummaryAddress> addresses = addressesForCurve( curve, m_axisRangeAggregation() );

                        for ( const auto& adr : addresses )
                        {
                            auto [min, max] = curveSet->summaryCaseCollection()->minMax( adr );

                            minimum = std::min( min, minimum );
                            maximum = std::max( max, maximum );
                        }
                    }

                    if ( axisRanges.count( axis->plotAxis() ) == 0 )
                    {
                        axisRanges[axis->plotAxis()] = std::make_pair( minimum, maximum );
                    }
                    else
                    {
                        auto& [currentMin, currentMax] = axisRanges[axis->plotAxis()];
                        axisRanges[axis->plotAxis()]   = std::make_pair( std::min( currentMin, minimum ), std::max( currentMax, maximum ) );
                    }
                }
            }
        }

        // set all plots to use the global min/max values for each category
        for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
        {
            auto [minVal, maxVal] = axisRanges[axis->plotAxis()];
            if ( RiaDefines::isVertical( axis->plotAxis().axis() ) && !std::isinf( minVal ) && !std::isinf( maxVal ) )
            {
                axis->setAutoZoom( false );

                if ( axis->isAxisInverted() ) std::swap( minVal, maxVal );

                auto [adjustedMinVal, adjustedMaxVal] = adjustedMinMax( axis, minVal, maxVal );

                axis->setAutoValueVisibleRangeMin( adjustedMinVal );
                axis->setAutoValueVisibleRangeMax( adjustedMaxVal );
            }
        }

        plot->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updatePlotVisibility()
{
    auto hasValuesAboveLimit = []( RimSummaryPlot* plot, double limit )
    {
        for ( auto curve : plot->summaryAndEnsembleCurves() )
        {
            auto address  = curve->valuesY();
            auto maxValue = std::max_element( address.begin(), address.end() );

            if ( *maxValue > limit ) return true;
        }

        return false;
    };

    for ( auto plot : summaryPlots() )
    {
        bool hasValueAboveLimit = hasValuesAboveLimit( plot, m_plotFilterYAxisThreshold );
        plot->setShowWindow( hasValueAboveLimit );
    }

    updateLayout();

    if ( !m_viewer.isNull() ) m_viewer->scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::setAutoValueStates()
{
    bool enableMinMaxAutoValue = m_axisRangeAggregation() != AxisRangeAggregation::NONE;
    for ( auto p : summaryPlots() )
    {
        setAutoValueStatesForPlot( p, enableMinMaxAutoValue, m_autoAdjustAppearance() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::setAutoValueStatesForPlot( RimSummaryPlot* summaryPlot, bool enableAutoValueMinMax, bool enableAutoValueAppearance )
{
    auto timeAxisProp = summaryPlot->timeAxisProperties();
    if ( timeAxisProp ) timeAxisProp->enableAutoValueForMajorTickmarkCount( enableAutoValueAppearance );

    for ( auto plotAxis : summaryPlot->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
    {
        plotAxis->enableAutoValueMinMax( enableAutoValueMinMax );
        plotAxis->enableAutoValueForMajorTickmarkCount( enableAutoValueAppearance );
        plotAxis->enableAutoValueForScaleFactor( enableAutoValueAppearance );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::summaryPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const
{
    for ( RimSummaryPlot* plot : summaryPlots() )
    {
        QString displayName;

        if ( plot && plot->userDescriptionField() && plot->userDescriptionField() )
        {
            displayName = plot->userDescriptionField()->uiCapability()->uiValue().toString();
        }

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
    if ( m_autoAdjustAppearance )
    {
        // Required to sync axis ranges before computing the auto scale
        syncAxisRanges();

        RiaSummaryAddressAnalyzer analyzer;

        for ( auto p : summaryPlots() )
        {
            auto addresses = RimSummaryAddressModifier::allSummaryAddressesY( p );
            analyzer.appendAddresses( addresses );
        }

        bool canShowOneAxisTitlePerRow = analyzer.isSingleQuantityIgnoreHistory() && ( m_axisRangeAggregation() != AxisRangeAggregation::NONE );

        const bool notifyFieldChanged = false;

        for ( auto p : summaryPlots() )
        {
            if ( auto timeAxisProp = p->timeAxisProperties() )
            {
                auto tickMarkCount = ( columnCount() < 3 ) ? RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_DEFAULT
                                                           : RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_FEW;

                timeAxisProp->setAutoValueForMajorTickmarkCount( tickMarkCount, notifyFieldChanged );
            }

            for ( auto* axisProp : p->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
            {
                auto tickMarkCount = ( rowsPerPage() == 1 ) ? RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_DEFAULT
                                                            : RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_FEW;

                axisProp->setAutoValueForMajorTickmarkCount( tickMarkCount, notifyFieldChanged );

                axisProp->computeAndSetAutoValueForScaleFactor();

                if ( canShowOneAxisTitlePerRow )
                {
                    auto [row, col] = gridLayoutInfoForSubPlot( p );

                    bool isFirstColumn = ( col == 0 );
                    axisProp->setShowUnitText( isFirstColumn );
                    axisProp->setShowDescription( isFirstColumn );
                }
                else
                {
                    axisProp->setShowUnitText( true );
                    axisProp->setShowDescription( true );
                }
            }

            p->updateAxes();
        }
    }
    else
    {
        for ( auto p : summaryPlots() )
        {
            for ( auto* axisProp : p->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
            {
                axisProp->computeAndSetAutoValueForScaleFactor();
                axisProp->setShowUnitText( true );
                axisProp->setShowDescription( true );
            }

            p->updateAxes();
        }
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
void RimSummaryMultiPlot::setTimeAxisLinked( bool enable )
{
    m_linkTimeAxis = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryMultiPlot::isTimeAxisLinked() const
{
    return m_linkTimeAxis();
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
    updatePlotTitles();
    applyPlotWindowTitleToWidgets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::onSubPlotZoomed( const caf::SignalEmitter* emitter )
{
    m_axisRangeAggregation = AxisRangeAggregation::NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::onSubPlotAxisChanged( const caf::SignalEmitter* emitter, RimSummaryPlot* summaryPlot )
{
    syncTimeAxisRanges( summaryPlot );

    if ( !m_linkSubPlotAxes() )
    {
        syncAxisRanges();
        return;
    }

    for ( auto plot : summaryPlots() )
    {
        if ( plot != summaryPlot )
        {
            plot->copyMatchingAxisPropertiesFromOther( *summaryPlot );
            plot->updateAll();
        }
    }

    syncAxisRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::onSubPlotAxisReloadRequired( const caf::SignalEmitter* emitter, RimSummaryPlot* summaryPlot )
{
    if ( !summaryPlot ) return;

    if ( m_linkTimeAxis() )
    {
        syncTimeAxisRanges( summaryPlot );

        for ( auto plot : summaryPlots() )
        {
            plot->loadDataAndUpdate();
        }
    }
    else
    {
        summaryPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::onSubPlotAutoTitleChanged( const caf::SignalEmitter* emitter, bool isEnabled )
{
    m_autoSubPlotTitle = isEnabled;

    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updateReadOnlyState()
{
    m_axisRangeAggregation.uiCapability()->setUiReadOnly( m_linkSubPlotAxes() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimSummaryMultiPlot::adjustedMinMax( const RimPlotAxisProperties* axis, double min, double max ) const
{
    if ( !axis->isLogarithmicScaleEnabled() )
    {
        int                  maxMajorTickIntervalCount = RimPlotAxisProperties::tickmarkCountFromEnum( axis->majorTickmarkCount() );
        double               stepSize                  = 0.0;
        QwtLinearScaleEngine scaleEngine;

        // Do not adjust minimum value, as we usually want to keep zero unchanged
        double adjustedMin = min;

        // Adjust the max value to get some space between the top of the plot and the top of the curve
        double adjustedMax = max * 1.05;

        scaleEngine.autoScale( maxMajorTickIntervalCount, adjustedMin, adjustedMax, stepSize );

        return { adjustedMin, adjustedMax };
    }

    auto adjustedMin = RiaNumericalTools::roundToClosestPowerOfTenFloor( min );
    auto adjustedMax = RiaNumericalTools::roundToClosestPowerOfTenCeil( max );

    return { adjustedMin, adjustedMax };
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

    // find matching plots
    std::vector<RimPlot*> plots = m_sourceStepping->plotsMatchingStepSettings( summaryPlots() );
    if ( plots.empty() ) return;

    // duplicate them
    auto newPlots = RicSummaryPlotBuilder::duplicatePlots( plots );
    if ( newPlots.empty() ) return;

    for ( auto plot : newPlots )
    {
        RimSummaryPlot* newPlot = dynamic_cast<RimSummaryPlot*>( plot );
        if ( newPlot == nullptr ) continue;

        addPlot( newPlot );
        newPlot->resolveReferencesRecursively();

        if ( m_sourceStepping()->stepDimension() == RimSummaryDataSourceStepping::SourceSteppingDimension::SUMMARY_CASE )
        {
            RimSummaryCase* newCase = m_sourceStepping()->stepCase( direction );
            for ( auto curve : newPlot->allCurves() )
            {
                curve->setSummaryCaseY( newCase );

                // NOTE: If summary cross plots should be handled here, we also need to call
                // curve->setSummaryCaseX( newCase );
                // Setting summaryCaseX with a default uninitialized summary address causes issues for the summary
                // name analyzer
            }
        }
        else if ( m_sourceStepping()->stepDimension() == RimSummaryDataSourceStepping::SourceSteppingDimension::ENSEMBLE )
        {
            RimSummaryCaseCollection* newEnsemble = m_sourceStepping()->stepEnsemble( direction );
            for ( auto curveSet : newPlot->curveSets() )
            {
                curveSet->setSummaryCaseCollection( newEnsemble );
            }
        }
        else
        {
            std::vector<RiaSummaryCurveAddress> newCurveAdrs;

            auto curveAddressProviders = RimSummaryAddressModifier::createAddressProviders( newPlot );
            for ( const auto& adr : RimSummaryAddressModifier::curveAddresses( curveAddressProviders ) )
            {
                const auto adrX = m_sourceStepping()->stepAddress( adr.summaryAddressX(), direction );
                const auto adrY = m_sourceStepping()->stepAddress( adr.summaryAddressY(), direction );
                newCurveAdrs.push_back( RiaSummaryCurveAddress( adrX, adrY ) );
            }

            RimSummaryAddressModifier::applyAddressesToCurveAddressProviders( curveAddressProviders, newCurveAdrs );
        }
    }

    loadDataAndUpdate();
    updateConnectedEditors();

    RiuPlotMainWindowTools::selectAsCurrentItem( newPlots.back() );

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

        for ( auto curve : plot->allCurves() )
        {
            auto address   = curve->summaryAddressY();
            auto sumCase   = curve->summaryCaseY();
            int  sumCaseId = sumCase->caseId();
            if ( m_sourceStepping()->stepDimension() == RimSummaryDataSourceStepping::SourceSteppingDimension::SUMMARY_CASE )
            {
                auto nextSumCase = m_sourceStepping->stepCase( direction );
                if ( nextSumCase ) sumCaseId = nextSumCase->caseId();
            }
            else
            {
                address = m_sourceStepping->stepAddress( address, direction );
            }
            addresses.push_back( RimSummaryAddress::wrapFileReaderAddress( address, sumCaseId ) );
        }

        for ( auto curveSet : plot->curveSets() )
        {
            auto address  = curveSet->summaryAddressY();
            auto sumEns   = curveSet->summaryCaseCollection();
            int  sumEnsId = sumEns->ensembleId();
            if ( m_sourceStepping()->stepDimension() == RimSummaryDataSourceStepping::SourceSteppingDimension::ENSEMBLE )
            {
                auto nextEns = m_sourceStepping->stepEnsemble( direction );
                if ( nextEns ) sumEnsId = nextEns->ensembleId();
            }
            else
            {
                address = m_sourceStepping->stepAddress( address, direction );
            }
            addresses.push_back( RimSummaryAddress::wrapFileReaderAddress( address, -1, sumEnsId ) );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::storeStepDimensionFromToolbar()
{
    m_defaultStepDimension = m_sourceStepping->stepDimension();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::updateStepDimensionFromDefault()
{
    m_sourceStepping->setStepDimension( m_defaultStepDimension() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::selectWell( QString wellName )
{
    if ( !m_allow3DSelectionLink ) return;
    if ( m_sourceStepping->stepDimension() != SourceSteppingDimension::WELL ) return;
    m_sourceStepping->setStep( wellName );
}
