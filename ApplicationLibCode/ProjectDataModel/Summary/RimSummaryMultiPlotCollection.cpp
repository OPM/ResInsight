/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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
#include "RimSummaryMultiPlotCollection.h"

#include "RimProject.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotReadOut.h"
#include "RimTools.h"

#include "RiuPlotMainWindowTools.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimSummaryMultiPlotCollection, "RimSummaryMultiPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlotCollection::RimSummaryMultiPlotCollection()
{
    CAF_PDM_InitObject( "Summary Plots", ":/MultiPlot16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_summaryMultiPlots, "MultiSummaryPlots", "Summary Plots" );
    caf::PdmFieldReorderCapability::addToField( &m_summaryMultiPlots );

    CAF_PDM_InitField( &m_useGlobalReadoutSettings, "UseGlobalReadoutSettings", false, "Use Global Readout Settings" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useGlobalReadoutSettings );

    CAF_PDM_InitFieldNoDefault( &m_readoutSettings, "ReadOutSettings", "Read Out Settings" );
    m_readoutSettings = new RimSummaryPlotReadOut;
    m_readoutSettings.uiCapability()->setUiTreeChildrenHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlotCollection::~RimSummaryMultiPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::initAfterRead()
{
    for ( auto& plot : m_summaryMultiPlots )
    {
        plot->duplicatePlot.connect( this, &RimSummaryMultiPlotCollection::onDuplicatePlot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::deleteAllPlots()
{
    m_summaryMultiPlots.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryMultiPlot*> RimSummaryMultiPlotCollection::multiPlots() const
{
    return m_summaryMultiPlots.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::addSummaryMultiPlot( RimSummaryMultiPlot* plot )
{
    m_summaryMultiPlots().push_back( plot );
    plot->duplicatePlot.connect( this, &RimSummaryMultiPlotCollection::onDuplicatePlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::removePlotNoUpdate( RimSummaryMultiPlot* plot )
{
    if ( plot == nullptr ) return;
    m_summaryMultiPlots().removeChild( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::loadDataAndUpdateAllPlots()
{
    for ( const auto& p : m_summaryMultiPlots.childrenByType() )
        p->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryMultiPlotCollection::plotCount() const
{
    return m_summaryMultiPlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::onDuplicatePlot( const caf::SignalEmitter* emitter, RimSummaryMultiPlot* plotToDuplicate )
{
    duplicatePlot( plotToDuplicate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::onRefreshTree( const caf::SignalEmitter* emitter, RimSummaryMultiPlot* plotRequesting )
{
    if ( !plotRequesting ) return;
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_useGlobalReadoutSettings )
    {
        updateReadOutSettingsInSubPlots();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    if ( changedChildField == &m_readoutSettings )
    {
        updateReadOutSettingsInSubPlots();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::updateReadOutSettingsInSubPlots()
{
    for ( auto plot : m_summaryMultiPlots )
    {
        plot->updateReadOutSettings();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    for ( auto& plot : m_summaryMultiPlots() )
    {
        uiTreeOrdering.add( plot );
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::onChildrenUpdated( caf::PdmChildArrayFieldHandle*      childArray,
                                                       std::vector<caf::PdmObjectHandle*>& updatedObjects )
{
    if ( childArray == &m_summaryMultiPlots )
    {
        RimTools::updateViewWindowContent( updatedObjects );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_useGlobalReadoutSettings );

    auto readOutGroup = uiOrdering.addNewGroup( "Mouse Cursor Readout" );
    m_readoutSettings->uiOrdering( uiConfigName, *readOutGroup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::summaryPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const
{
    for ( RimSummaryMultiPlot* multiPlot : multiPlots() )
    {
        auto mainPlotName = multiPlot->description();

        for ( RimSummaryPlot* plot : multiPlot->summaryPlots() )
        {
            QString displayName = mainPlotName + " : ";
            displayName += plot->userDescriptionField()->uiCapability()->uiValue().toString();
            optionInfos->push_back( caf::PdmOptionItemInfo( displayName, plot, false, plot->uiIconProvider() ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::duplicatePlot( RimSummaryMultiPlot* plotToDuplicate )
{
    if ( !plotToDuplicate ) return;

    auto plotCopy = plotToDuplicate->copyObject<RimSummaryMultiPlot>();
    addSummaryMultiPlot( plotCopy );

    plotCopy->resolveReferencesRecursively();
    plotCopy->initAfterReadRecursively();
    plotCopy->updateAllRequiredEditors();
    plotCopy->loadDataAndUpdate();

    updateConnectedEditors();

    RiuPlotMainWindowTools::selectAsCurrentItem( plotCopy );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::updateSummaryNameHasChanged()
{
    for ( RimSummaryMultiPlot* multiPlot : multiPlots() )
    {
        for ( RimSummaryPlot* plot : multiPlot->summaryPlots() )
        {
            plot->updateCaseNameHasChanged();
        }

        multiPlot->updatePlotTitles();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RimSummaryPlotReadOut*> RimSummaryMultiPlotCollection::globalReadOutSettings() const
{
    if ( m_useGlobalReadoutSettings() )
    {
        return m_readoutSettings();
    }
    return std::nullopt;
}
