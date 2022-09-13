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
    this->setDeletable( true );

    CAF_PDM_InitField( &m_autoPlotTitle, "AutoPlotTitle", true, "Auto Plot Title" );
    CAF_PDM_InitField( &m_autoSubPlotTitle, "AutoSubPlotTitle", true, "Auto Sub Plot Title" );

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
    m_hidePlotsWithValuesBelow.xmlCapability()->disableIO();
    m_hidePlotsWithValuesBelow.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_plotFilterYAxisThreshold, "PlotFilterYAxisThreshold", 0.0, "Y-Axis Filter Threshold" );

    CAF_PDM_InitFieldNoDefault( &m_sourceStepping, "SourceStepping", "" );

    m_sourceStepping = new RimSummaryPlotSourceStepping;
    m_sourceStepping->setSourceSteppingType( RimSummaryDataSourceStepping::Axis::Y_AXIS );
    m_sourceStepping->setSourceSteppingObject( this );
    m_sourceStepping.uiCapability()->setUiTreeHidden( true );
    m_sourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_sourceStepping.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_defaultStepDimension, "DefaultStepDimension", "Default Step Dimension" );
    m_defaultStepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR;
    m_defaultStepDimension.uiCapability()->setUiHidden( true );

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
        sumPlot->plotZoomedByUser.connect( this, &RimSummaryMultiPlot::onSubPlotZoomed );

        bool isMinMaxOverridden = m_axisRangeAggregation() != AxisRangeAggregation::NONE;
        setAutoValueStatesForPlot( sumPlot, isMinMaxOverridden, m_autoAdjustAppearance() );

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
    axesGroup->add( &m_linkTimeAxis );
    axesGroup->add( &m_autoAdjustAppearance );

    auto plotVisibilityFilterGroup = uiOrdering.addNewGroup( "Plot Visibility Filter" );
    plotVisibilityFilterGroup->add( &m_plotFilterYAxisThreshold );
    plotVisibilityFilterGroup->add( &m_hidePlotsWithValuesBelow );

    auto dataSourceGroup = uiOrdering.addNewGroup( "Data Source" );
    dataSourceGroup->setCollapsedByDefault();
    m_sourceStepping()->uiOrdering( uiConfigName, *dataSourceGroup );

    if ( m_sourceStepping->stepDimension() == SourceSteppingDimension::WELL )
        dataSourceGroup->add( &m_allow3DSelectionLink );

    auto titlesGroup = uiOrdering.addNewGroup( "Main Plot Settings" );
    titlesGroup->setCollapsedByDefault();
    titlesGroup->add( &m_autoPlotTitle );
    titlesGroup->add( &m_showPlotWindowTitle );
    titlesGroup->add( &m_plotWindowTitle );
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
void RimSummaryMultiPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
    if ( changedField == &m_autoPlotTitle || changedField == &m_autoSubPlotTitle )
    {
        onLoadDataAndUpdate();
        updateLayout();
    }
    else if ( changedField == &m_linkTimeAxis )
    {
        auto plots = summaryPlots();
        if ( !plots.empty() )
        {
            syncTimeAxisRanges( plots.front() );
        }
    }
    else if ( changedField == &m_linkSubPlotAxes || changedField == &m_axisRangeAggregation ||
              changedField == &m_linkTimeAxis )
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
    else
    {
        RimMultiPlot::fieldChangedByUi( changedField, oldValue, newValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                 QString                    uiConfigName,
                                                 caf::PdmUiEditorAttribute* attribute )
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
void RimSummaryMultiPlot::updatePlotWindowTitle()
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

    if ( m_autoSubPlotTitle )
    {
        for ( auto plot : summaryPlots() )
        {
            auto subPlotNameHelper = plot->plotTitleHelper();

            // Disable auto plot title, as this is required to be able to include the information in the multi plot
            // title
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
    }
    updateStepDimensionFromDefault();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::onLoadDataAndUpdate()
{
    RimMultiPlot::onLoadDataAndUpdate();
    updatePlotWindowTitle();

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

        return;
    }

    // Reset zoom to make sure the complete range for min/max is available
    RimMultiPlot::zoomAll();

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

    auto rangeAggregation = AxisRangeAggregation::NONE;
    if ( analyzer.quantities().size() == 1 && summaryPlots().size() > 1 )
    {
        // Many plots, single summary vector
        rangeAggregation = AxisRangeAggregation::SUB_PLOTS;
    }

    if ( !analyzer.wellNames().empty() )
    {
        rangeAggregation = AxisRangeAggregation::WELLS;
    }
    else if ( !analyzer.regionNumbers().empty() )
    {
        rangeAggregation = AxisRangeAggregation::REGIONS;
    }

    auto stepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR;
    if ( analyzer.wellNames().size() == 1 )
    {
        stepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::WELL;
    }
    else if ( analyzer.groupNames().size() == 1 )
    {
        stepDimension = RimSummaryDataSourceStepping::SourceSteppingDimension::GROUP;
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

    m_axisRangeAggregation = rangeAggregation;
    m_sourceStepping->setStepDimension( stepDimension );

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
        for ( auto ax : p->plotYAxes() )
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
            for ( auto axis : plot->plotYAxes() )
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
                    axisRanges[key] = std::make_pair( std::min( currentMin, minVal ), std::max( currentMax, maxVal ) );
                }
            }
        }

        // set all plots to use the global min/max values for each category
        for ( auto plot : summaryPlots() )
        {
            for ( auto axis : plot->plotYAxes() )
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
void RimSummaryMultiPlot::syncTimeAxisRanges( RimSummaryPlot* summaryPlot )
{
    if ( m_linkTimeAxis )
    {
        for ( auto plot : summaryPlots() )
        {
            if ( plot != summaryPlot )
            {
                plot->copyAxisPropertiesFromOther( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, *summaryPlot );
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
                        addresses.push_back(
                            RifEclipseSummaryAddress::wellAddress( curve->summaryAddressY().vectorName(), wellName ) );
                    }
                }

                if ( axisRangeAggregation == AxisRangeAggregation::REGIONS )
                {
                    for ( auto regionNumber : analyzer->regionNumbers() )
                    {
                        addresses.push_back( RifEclipseSummaryAddress::regionAddress( curve->summaryAddressY().vectorName(),
                                                                                      regionNumber ) );
                    }
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

        for ( auto axis : plot->plotYAxes() )
        {
            for ( auto curve : plot->summaryCurves() )
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

            for ( auto curveSet : plot->curveSets() )
            {
                if ( !curveSet->summaryCaseCollection() ) continue;

                if ( curveSet->axisY() == axis->plotAxisType() )
                {
                    double minimum( std::numeric_limits<double>::infinity() );
                    double maximum( -std::numeric_limits<double>::infinity() );

                    auto curves = curveSet->curves();
                    if ( !curves.empty() )
                    {
                        // TODO: Use analyzer as input to addressesForCurve instead of curve

                        auto curve = curves.front();

                        std::vector<RifEclipseSummaryAddress> addresses =
                            addressesForCurve( curve, m_axisRangeAggregation() );

                        for ( const auto& adr : addresses )
                        {
                            auto [min, max] = curveSet->summaryCaseCollection()->minMax( adr );

                            minimum = std::min( min, minimum );
                            maximum = std::max( max, maximum );
                        }
                    }

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
        for ( auto axis : plot->plotYAxes() )
        {
            auto [minVal, maxVal] = axisRanges[axis->plotAxisType()];
            if ( RiaDefines::isVertical( axis->plotAxisType().axis() ) && !std::isinf( minVal ) && !std::isinf( maxVal ) )
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
    auto hasValuesAboveLimit = []( RimSummaryPlot* plot, double limit ) {
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
void RimSummaryMultiPlot::setAutoValueStatesForPlot( RimSummaryPlot* summaryPlot,
                                                     bool            enableAutoValueMinMax,
                                                     bool            enableAutoValueAppearance )
{
    auto timeAxisProp = summaryPlot->timeAxisProperties();
    if ( timeAxisProp ) timeAxisProp->enableAutoValueForMajorTickmarkCount( enableAutoValueAppearance );

    for ( auto plotAxis : summaryPlot->plotYAxes() )
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
            auto addresses = RimSummaryAddressModifier::createEclipseSummaryAddress( p );
            analyzer.appendAddresses( addresses );
        }

        bool canShowOneAxisTitlePerRow = analyzer.isSingleQuantityIgnoreHistory() &&
                                         ( m_axisRangeAggregation() != AxisRangeAggregation::NONE );

        for ( auto p : summaryPlots() )
        {
            auto timeAxisProp = p->timeAxisProperties();

            if ( columnCount() < 3 )
                timeAxisProp->setAutoValueForMajorTickmarkCount(
                    RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_DEFAULT );
            else
                timeAxisProp->setAutoValueForMajorTickmarkCount( RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_FEW );

            for ( auto* axisProp : p->plotYAxes() )
            {
                if ( !axisProp ) continue;

                if ( rowsPerPage() == 1 )
                    axisProp->setAutoValueForMajorTickmarkCount(
                        RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_DEFAULT );
                else
                    axisProp->setAutoValueForMajorTickmarkCount(
                        RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_FEW );

                axisProp->computeAndSetAutoValueForScaleFactor();

                if ( canShowOneAxisTitlePerRow )
                {
                    auto [row, col] = gridLayoutInfoForSubPlot( p );
                    if ( col == 0 )
                    {
                        axisProp->setShowUnitText( true );
                        axisProp->setShowDescription( true );
                    }
                    else
                    {
                        axisProp->setShowUnitText( false );
                        axisProp->setShowDescription( false );
                    }
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
            for ( auto* axisProp : p->plotYAxes() )
            {
                if ( !axisProp ) continue;

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
    updatePlotWindowTitle();
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
        int                  maxMajorTickIntervalCount = axis->tickmarkCountFromEnum( axis->majorTickmarkCount() );
        double               stepSize                  = 0.0;
        QwtLinearScaleEngine scaleEngine;

        // Do not adjust minimum value, as we usually want to keep zero unchanged
        double adjustedMin = min;

        // Adjust the max value to get some space between the top of the plot and the top of the curve
        double adjustedMax = max * 1.05;

        scaleEngine.autoScale( maxMajorTickIntervalCount, adjustedMin, adjustedMax, stepSize );

        return { adjustedMin, adjustedMax };
    }

    return { min, max };
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
            for ( auto curve : newPlot->allCurves( RimSummaryDataSourceStepping::Axis::Y_AXIS ) )
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
            auto mods = RimSummaryAddressModifier::createAddressModifiersForPlot( newPlot );
            for ( auto& mod : mods )
            {
                auto modifiedAdr = m_sourceStepping()->stepAddress( mod.address(), direction );
                mod.setAddress( modifiedAdr );
            }
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

        for ( auto curve : plot->allCurves( RimSummaryDataSourceStepping::Axis::Y_AXIS ) )
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
            auto address  = curveSet->summaryAddress();
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
