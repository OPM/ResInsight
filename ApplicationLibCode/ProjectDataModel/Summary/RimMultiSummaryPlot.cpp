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

#include "RimMultiSummaryPlot.h"
#include "RimMultiPlot.h"

#include "RimMainPlotCollection.h"
#include "RimMultiPlotCollection.h"
#include "RimProject.h"
#include "RiuSummaryVectorSelectionUi.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimMultiSummaryPlot, "MultiSummaryPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiSummaryPlot::RimMultiSummaryPlot()
{
    CAF_PDM_InitObject( "Multi Summary Plot Plot", "", "", "" );
    this->setDeletable( true );

    CAF_PDM_InitField( &m_individualPlotPerVector, "IndividualPlotPerVector", false, "One plot per Vector" );
    CAF_PDM_InitField( &m_individualPlotPerDataSource, "IndividualPlotPerDataSource", false, "One plot per Data Source" );

    CAF_PDM_InitFieldNoDefault( &m_selectedSources, "SummaryCases", "Cases" );
    m_selectedSources.uiCapability()->setAutoAddingOptionFromValue( false );
    m_selectedSources.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedSources.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_multiPlot, "MultiPlot", "Multi Plot" );

    CAF_PDM_InitFieldNoDefault( &m_addressCandidates, "AddressCandidates", "Vectors" );
    m_addressCandidates.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    m_multiPlot = new RimMultiPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiSummaryPlot::~RimMultiSummaryPlot()
{
    removeMdiWindowFromMdiArea();
    m_multiPlot->cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimMultiSummaryPlot::viewWidget()
{
    return m_multiPlot->viewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimMultiSummaryPlot::snapshotWindowContent()
{
    return m_multiPlot->snapshotWindowContent();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiSummaryPlot::zoomAll()
{
    m_multiPlot->zoomAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMultiSummaryPlot::description() const
{
    return "RimMultiSummaryPlot Placeholder Text";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiSummaryPlot::addPlot( RimPlot* plot )
{
    m_multiPlot->addPlot( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiSummaryPlot* RimMultiSummaryPlot::createAndAppendMultiPlot( const std::vector<RimPlot*>& plots )
{
    RimProject* project        = RimProject::current();
    auto*       plotCollection = project->mainPlotCollection()->multiPlotCollection();

    auto* plotWindow = new RimMultiSummaryPlot;
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addMultiSummaryPlot( plotWindow );

    for ( auto plot : plots )
    {
        plotWindow->addPlot( plot );

        plot->resolveReferencesRecursively();
        plot->revokeMdiWindowStatus();
        plot->setShowWindow( true );

        plot->loadDataAndUpdate();
    }

    plotCollection->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();

    return plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimMultiSummaryPlot::createViewWidget( QWidget* mainWindowParent /*= nullptr*/ )
{
    m_multiPlot->revokeMdiWindowStatus();

    return m_multiPlot->createViewWidget( mainWindowParent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiSummaryPlot::deleteViewWidget()
{
    m_multiPlot->deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiSummaryPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    m_multiPlot->onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiSummaryPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    m_multiPlot->doRenderWindowContent( paintDevice );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimMultiSummaryPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                          bool*                      useOptionsOnly )
{
    if ( fieldNeedingOptions == &m_selectedSources )
    {
        // 		static QList<caf::PdmOptionItemInfo>
        // 			optionsForSummaryDataSource(bool hideSummaryCases, bool hideEnsembles, bool
        // showIndividualEnsembleCases);

        bool hideSummaryCases            = false;
        bool hideEnsembles               = false;
        bool showIndividualEnsembleCases = false;

        auto optionsForDataSource = RiuSummaryVectorSelectionUi::optionsForSummaryDataSource( hideSummaryCases,
                                                                                              hideEnsembles,
                                                                                              showIndividualEnsembleCases );

        return optionsForDataSource;
    }

    QList<caf::PdmOptionItemInfo> options;
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiSummaryPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_selectedSources );
    uiOrdering.add( &m_addressCandidates );
    uiOrdering.add( &m_individualPlotPerVector );
    uiOrdering.add( &m_individualPlotPerDataSource );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultiSummaryPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showWindow && m_showWindow() )
    {
        // Plots contained in a RimMultiPlot will automatically be set invisible
        // Restore plot visibility

        for ( auto p : m_multiPlot->plots() )
        {
            p->setShowWindow( true );
        }
    }
}
