/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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
#include "RimCorrelationReportPlot.h"

#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaSummaryCurveDefinition.h"

#include "RimCorrelationMatrixPlot.h"
#include "RimParameterResultCrossPlot.h"
#include "RimRegularLegendConfig.h"

#include "RiuMultiPlotPage.h"

#include "cafAssert.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeOrdering.h"

#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QStringList>

//==================================================================================================
//
//
//
//==================================================================================================
CAF_PDM_SOURCE_INIT( RimCorrelationReportPlot, "CorrelationReportPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationReportPlot::RimCorrelationReportPlot()
{
    CAF_PDM_InitObject( "Correlation Report Plot", ":/CorrelationReportPlot16x16.png", "", "" );
    this->setDeletable( true );

    CAF_PDM_InitFieldNoDefault( &m_plotWindowTitle, "PlotWindowTitle", "Title", "", "", "" );
    m_plotWindowTitle.registerGetMethod( this, &RimCorrelationReportPlot::createPlotWindowTitle );

    CAF_PDM_InitFieldNoDefault( &m_correlationMatrixPlot, "MatrixPlot", "Matrix Plot", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_correlationPlot, "CorrelationPlot", "Correlation Plot", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_parameterResultCrossPlot, "CrossPlot", "Cross Plot", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_subTitleFontSize, "SubTitleFontSize", "Sub Plot Title Font Size", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_labelFontSize, "LabelFontSize", "Label Font Size", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_axisValueFontSize, "AxisValueFontSize", "Axis Value Font Size", "", "", "" );

    setAsPlotMdiWindow();

    m_showWindow      = true;
    m_showPlotLegends = false;

    m_titleFontSize     = caf::FontTools::RelativeSize::XLarge;
    m_subTitleFontSize  = caf::FontTools::RelativeSize::Large;
    m_labelFontSize     = caf::FontTools::RelativeSize::XSmall;
    m_axisTitleFontSize = caf::FontTools::RelativeSize::Small;
    m_axisValueFontSize = caf::FontTools::RelativeSize::XSmall;

    m_correlationMatrixPlot = new RimCorrelationMatrixPlot;
    m_correlationMatrixPlot->setLegendsVisible( false );
    m_correlationMatrixPlot->setColSpan( RimPlot::TWO );
    m_correlationMatrixPlot->setRowSpan( RimPlot::TWO );

    m_correlationPlot = new RimCorrelationPlot;
    m_correlationPlot->setLegendsVisible( false );

    m_parameterResultCrossPlot = new RimParameterResultCrossPlot;
    m_parameterResultCrossPlot->setLegendsVisible( true );

    this->uiCapability()->setUiTreeChildrenHidden( true );

    m_correlationMatrixPlot->matrixCellSelected.connect( this, &RimCorrelationReportPlot::onDataSelection );
    m_correlationPlot->tornadoItemSelected.connect( this, &RimCorrelationReportPlot::onDataSelection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationReportPlot::~RimCorrelationReportPlot()
{
    removeMdiWindowFromMdiArea();
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimCorrelationReportPlot::viewWidget()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCorrelationReportPlot::description() const
{
    return m_plotWindowTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimCorrelationReportPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_viewer )
    {
        QPixmap pix( m_viewer->size() );
        m_viewer->renderTo( &pix );
        image = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCorrelationReportPlot::userDescriptionField()
{
    return &m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationMatrixPlot* RimCorrelationReportPlot::matrixPlot() const
{
    return m_correlationMatrixPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlot* RimCorrelationReportPlot::correlationPlot() const
{
    return m_correlationPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterResultCrossPlot* RimCorrelationReportPlot::crossPlot() const
{
    return m_parameterResultCrossPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCorrelationReportPlot::columnCount() const
{
    return 3;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCorrelationReportPlot::subTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_subTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCorrelationReportPlot::axisTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCorrelationReportPlot::axisValueFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisValueFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCorrelationReportPlot::createPlotWindowTitle() const
{
    QStringList ensembles;
    for ( auto entry : m_correlationMatrixPlot->curveDefinitions() )
    {
        if ( entry.ensemble() )
        {
            ensembles.push_back( entry.ensemble()->name() );
        }
    }
    ensembles.removeDuplicates();
    QString ensembleNames = ensembles.join( ", " );
    QString timeStep      = m_correlationMatrixPlot->timeStepString();

    return QString( "Correlation Report for %1 at %2" ).arg( ensembleNames ).arg( timeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::recreatePlotWidgets()
{
    CAF_ASSERT( m_viewer );
    m_correlationMatrixPlot->createPlotWidget();
    m_correlationPlot->createPlotWidget();
    m_parameterResultCrossPlot->createPlotWidget();

    m_viewer->addPlot( m_correlationMatrixPlot->viewer() );
    m_viewer->addPlot( m_correlationPlot->viewer() );
    m_viewer->addPlot( m_parameterResultCrossPlot->viewer() );

    m_viewer->scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::cleanupBeforeClose()
{
    m_correlationMatrixPlot->detachAllCurves();
    m_correlationPlot->detachAllCurves();
    m_parameterResultCrossPlot->detachAllCurves();

    if ( m_viewer )
    {
        m_viewer->setParent( nullptr );
        delete m_viewer;
        m_viewer = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( m_viewer )
    {
        m_viewer->renderTo( paintDevice );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimCorrelationReportPlot::createViewWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    m_viewer = new RiuMultiPlotPage( this, mainWindowParent );
    m_viewer->setPlotTitle( m_plotWindowTitle() );
    recreatePlotWidgets();

    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();
    if ( m_showWindow )
    {
        auto timeStep                 = m_correlationMatrixPlot->timeStep().toTime_t();
        bool showOnlyTopNCorrelations = m_correlationMatrixPlot->showTopNCorrelations();
        int  topNFilterCount          = m_correlationMatrixPlot->topNFilterCount();

        m_correlationPlot->setTimeStep( timeStep );
        m_correlationPlot->setShowOnlyTopNCorrelations( showOnlyTopNCorrelations );
        m_correlationPlot->setTopNFilterCount( topNFilterCount );
        m_parameterResultCrossPlot->setTimeStep( timeStep );

        m_correlationMatrixPlot->setLabelFontSize( m_labelFontSize() );
        m_correlationMatrixPlot->setAxisTitleFontSize( m_axisTitleFontSize() );
        m_correlationMatrixPlot->setAxisValueFontSize( m_axisValueFontSize() );

        m_correlationPlot->setLabelFontSize( m_labelFontSize() );
        m_correlationPlot->setAxisTitleFontSize( m_axisTitleFontSize() );
        m_correlationPlot->setAxisValueFontSize( m_axisValueFontSize() );

        m_parameterResultCrossPlot->setLabelFontSize( m_labelFontSize() );
        m_parameterResultCrossPlot->setLegendFontSize( m_legendFontSize() );
        m_parameterResultCrossPlot->setAxisTitleFontSize( m_axisTitleFontSize() );
        m_parameterResultCrossPlot->setAxisValueFontSize( m_axisValueFontSize() );

        m_correlationPlot->setShowAbsoluteValues( m_correlationMatrixPlot->showAbsoluteValues() );
        m_correlationPlot->setSortByAbsoluteValues( m_correlationMatrixPlot->sortByAbsoluteValues() );

        m_correlationMatrixPlot->loadDataAndUpdate();
        m_correlationPlot->loadDataAndUpdate();
        m_parameterResultCrossPlot->loadDataAndUpdate();
        m_viewer->setPlotTitle( m_plotWindowTitle() );
    }
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_correlationMatrixPlot->uiOrdering( "report", uiOrdering );
    auto plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->add( &m_titleFontSize );
    plotGroup->add( &m_subTitleFontSize );
    plotGroup->add( &m_labelFontSize );
    plotGroup->add( &m_legendFontSize );
    plotGroup->add( &m_axisTitleFontSize );
    plotGroup->add( &m_axisValueFontSize );
    m_correlationMatrixPlot->legendConfig()->uiOrdering( "ColorsOnly", *plotGroup );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
    this->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    this->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationReportPlot::doUpdateLayout()
{
    if ( m_showWindow && m_viewer )
    {
        m_viewer->setTitleFontSizes( titleFontSize(), subTitleFontSize() );
        m_viewer->setLegendFontSize( legendFontSize() );
        m_viewer->setAxisFontSizes( axisTitleFontSize(), axisValueFontSize() );
        m_viewer->setSubTitlesVisible( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimCorrelationReportPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotWindow::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_subTitleFontSize || fieldNeedingOptions == &m_labelFontSize ||
         fieldNeedingOptions == &m_axisTitleFontSize || fieldNeedingOptions == &m_axisValueFontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() );
    }
    return options;
}

void RimCorrelationReportPlot::onDataSelection( const caf::SignalEmitter*                     emitter,
                                                std::pair<QString, RiaSummaryCurveDefinition> parameterAndCurveDef )
{
    auto paramName = parameterAndCurveDef.first;
    auto curveDef  = parameterAndCurveDef.second;

    m_correlationPlot->setCurveDefinitions( {curveDef} );
    m_correlationPlot->loadDataAndUpdate();
    m_parameterResultCrossPlot->setCurveDefinitions( {curveDef} );
    m_parameterResultCrossPlot->setEnsembleParameter( paramName );
    m_parameterResultCrossPlot->loadDataAndUpdate();
    if ( m_viewer )
    {
        m_viewer->updateSubTitles();
    }
}
