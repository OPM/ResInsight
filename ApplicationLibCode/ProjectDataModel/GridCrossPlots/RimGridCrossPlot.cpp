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

#include "RiaDefines.h"
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
#include "RimMultiPlot.h"
#include "RimPlotAxisLogRangeCalculator.h"
#include "RimPlotAxisProperties.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cafProgressInfo.h"
#include "cvfAssert.h"

#include "qwt_legend.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"

#include <QDebug>

CAF_PDM_SOURCE_INIT( RimGridCrossPlot, "RimGridCrossPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlot::RimGridCrossPlot()
{
    CAF_PDM_InitObject( "Grid Cross Plot", ":/SummaryXPlotLight16x16.png", "", "" );

    CAF_PDM_InitField( &m_showInfoBox, "ShowInfoBox", true, "Show Info Box" );

    CAF_PDM_InitFieldNoDefault( &m_nameConfig, "NameConfig", "Name Config" );
    m_nameConfig.uiCapability()->setUiTreeHidden( true );
    m_nameConfig.uiCapability()->setUiTreeChildrenHidden( true );
    m_nameConfig = new RimGridCrossPlotNameConfig();

    CAF_PDM_InitFieldNoDefault( &m_xAxisProperties, "xAxisProperties", "X Axis" );
    m_xAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_xAxisProperties = new RimPlotAxisProperties;
    m_xAxisProperties->setNameAndAxis( "X-Axis", RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
    m_xAxisProperties->setEnableTitleTextSettings( false );

    CAF_PDM_InitFieldNoDefault( &m_yAxisProperties, "yAxisProperties", "Y Axis" );
    m_yAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_yAxisProperties = new RimPlotAxisProperties;
    m_yAxisProperties->setNameAndAxis( "Y-Axis", RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    m_yAxisProperties->setEnableTitleTextSettings( false );

    connectAxisSignals( m_xAxisProperties() );
    connectAxisSignals( m_yAxisProperties() );

    CAF_PDM_InitFieldNoDefault( &m_crossPlotDataSets, "CrossPlotCurve", "Cross Plot Data Set" );
    m_crossPlotDataSets.uiCapability()->setUiTreeHidden( true );

    setDeletable( true );
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
RiuPlotWidget* RimGridCrossPlot::plotWidget()
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

    updateZoomInParentPlot();
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
                dataSet->setParentPlotNoReplot( m_plotWidget );
            }
        }
        updateZoomInParentPlot();
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
                m_infoBox =
                    new RiuDraggableOverlayFrame( m_plotWidget->qwtPlot()->canvas(), m_plotWidget->overlayMargins() );
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
caf::PdmObject* RimGridCrossPlot::findPdmObjectFromPlotCurve( const RiuPlotCurve* plotCurve ) const
{
    for ( auto dataSet : m_crossPlotDataSets )
    {
        for ( auto curve : dataSet->curves() )
        {
            if ( curve->isSameCurve( plotCurve ) )
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
void RimGridCrossPlot::connectAxisSignals( RimPlotAxisProperties* axis )
{
    axis->settingsChanged.connect( this, &RimGridCrossPlot::axisSettingsChanged );
    axis->logarithmicChanged.connect( this, &RimGridCrossPlot::axisLogarithmicChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::axisSettingsChanged( const caf::SignalEmitter* emitter )
{
    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::onPlotZoomed()
{
    setAutoScaleXEnabled( false );
    setAutoScaleYEnabled( false );
    updateZoomFromParentPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimGridCrossPlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuGridCrossQwtPlot( this, mainWindowParent );

        for ( auto dataSet : m_crossPlotDataSets )
        {
            dataSet->setParentPlotNoReplot( m_plotWidget );
        }

        updateCurveNamesAndPlotTitle();

        this->connect( m_plotWidget, SIGNAL( plotZoomed() ), SLOT( onPlotZoomed() ) );
    }
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
void RimGridCrossPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* generalGroup = uiOrdering.addNewGroup( "Plot Options" );
    generalGroup->add( &m_showInfoBox );

    if ( isMdiWindow() )
    {
        RimPlotWindow::uiOrderingForPlotLayout( uiConfigName, *generalGroup );
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
    else if ( changedField == &m_showInfoBox )
    {
        updateLayout();
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

    updateAxisInQwt( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
    updateAxisInQwt( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );

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
        RiuQwtPlotTools::setCommonPlotBehaviour( m_plotWidget->qwtPlot() );
        RiuQwtPlotTools::setDefaultAxes( m_plotWidget->qwtPlot() );

        updateFonts();
        updateAxes();

        for ( auto dataSet : m_crossPlotDataSets )
        {
            dataSet->setParentPlotNoReplot( m_plotWidget );
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
        QString plotTitle = this->createAutoName();
        m_plotWidget->setPlotTitle( plotTitle );
        m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && !isSubPlot() );
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

    QString              tmpName = xAxisProperties->name();
    RiaDefines::PlotAxis tmpAxis = xAxisProperties->plotAxisType();
    xAxisProperties->setNameAndAxis( yAxisProperties->name(), yAxisProperties->plotAxisType() );
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
    if ( m_plotWidget )
    {
        m_plotWidget->setInternalQwtLegendVisible( legendsVisible() );
        m_plotWidget->setLegendFontSize( legendFontSize() );
        for ( auto dataSet : m_crossPlotDataSets )
        {
            dataSet->updateLegendIcons();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateZoomInParentPlot()
{
    if ( m_plotWidget )
    {
        updateAxisInQwt( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
        updateAxisInQwt( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
        m_plotWidget->qwtPlot()->updateAxes();
        updateZoomFromParentPlot();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateZoomFromParentPlot()
{
    updateAxisFromQwt( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    updateAxisFromQwt( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
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

    if ( axisType == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
    {
        axisProperties      = m_yAxisProperties();
        axisParameterString = yAxisParameterString();
    }

    RiaDefines::PlotAxis axis = axisProperties->plotAxisType();
    if ( axisProperties->isActive() )
    {
        m_plotWidget->enableAxis( axis, true );

        Qt::AlignmentFlag alignment = Qt::AlignCenter;
        if ( axisProperties->titlePosition() == RimPlotAxisPropertiesInterface::AXIS_TITLE_END )
        {
            alignment = Qt::AlignRight;
        }
        m_plotWidget->setAxisFontsAndAlignment( axis,
                                                caf::FontTools::pointSizeToPixelSize( axisProperties->titleFontSize() ),
                                                caf::FontTools::pointSizeToPixelSize( axisProperties->valuesFontSize() ),
                                                true,
                                                alignment );
        m_plotWidget->setAxisTitleText( axis, axisParameterString );
        m_plotWidget->setAxisTitleEnabled( axis, true );

        if ( axisProperties->isLogarithmicScaleEnabled )
        {
            bool isLogScale = m_plotWidget->axisScaleType( axis ) == RiuQwtPlotWidget::AxisScaleType::LOGARITHMIC;
            if ( !isLogScale )
            {
                m_plotWidget->setAxisScaleType( axis, RiuQwtPlotWidget::AxisScaleType::LOGARITHMIC );
                m_plotWidget->setAxisMaxMinor( axis, 5 );
            }

            double min = axisProperties->visibleRangeMin;
            double max = axisProperties->visibleRangeMax;
            if ( axisProperties->isAutoZoom() )
            {
                // TODO: fix this!!
                // std::vector<const QwtPlotCurve*> plotCurves = visibleCurves();

                // RimPlotAxisLogRangeCalculator logRangeCalculator( qwtAxisId, plotCurves );
                // logRangeCalculator.computeAxisRange( &min, &max );
            }

            if ( axisProperties->isAxisInverted() )
            {
                std::swap( min, max );
            }
            m_plotWidget->setAxisScale( axis, min, max );
        }
        else
        {
            bool isLinearScale = m_plotWidget->axisScaleType( axis ) == RiuQwtPlotWidget::AxisScaleType::LINEAR;
            if ( !isLinearScale )
            {
                m_plotWidget->setAxisScaleType( axis, RiuQwtPlotWidget::AxisScaleType::LINEAR );
                m_plotWidget->setAxisMaxMinor( axis, 3 );
            }

            if ( axisProperties->isAutoZoom() )
            {
                m_plotWidget->setAxisAutoScale( axis, true );
                m_plotWidget->setAxisInverted( axis, axisProperties->isAxisInverted() );
            }
            else
            {
                double min = axisProperties->visibleRangeMin;
                double max = axisProperties->visibleRangeMax;
                if ( axisProperties->isAxisInverted() )
                {
                    std::swap( min, max );
                }

                m_plotWidget->setAxisScale( axis, min, max );
            }
        }
    }
    else
    {
        m_plotWidget->enableAxis( axis, false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateAxisFromQwt( RiaDefines::PlotAxis axisType )
{
    if ( !m_plotWidget ) return;

    auto [xAxisRangeMin, xAxisRangeMax] = m_plotWidget->axisRange( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );

    RimPlotAxisProperties* axisProperties = m_xAxisProperties();
    double                 axisRangeMin   = xAxisRangeMin;
    double                 axisRangeMax   = xAxisRangeMax;

    if ( axisType == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
    {
        axisProperties                      = m_yAxisProperties();
        auto [yAxisRangeMin, yAxisRangeMax] = m_plotWidget->axisRange( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );

        axisRangeMin = yAxisRangeMin;
        axisRangeMax = yAxisRangeMax;
    }

    axisProperties->visibleRangeMin = std::min( axisRangeMin, axisRangeMax );
    axisProperties->visibleRangeMax = std::max( axisRangeMin, axisRangeMax );

    axisProperties->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RimPlotCurve*> RimGridCrossPlot::visibleCurves() const
{
    std::vector<const RimPlotCurve*> plotCurves;
    for ( auto dataSet : m_crossPlotDataSets )
    {
        if ( dataSet->isChecked() )
        {
            for ( auto curve : dataSet->curves() )
            {
                if ( curve->isCurveVisible() )
                {
                    plotCurves.push_back( curve );
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
    return { m_xAxisProperties, m_yAxisProperties };
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
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlot::isDeletable() const
{
    RimMultiPlot* plotWindow = nullptr;
    firstAncestorOrThisOfType( plotWindow );
    return plotWindow == nullptr;
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
    CAF_PDM_InitObject( "Cross Plot Name Generator" );

    CAF_PDM_InitField( &addDataSetNames, "AddDataSetNames", true, "Add Data Set Names" );
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
