/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RimGridCrossPlot.h"

#include "RiaApplication.h"
#include "RiaFontCache.h"
#include "RiaPreferences.h"

#include "RifTextDataTableFormatter.h"
#include "RiuDraggableOverlayFrame.h"
#include "RiuGridCrossQwtPlot.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotTools.h"

#include "RimGridCrossPlotCollection.h"
#include "RimGridCrossPlotCurve.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimMultiPlotWindow.h"
#include "RimPlotAxisProperties.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cafProgressInfo.h"
#include "cvfAssert.h"

#include "qwt_legend.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_engine.h"

#include <QDebug>

CAF_PDM_SOURCE_INIT( RimGridCrossPlot, "RimGridCrossPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlot::RimGridCrossPlot()
{
    CAF_PDM_InitObject( "Grid Cross Plot", ":/SummaryXPlotLight16x16.png", "", "" );

    CAF_PDM_InitField( &m_showInfoBox, "ShowInfoBox", true, "Show Info Box", "", "", "" );
    CAF_PDM_InitField( &m_showLegend_OBSOLETE, "ShowLegend", false, "Show Legend", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_nameConfig, "NameConfig", "Name Config", "", "", "" );
    m_nameConfig.uiCapability()->setUiTreeHidden( true );
    m_nameConfig.uiCapability()->setUiTreeChildrenHidden( true );
    m_nameConfig = new RimGridCrossPlotNameConfig();

    CAF_PDM_InitFieldNoDefault( &m_xAxisProperties, "xAxisProperties", "X Axis", "", "", "" );
    m_xAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_xAxisProperties = new RimPlotAxisProperties;
    m_xAxisProperties->setNameAndAxis( "X-Axis", QwtPlot::xBottom );
    m_xAxisProperties->setEnableTitleTextSettings( false );

    CAF_PDM_InitFieldNoDefault( &m_yAxisProperties, "yAxisProperties", "Y Axis", "", "", "" );
    m_yAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_yAxisProperties = new RimPlotAxisProperties;
    m_yAxisProperties->setNameAndAxis( "Y-Axis", QwtPlot::yLeft );
    m_yAxisProperties->setEnableTitleTextSettings( false );

    CAF_PDM_InitFieldNoDefault( &m_crossPlotDataSets, "CrossPlotCurve", "Cross Plot Data Set", "", "", "" );
    m_crossPlotDataSets.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlot::~RimGridCrossPlot()
{
    removeMdiWindowFromMdiArea();
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::description() const
{
    return createAutoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotDataSet* RimGridCrossPlot::createDataSet()
{
    RimGridCrossPlotDataSet* dataSet = new RimGridCrossPlotDataSet();
    m_crossPlotDataSets.push_back( dataSet );
    return dataSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCrossPlot::indexOfDataSet( const RimGridCrossPlotDataSet* dataSetToCheck ) const
{
    int index = 0;
    for ( auto dataSet : m_crossPlotDataSets() )
    {
        if ( dataSet == dataSetToCheck )
        {
            return index;
        }
        if ( dataSet->isChecked() && dataSet->visibleCurveCount() > 0u )
        {
            index++;
        }
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::addDataSet( RimGridCrossPlotDataSet* dataSet )
{
    m_crossPlotDataSets.push_back( dataSet );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridCrossPlotDataSet*> RimGridCrossPlot::dataSets() const
{
    return m_crossPlotDataSets.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGridCrossPlot::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimGridCrossPlot::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimGridCrossPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_plotWidget )
    {
        QPixmap pix = m_plotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleYEnabled( true );

    updateZoomInQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::calculateZoomRangeAndUpdateQwt()
{
    if ( m_plotWidget )
    {
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::reattachAllCurves()
{
    if ( m_plotWidget )
    {
        for ( auto dataSet : m_crossPlotDataSets )
        {
            dataSet->detachAllCurves();
            if ( dataSet->isChecked() )
            {
                dataSet->setParentQwtPlotNoReplot( m_plotWidget );
            }
        }
        updateZoomInQwt();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::createAutoName() const
{
    QStringList autoName;
    if ( !m_nameConfig->customName().isEmpty() )
    {
        autoName += m_nameConfig->customName();
    }

    if ( m_nameConfig->addDataSetNames() )
    {
        QStringList                                                          dataSetStrings;
        std::map<RimGridCrossPlotDataSet::NameComponents, std::set<QString>> allNameComponents;
        for ( auto dataSet : m_crossPlotDataSets )
        {
            if ( dataSet->isChecked() )
            {
                QStringList componentList;
                auto        dataSetNameComponents = dataSet->nameComponents();

                for ( auto dataSetNameComponent : dataSetNameComponents )
                {
                    if ( !dataSetNameComponent.second.isEmpty() )
                    {
                        if ( allNameComponents[dataSetNameComponent.first].count( dataSetNameComponent.second ) == 0u )
                        {
                            componentList += dataSetNameComponent.second;
                            allNameComponents[dataSetNameComponent.first].insert( dataSetNameComponent.second );
                        }
                    }
                }
                if ( !componentList.isEmpty() )
                {
                    dataSetStrings += componentList.join( ", " );
                }
            }
        }

        dataSetStrings.removeDuplicates();
        if ( dataSetStrings.size() > 2 )
        {
            autoName += QString( "(%1 Data Sets)" ).arg( dataSetStrings.size() );
        }
        if ( !dataSetStrings.isEmpty() )
        {
            autoName += QString( "(%1)" ).arg( dataSetStrings.join( "; " ) );
        }
    }

    return autoName.join( " " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlot::showInfoBox() const
{
    return m_showInfoBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateInfoBox()
{
    if ( m_plotWidget )
    {
        if ( m_showInfoBox )
        {
            if ( !m_infoBox )
            {
                m_infoBox = new RiuDraggableOverlayFrame( m_plotWidget->canvas(), m_plotWidget->overlayMargins() );
                m_infoBox->setAnchorCorner( RiuDraggableOverlayFrame::AnchorCorner::TopRight );
                RiuTextOverlayContentFrame* textFrame = new RiuTextOverlayContentFrame( m_infoBox );
                textFrame->setText( generateInfoBoxText() );
                m_infoBox->setContentFrame( textFrame );
            }
            m_plotWidget->addOverlayFrame( m_infoBox );
        }
        else
        {
            if ( m_plotWidget && m_infoBox )
            {
                m_plotWidget->removeOverlayFrame( m_infoBox );
                delete m_infoBox;
                m_infoBox = nullptr;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGridCrossPlot::userDescriptionField()
{
    return m_nameConfig->nameField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::detachAllCurves()
{
    for ( auto dataSet : m_crossPlotDataSets() )
    {
        dataSet->detachAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::setAutoScaleXEnabled( bool enabled )
{
    m_xAxisProperties->setAutoZoom( enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::setAutoScaleYEnabled( bool enabled )
{
    m_yAxisProperties->setAutoZoom( enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimGridCrossPlot::findPdmObjectFromQwtCurve( const QwtPlotCurve* qwtCurve ) const
{
    for ( auto dataSet : m_crossPlotDataSets )
    {
        for ( auto curve : dataSet->curves() )
        {
            if ( curve->qwtPlotCurve() == qwtCurve )
            {
                return curve;
            }
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::onAxisSelected( int axis, bool toggle )
{
    RiuPlotMainWindowTools::showPlotMainWindow();
    RimPlotAxisProperties* properties = nullptr;
    if ( axis == QwtPlot::yLeft )
    {
        properties = m_yAxisProperties;
    }
    else if ( axis == QwtPlot::xBottom )
    {
        properties = m_xAxisProperties;
    }

    if ( toggle )
    {
        RiuPlotMainWindowTools::toggleItemInSelection( properties );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( properties );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::doRemoveFromCollection()
{
    RimGridCrossPlotCollection* crossPlotCollection = nullptr;
    this->firstAncestorOrThisOfType( crossPlotCollection );
    if ( crossPlotCollection )
    {
        crossPlotCollection->removeGridCrossPlot( this );
        crossPlotCollection->updateAllRequiredEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::generateInfoBoxText() const
{
    QStringList curveInfoTexts;
    for ( auto dataSet : dataSets() )
    {
        QString curveInfoText = dataSet->infoText();
        if ( dataSet->isChecked() && !curveInfoText.isEmpty() )
        {
            curveInfoTexts += curveInfoText;
        }
    }
    QStringList infoText;
    infoText << QString( "<b>View ID:</b> %1<br/>" ).arg( id() );
    if ( curveInfoTexts.size() > 1 )
    {
        infoText += QString( "<ol style=\"margin-top: 0px; margin-left: 15px; -qt-list-indent:0;\">" );
        for ( QString curveInfoText : curveInfoTexts )
        {
            infoText += QString( "<li>%1</li>" ).arg( curveInfoText );
        }
        infoText += QString( "</ol>" );
    }
    else if ( curveInfoTexts.size() > 0 )
    {
        infoText += curveInfoTexts.front();
    }
    return infoText.join( "\n" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGridCrossPlot::createViewWidget( QWidget* mainWindowParent )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuGridCrossQwtPlot( this, mainWindowParent );

        for ( auto dataSet : m_crossPlotDataSets )
        {
            dataSet->setParentQwtPlotNoReplot( m_plotWidget );
        }
    }

    m_plotWidget->scheduleReplot();
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    for ( auto dataSet : m_crossPlotDataSets )
    {
        dataSet->loadDataAndUpdate( false );
        dataSet->updateConnectedEditors();
    }

    updateCurveNamesAndPlotTitle();
    updatePlot();
    updateInfoBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::initAfterRead()
{
    if ( m_showLegend_OBSOLETE() )
    {
        setLegendsVisible( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* generalGroup = uiOrdering.addNewGroup( "Plot Options" );
    generalGroup->add( &m_showInfoBox );

    if ( isMdiWindow() )
    {
        RimPlotWindow::uiOrderingForLegendSettings( uiConfigName, *generalGroup );
    }
    else
    {
        generalGroup->add( &m_rowSpan );
        generalGroup->add( &m_colSpan );
    }

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Name Configuration" );
    m_nameConfig->uiOrdering( uiConfigName, *nameGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    caf::PdmUiTreeOrdering* axisFolder = uiTreeOrdering.add( "Axes", ":/Axes16x16.png" );

    axisFolder->add( &m_xAxisProperties );
    axisFolder->add( &m_yAxisProperties );

    uiTreeOrdering.add( &m_crossPlotDataSets );

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_colSpan || changedField == &m_rowSpan )
    {
        updateParentLayout();
    }
    else
    {
        onLoadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::performAutoNameUpdate()
{
    updateCurveNamesAndPlotTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    updateAxisInQwt( RiaDefines::PLOT_AXIS_BOTTOM );
    updateAxisInQwt( RiaDefines::PLOT_AXIS_LEFT );

    m_plotWidget->updateAnnotationObjects( m_xAxisProperties );
    m_plotWidget->updateAnnotationObjects( m_yAxisProperties );

    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updatePlot()
{
    if ( m_plotWidget )
    {
        RiuQwtPlotTools::setCommonPlotBehaviour( m_plotWidget );
        RiuQwtPlotTools::setDefaultAxes( m_plotWidget );

        updateAxes();

        for ( auto dataSet : m_crossPlotDataSets )
        {
            dataSet->setParentQwtPlotNoReplot( m_plotWidget );
        }

        updateLegend();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateCurveNamesAndPlotTitle()
{
    for ( size_t i = 0; i < m_crossPlotDataSets.size(); ++i )
    {
        m_crossPlotDataSets[i]->updateCurveNames( i, m_crossPlotDataSets.size() );
    }

    if ( m_plotWidget )
    {
        if ( isMdiWindow() )
        {
            m_plotWidget->setTitle( this->createAutoName() );
        }
    }
    updateMdiWindowTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::swapAxes()
{
    RimPlotAxisProperties* xAxisProperties = m_xAxisProperties();
    RimPlotAxisProperties* yAxisProperties = m_yAxisProperties();

    QString       tmpName = xAxisProperties->name();
    QwtPlot::Axis tmpAxis = xAxisProperties->qwtPlotAxisType();
    xAxisProperties->setNameAndAxis( yAxisProperties->name(), yAxisProperties->qwtPlotAxisType() );
    yAxisProperties->setNameAndAxis( tmpName, tmpAxis );

    m_xAxisProperties.removeChildObject( xAxisProperties );
    m_yAxisProperties.removeChildObject( yAxisProperties );
    m_yAxisProperties = xAxisProperties;
    m_xAxisProperties = yAxisProperties;

    for ( auto dataSet : m_crossPlotDataSets )
    {
        dataSet->swapAxisProperties( false );
    }

    loadDataAndUpdate();

    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::asciiDataForPlotExport() const
{
    QString fullData;
    for ( int i = 0; i < (int)m_crossPlotDataSets.size(); ++i )
    {
        fullData += asciiTitleForPlotExport( i ) + "\n";
        fullData += asciiDataForGridCrossPlotExport( i ) + "\n";
    }
    return fullData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::asciiTitleForPlotExport( int dataSetIndex ) const
{
    if ( (size_t)dataSetIndex < m_crossPlotDataSets.size() )
    {
        return m_crossPlotDataSets[dataSetIndex]->createAutoName();
    }
    return "Data invalid";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::asciiDataForGridCrossPlotExport( int dataSetIndex ) const
{
    if ( (size_t)dataSetIndex < m_crossPlotDataSets.size() )
    {
        QString     asciiData;
        QTextStream stringStream( &asciiData );

        RifTextDataTableFormatter formatter( stringStream );
        formatter.setCommentPrefix( "" );
        formatter.setHeaderPrefix( "" );
        formatter.setTableRowPrependText( "" );
        formatter.setTableRowLineAppendText( "" );
        formatter.setColumnSpacing( 3 );

        m_crossPlotDataSets[dataSetIndex]->exportFormattedData( formatter );
        formatter.tableCompleted();
        return asciiData;
    }
    else
    {
        return "Data invalid and may have been deleted.";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlot::isXAxisLogarithmic() const
{
    return m_xAxisProperties->isLogarithmicScaleEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlot::isYAxisLogarithmic() const
{
    return m_yAxisProperties->isLogarithmicScaleEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::setYAxisInverted( bool inverted )
{
    m_yAxisProperties->setAxisInverted( inverted );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlot::hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const
{
    if ( fontSettingType != RiaDefines::PLOT_FONT ) return false;

    for ( auto plotAxis : allPlotAxes() )
    {
        if ( plotAxis->titleFontSize() != defaultFontSize || plotAxis->valuesFontSize() != defaultFontSize )
        {
            return true;
        }
    }

    if ( legendFontSize() != defaultFontSize )
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlot::applyFontSize( RiaDefines::FontSettingType fontSettingType,
                                      int                         oldFontSize,
                                      int                         fontSize,
                                      bool                        forceChange /*= false*/ )
{
    bool anyChange = false;
    if ( fontSettingType == RiaDefines::PLOT_FONT && m_plotWidget )
    {
        for ( auto plotAxis : allPlotAxes() )
        {
            if ( forceChange || plotAxis->titleFontSize() == oldFontSize )
            {
                plotAxis->setTitleFontSize( fontSize );
                anyChange = true;
            }
            if ( forceChange || plotAxis->valuesFontSize() == oldFontSize )
            {
                plotAxis->setValuesFontSize( fontSize );
                anyChange = true;
            }
        }

        if ( forceChange || legendFontSize() == oldFontSize )
        {
            setLegendFontSize( fontSize );

            anyChange = true;
        }

        if ( anyChange ) loadDataAndUpdate();
    }
    return anyChange;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::doUpdateLayout()
{
    updateLegend();
    updatePlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateLegend()
{
    m_plotWidget->setInternalQwtLegendVisible( legendsVisible() && isMdiWindow() );
    m_plotWidget->setLegendFontSize( legendFontSize() );
    for ( auto dataSet : m_crossPlotDataSets )
    {
        dataSet->updateLegendIcons();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateZoomInQwt()
{
    if ( m_plotWidget )
    {
        updateAxisInQwt( RiaDefines::PLOT_AXIS_LEFT );
        updateAxisInQwt( RiaDefines::PLOT_AXIS_BOTTOM );
        m_plotWidget->updateAxes();
        updateZoomFromQwt();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateZoomFromQwt()
{
    updateAxisFromQwt( RiaDefines::PLOT_AXIS_LEFT );
    updateAxisFromQwt( RiaDefines::PLOT_AXIS_BOTTOM );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::xAxisParameterString() const
{
    QStringList xAxisParams;
    for ( auto dataSet : m_crossPlotDataSets )
    {
        if ( dataSet->isChecked() && dataSet->sampleCount() > 0u )
        {
            xAxisParams.push_back( dataSet->xAxisName() );
        }
    }

    xAxisParams.removeDuplicates();

    if ( xAxisParams.size() > 4 )
    {
        return QString( "%1 parameters" ).arg( xAxisParams.size() );
    }

    return xAxisParams.join( ", " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::yAxisParameterString() const
{
    QStringList yAxisParams;
    for ( auto dataSet : m_crossPlotDataSets )
    {
        if ( dataSet->isChecked() && dataSet->sampleCount() > 0u )
        {
            yAxisParams.push_back( dataSet->yAxisName() );
        }
    }

    yAxisParams.removeDuplicates();

    if ( yAxisParams.size() > 4 )
    {
        return QString( "%1 parameters" ).arg( yAxisParams.size() );
    }

    return yAxisParams.join( ", " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateAxisInQwt( RiaDefines::PlotAxis axisType )
{
    if ( !m_plotWidget ) return;

    RimPlotAxisProperties* axisProperties      = m_xAxisProperties();
    QString                axisParameterString = xAxisParameterString();

    if ( axisType == RiaDefines::PLOT_AXIS_LEFT )
    {
        axisProperties      = m_yAxisProperties();
        axisParameterString = yAxisParameterString();
    }

    QwtPlot::Axis qwtAxisId = axisProperties->qwtPlotAxisType();

    if ( axisProperties->isActive() )
    {
        m_plotWidget->enableAxis( qwtAxisId, true );

        Qt::AlignmentFlag alignment = Qt::AlignCenter;
        if ( axisProperties->titlePosition() == RimPlotAxisPropertiesInterface::AXIS_TITLE_END )
        {
            alignment = Qt::AlignRight;
        }
        m_plotWidget->setAxisFontsAndAlignment( qwtAxisId,
                                                axisProperties->titleFontSize(),
                                                axisProperties->valuesFontSize(),
                                                true,
                                                alignment );
        m_plotWidget->setAxisTitleText( qwtAxisId, axisParameterString );
        m_plotWidget->setAxisTitleEnabled( qwtAxisId, true );

        if ( axisProperties->isLogarithmicScaleEnabled )
        {
            QwtLogScaleEngine* currentScaleEngine = dynamic_cast<QwtLogScaleEngine*>(
                m_plotWidget->axisScaleEngine( axisProperties->qwtPlotAxisType() ) );
            if ( !currentScaleEngine )
            {
                m_plotWidget->setAxisScaleEngine( axisProperties->qwtPlotAxisType(), new QwtLogScaleEngine );
                m_plotWidget->setAxisMaxMinor( axisProperties->qwtPlotAxisType(), 5 );
            }

            if ( axisProperties->isAutoZoom() )
            {
                std::vector<const QwtPlotCurve*> plotCurves = visibleQwtCurves();

                double                        min, max;
                RimPlotAxisLogRangeCalculator logRangeCalculator( qwtAxisId, plotCurves );
                logRangeCalculator.computeAxisRange( &min, &max );
                if ( axisProperties->isAxisInverted() )
                {
                    std::swap( min, max );
                }

                m_plotWidget->setAxisScale( qwtAxisId, min, max );
            }
            else
            {
                m_plotWidget->setAxisScale( qwtAxisId, axisProperties->visibleRangeMin, axisProperties->visibleRangeMax );
            }
        }
        else
        {
            QwtLinearScaleEngine* currentScaleEngine = dynamic_cast<QwtLinearScaleEngine*>(
                m_plotWidget->axisScaleEngine( axisProperties->qwtPlotAxisType() ) );
            if ( !currentScaleEngine )
            {
                m_plotWidget->setAxisScaleEngine( axisProperties->qwtPlotAxisType(), new QwtLinearScaleEngine );
                m_plotWidget->setAxisMaxMinor( axisProperties->qwtPlotAxisType(), 3 );
            }

            if ( axisProperties->isAutoZoom() )
            {
                m_plotWidget->setAxisAutoScale( qwtAxisId );
            }
            else
            {
                m_plotWidget->setAxisScale( qwtAxisId, axisProperties->visibleRangeMin, axisProperties->visibleRangeMax );
            }
        }
        m_plotWidget->axisScaleEngine( axisProperties->qwtPlotAxisType() )
            ->setAttribute( QwtScaleEngine::Inverted, axisProperties->isAxisInverted() );
    }
    else
    {
        m_plotWidget->enableAxis( qwtAxisId, false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateAxisFromQwt( RiaDefines::PlotAxis axisType )
{
    if ( !m_plotWidget ) return;

    QwtInterval xAxisRange = m_plotWidget->axisRange( QwtPlot::xBottom );
    QwtInterval yAxisRange = m_plotWidget->axisRange( QwtPlot::yLeft );

    RimPlotAxisProperties* axisProperties = m_xAxisProperties();
    QwtInterval            axisRange      = xAxisRange;

    if ( axisType == RiaDefines::PLOT_AXIS_LEFT )
    {
        axisProperties = m_yAxisProperties();
        axisRange      = yAxisRange;
    }

    axisProperties->visibleRangeMin = std::min( axisRange.minValue(), axisRange.maxValue() );
    axisProperties->visibleRangeMax = std::max( axisRange.minValue(), axisRange.maxValue() );

    axisProperties->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const QwtPlotCurve*> RimGridCrossPlot::visibleQwtCurves() const
{
    std::vector<const QwtPlotCurve*> plotCurves;
    for ( auto dataSet : m_crossPlotDataSets )
    {
        if ( dataSet->isChecked() )
        {
            for ( auto curve : dataSet->curves() )
            {
                if ( curve->isCurveVisible() )
                {
                    plotCurves.push_back( curve->qwtPlotCurve() );
                }
            }
        }
    }
    return plotCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties* RimGridCrossPlot::xAxisProperties()
{
    return m_xAxisProperties();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties* RimGridCrossPlot::yAxisProperties()
{
    return m_yAxisProperties();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotNameConfig* RimGridCrossPlot::nameConfig()
{
    return m_nameConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::setShowInfoBox( bool enable )
{
    m_showInfoBox = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimPlotAxisPropertiesInterface*> RimGridCrossPlot::allPlotAxes() const
{
    return {m_xAxisProperties, m_yAxisProperties};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::cleanupBeforeClose()
{
    for ( auto dataSet : m_crossPlotDataSets() )
    {
        dataSet->detachAllCurves();
    }

    if ( m_plotWidget )
    {
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// Name Configuration
///
//--------------------------------------------------------------------------------------------------

CAF_PDM_SOURCE_INIT( RimGridCrossPlotNameConfig, "RimGridCrossPlotNameConfig" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotNameConfig::RimGridCrossPlotNameConfig()
    : RimNameConfig( "Cross Plot" )
{
    CAF_PDM_InitObject( "Cross Plot Name Generator", "", "", "" );

    CAF_PDM_InitField( &addDataSetNames, "AddDataSetNames", true, "Add Data Set Names", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotNameConfig::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimNameConfig::defineUiOrdering( uiConfigName, uiOrdering );
    uiOrdering.add( &addDataSetNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotNameConfig::doEnableAllAutoNameTags( bool enable )
{
    addDataSetNames = enable;
}
