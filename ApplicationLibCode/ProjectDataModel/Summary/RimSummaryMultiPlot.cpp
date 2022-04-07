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

#include "RiaSummaryAddressAnalyzer.h"
#include "RiaSummaryStringTools.h"

#include "RimEnsembleCurveSet.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlotCollection.h"
#include "RimMultipleSummaryPlotNameHelper.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlotControls.h"

#include "RimMultiPlot.h"
#include "RimSummaryAddress.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotNameHelper.h"
#include "RimSummaryPlotSourceStepping.h"

#include "RiuSummaryVectorSelectionUi.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QKeyEvent>

CAF_PDM_SOURCE_INIT( RimSummaryMultiPlot, "MultiSummaryPlot" );
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

    CAF_PDM_InitField( &m_syncAxisRanges, "SyncAxisRanges", false, "", "", "Sync Axis Ranges in All Plots" );
    m_syncAxisRanges.xmlCapability()->disableIO();
    m_syncAxisRanges.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_syncAxisRanges.uiCapability()->setUiIconFromResourceString( ":/AxesSync16x16.png" );

    CAF_PDM_InitField( &m_createPlotDuplicate, "DuplicatePlot", false, "", "", "Duplicate Plot" );
    m_createPlotDuplicate.xmlCapability()->disableIO();
    m_createPlotDuplicate.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_createPlotDuplicate.uiCapability()->setUiIconFromResourceString( ":/Copy.svg" );

    CAF_PDM_InitField( &m_disableWheelZoom, "DisableWheelZoom", true, "", "", "Disable Mouse Wheel Zooming in Multi Summary Plot" );
    m_disableWheelZoom.xmlCapability()->disableIO();
    m_disableWheelZoom.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_disableWheelZoom.uiCapability()->setUiIconFromResourceString( ":/DisableZoom.png" );

    CAF_PDM_InitField( &m_syncSubPlotAxes, "SyncSubPlotAxes", false, "Sync Subplot Axes" );

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
    RimSummaryPlot* sumPlot = dynamic_cast<RimSummaryPlot*>( plot );
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
    RimSummaryPlot* sumPlot = dynamic_cast<RimSummaryPlot*>( plot );
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
void RimSummaryMultiPlot::addPlot( const std::vector<caf::PdmObjectHandle*>& objects )
{
    if ( objects.empty() ) return;

    RimSummaryAddress* addr = dynamic_cast<RimSummaryAddress*>( objects[0] );
    if ( addr )
    {
        RimSummaryPlot* plot = new RimSummaryPlot();
        plot->enableAutoPlotTitle( true );

        plot->handleDroppedObjects( objects );

        addPlot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlot::removePlot( RimPlot* plot )
{
    RimSummaryPlot* sumPlot = dynamic_cast<RimSummaryPlot*>( plot );
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
    RimSummaryPlot* sumPlot = dynamic_cast<RimSummaryPlot*>( plot );
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
    caf::PdmUiGroup* titlesGroup = uiOrdering.addNewGroup( "Titles" );
    titlesGroup->add( &m_autoPlotTitles );
    titlesGroup->add( &m_autoPlotTitlesOnSubPlots );

    titlesGroup->add( &m_showPlotWindowTitle );
    titlesGroup->add( &m_plotWindowTitle );
    titlesGroup->add( &m_showIndividualPlotTitles );
    titlesGroup->add( &m_titleFontSize );
    titlesGroup->add( &m_subTitleFontSize );

    caf::PdmUiGroup* legendsGroup = uiOrdering.addNewGroup( "Legends" );
    legendsGroup->add( &m_showPlotLegends );
    legendsGroup->add( &m_plotLegendsHorizontal );
    legendsGroup->add( &m_legendFontSize );

    caf::PdmUiGroup* layoutGroup = uiOrdering.addNewGroup( "Layout" );
    layoutGroup->add( &m_columnCount );
    layoutGroup->add( &m_rowsPerPage );
    layoutGroup->add( &m_majorTickmarkCount );

    caf::PdmUiGroup* axesGroup = uiOrdering.addNewGroup( "Axes" );
    axesGroup->add( &m_syncSubPlotAxes );

    {
        auto group = uiOrdering.addNewGroup( "Data Source" );
        m_sourceStepping()->uiOrdering( uiConfigName, *group );
    }

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
    else if ( changedField == &m_syncAxisRanges )
    {
        m_syncAxisRanges = false;
        syncAxisRanges();
    }
    else if ( changedField == &m_createPlotDuplicate )
    {
        m_createPlotDuplicate = false;
        duplicate();
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
    if ( &m_syncAxisRanges == field )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Sync Axes";
        }
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

            // Disable auto plot, as this is required to be able to include the information in the multi plot title
            plot->enableAutoPlotTitle( false );

            auto plotName = subPlotNameHelper->aggregatedPlotTitle( *m_nameHelper );
            plot->setDescription( plotName );
            plot->updatePlotTitle();
        }
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
    toolBarFields.push_back( &m_syncAxisRanges );
    toolBarFields.push_back( &m_createPlotDuplicate );

    auto& sourceObject = m_sourceStepping();
    if ( sourceObject )
    {
        auto fields = sourceObject->fieldsToShowInToolbar();
        toolBarFields.insert( std::end( toolBarFields ), std::begin( fields ), std::end( fields ) );
    }

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
void RimSummaryMultiPlot::syncAxisRanges()
{
    std::map<QString, std::pair<double, double>> axisRanges;

    // Reset zoom to make sure the complete range for min/max is available
    zoomAll();

    // gather current min/max values for each category (axis label)
    for ( auto plot : summaryPlots() )
    {
        for ( auto axis : plot->plotAxes() )
        {
            double minVal = axis->visibleRangeMin();
            double maxVal = axis->visibleRangeMax();

            if ( axisRanges.count( axis->name() ) == 0 )
            {
                axisRanges[axis->name()] = std::make_pair( axis->visibleRangeMin(), axis->visibleRangeMax() );
            }
            else
            {
                auto& [currentMin, currentMax] = axisRanges[axis->name()];
                axisRanges[axis->name()] = std::make_pair( std::min( currentMin, minVal ), std::max( currentMax, maxVal ) );
            }
        }
    }

    // set all plots to use the global min/max values for each category
    for ( auto plot : summaryPlots() )
    {
        for ( auto axis : plot->plotAxes() )
        {
            const auto& [minVal, maxVal] = axisRanges[axis->name()];
            axis->setAutoZoom( false );
            axis->setVisibleRangeMin( minVal );
            axis->setVisibleRangeMax( maxVal );
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
void RimSummaryMultiPlot::makeSureIsVisible( RimSummaryPlot* summaryPlot )
{
    if ( summaryPlot->plotWidget() && !m_viewer.isNull() ) m_viewer->scrollToPlot( summaryPlot->plotWidget() );
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
    if ( !m_syncSubPlotAxes() ) return;

    for ( auto plot : summaryPlots() )
    {
        if ( plot != summaryPlot )
        {
            plot->copyMatchingAxisPropertiesFromOther( *summaryPlot );
            plot->updateAll();
        }
    }
}
