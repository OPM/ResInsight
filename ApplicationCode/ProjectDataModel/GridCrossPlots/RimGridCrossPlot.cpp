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

#include "RiaGridCrossPlotCurveNameHelper.h"

#include "RiuSummaryQwtPlot.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotTools.h"

#include "RimGridCrossPlotCurve.h"
#include "RimGridCrossPlotCurveSet.h"
#include "RimPlotAxisProperties.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfAssert.h"

#include "qwt_legend.h"
#include "qwt_scale_engine.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"

#include <QDebug>

CAF_PDM_SOURCE_INIT(RimGridCrossPlot, "RimGridCrossPlot");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlot::RimGridCrossPlot()
{
    CAF_PDM_InitObject("Grid Cross Plot", ":/SummaryXPlotLight16x16.png", "", "");

    CAF_PDM_InitField(&m_showLegend, "ShowLegend", true, "Show Legend", "", "", "");
    CAF_PDM_InitField(&m_legendFontSize, "LegendFontSize", 10, "Legend Font Size", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_nameConfig, "NameConfig", "Name Config", "", "", "");
    m_nameConfig.uiCapability()->setUiTreeHidden(true);
    m_nameConfig.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_xAxisProperties, "xAxisProperties", "X Axis", "", "", "");
    m_xAxisProperties.uiCapability()->setUiTreeHidden(true);
    m_xAxisProperties = new RimPlotAxisProperties;
    m_xAxisProperties->setNameAndAxis("X-Axis", QwtPlot::xBottom);
    m_xAxisProperties->setEnableTitleTextSettings(false);

    CAF_PDM_InitFieldNoDefault(&m_yAxisProperties, "yAxisProperties", "Y Axis", "", "", "");
    m_yAxisProperties.uiCapability()->setUiTreeHidden(true);
    m_yAxisProperties = new RimPlotAxisProperties;
    m_yAxisProperties->setNameAndAxis("Y-Axis", QwtPlot::yLeft);
    m_yAxisProperties->setEnableTitleTextSettings(false);

    CAF_PDM_InitFieldNoDefault(&m_crossPlotCurveSets, "CrossPlotCurve", "Cross Plot Data Set", "", "", "");
    m_crossPlotCurveSets.uiCapability()->setUiHidden(true);

    m_nameConfig        = new RimGridCrossPlotNameConfig(this);    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlot::~RimGridCrossPlot()
{
    removeMdiWindowFromMdiArea();
    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotCurveSet* RimGridCrossPlot::createCurveSet()
{
    RimGridCrossPlotCurveSet* curveSet = new RimGridCrossPlotCurveSet();
    m_crossPlotCurveSets.push_back(curveSet);
    return curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCrossPlot::indexOfCurveSet(const RimGridCrossPlotCurveSet* curveSet) const
{
    for (size_t i = 0; i < m_crossPlotCurveSets.size(); ++i)
    {
        if (curveSet == m_crossPlotCurveSets[i])
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGridCrossPlot::viewWidget()
{
    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimGridCrossPlot::snapshotWindowContent()
{
    QImage image;

    if (m_qwtPlot)
    {
        QPixmap pix = QPixmap::grabWidget(m_qwtPlot);
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::zoomAll()
{
    setAutoZoomForAllAxes(true);
    updateAxisInQwt(RiaDefines::PLOT_AXIS_LEFT);
    updateAxisInQwt(RiaDefines::PLOT_AXIS_BOTTOM);
    
    m_qwtPlot->replot();
    
    updateZoomWindowFromQwt();    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::calculateZoomRangeAndUpdateQwt()
{
    // this->calculateXZoomRange();
    m_qwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::reattachCurvesToQwtAndReplot()
{   
    if (m_qwtPlot)
    {
        for (auto curveSet : m_crossPlotCurveSets)
        {
            curveSet->detachAllCurves();
            if (curveSet->isChecked())
            {
                curveSet->setParentQwtPlotNoReplot(m_qwtPlot);
            }
        }
        m_qwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::createAutoName() const
{
    QStringList autoName;
    if (!m_nameConfig->customName().isEmpty())
    {
        autoName += m_nameConfig->customName();
    }

    if (m_nameConfig->addDataSetNames())
    {
        QStringList dataSets;
        for (auto curveSet : m_crossPlotCurveSets)
        {
            dataSets += curveSet->createAutoName();
        }
        if (!dataSets.isEmpty())
        {
            autoName += QString("(%1)").arg(dataSets.join("; "));
        }
    }

    return autoName.join(" ");
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
    for (auto curveSet : m_crossPlotCurveSets())
    {
        curveSet->detachAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateAxisScaling()
{
    updateAxisDisplay();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateAxisDisplay()
{
    updateAxisInQwt(RiaDefines::PLOT_AXIS_BOTTOM);
    updateAxisInQwt(RiaDefines::PLOT_AXIS_LEFT);
    m_qwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateZoomWindowFromQwt()
{
    if (!m_qwtPlot) return;
    
    updateAxisFromQwt(RiaDefines::PLOT_AXIS_LEFT);
    updateAxisFromQwt(RiaDefines::PLOT_AXIS_BOTTOM);
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::selectAxisInPropertyEditor(int axis)
{
    RiuPlotMainWindowTools::showPlotMainWindow();
    if (axis == QwtPlot::yLeft)
    {
        RiuPlotMainWindowTools::selectAsCurrentItem(m_yAxisProperties);
    }
    else if (axis == QwtPlot::xBottom)
    {
        RiuPlotMainWindowTools::selectAsCurrentItem(m_xAxisProperties);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::setAutoZoomForAllAxes(bool enableAutoZoom)
{
    m_xAxisProperties->setAutoZoom(enableAutoZoom);
    m_yAxisProperties->setAutoZoom(enableAutoZoom);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimGridCrossPlot::findRimPlotObjectFromQwtCurve(const QwtPlotCurve* qwtCurve) const
{
    for (auto curveSet : m_crossPlotCurveSets)
    {
        for (auto curve : curveSet->curves())
        {
            if (curve->qwtPlotCurve() == qwtCurve)
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
QWidget* RimGridCrossPlot::createViewWidget(QWidget* mainWindowParent)
{
    if (!m_qwtPlot)
    {
        m_qwtPlot = new RiuQwtPlot(this, mainWindowParent);

        for (auto curveSet : m_crossPlotCurveSets)
        {
            curveSet->setParentQwtPlotNoReplot(m_qwtPlot);
        }
        m_qwtPlot->replot();
    }

    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::deleteViewWidget()
{
    detachAllCurves();
    if (m_qwtPlot)
    {
        m_qwtPlot->deleteLater();
        m_qwtPlot = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    for (auto curveSet : m_crossPlotCurveSets)
    {
        curveSet->loadDataAndUpdate(false);
    }

    updateCurveNamesAndPlotTitle();
    updateAllRequiredEditors();
    
    updatePlot();
    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_showLegend);

    if (m_showLegend())
    {
        uiOrdering.add(&m_legendFontSize);
    }

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Name Configuration");
    m_nameConfig->uiOrdering(uiConfigName, *nameGroup);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    caf::PdmUiTreeOrdering* axisFolder = uiTreeOrdering.add("Axes", ":/Axes16x16.png");

    axisFolder->add(&m_xAxisProperties);
    axisFolder->add(&m_yAxisProperties);

    uiTreeOrdering.add(&m_crossPlotCurveSets);

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue)
{
    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGridCrossPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_legendFontSize)
    {
        std::vector<int> fontSizes;
        fontSizes.push_back(8);
        fontSizes.push_back(10);
        fontSizes.push_back(12);
        fontSizes.push_back(14);
        fontSizes.push_back(16);
        

        for (int value : fontSizes)
        {
            QString text = QString("%1").arg(value);
            options.push_back(caf::PdmOptionItemInfo(text, value));
        }
    }
    return options;
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
void RimGridCrossPlot::updatePlot()
{
    if (m_qwtPlot)
    {
        RiuQwtPlotTools::setCommonPlotBehaviour(m_qwtPlot);
        RiuQwtPlotTools::setDefaultAxes(m_qwtPlot);

        updateAxisDisplay();

        for (auto curveSet : m_crossPlotCurveSets)
        {
            curveSet->setParentQwtPlotNoReplot(m_qwtPlot);
        }

        if (m_showLegend())
        {
            // Will be released in plot destructor or when a new legend is set
            QwtLegend* legend = new QwtLegend(m_qwtPlot);

            auto font = legend->font();
            font.setPixelSize(m_legendFontSize());
            legend->setFont(font);
            m_qwtPlot->insertLegend(legend, QwtPlot::BottomLegend);
        }
        else
        {
            m_qwtPlot->insertLegend(nullptr);
        }
        m_qwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateCurveNamesAndPlotTitle()
{
    updateCurveNames();

    if (m_qwtPlot)
    {
        m_qwtPlot->setTitle(this->createAutoName());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::xAxisParameterString() const
{
    QStringList xAxisParams;
    for (auto curveSet : m_crossPlotCurveSets)
    {
        xAxisParams.push_back(curveSet->xAxisName());
    }

    xAxisParams.removeDuplicates();

    if (xAxisParams.size() > 5)
    {
        return QString("%1 parameters").arg(xAxisParams.size());
    }

    return xAxisParams.join(", ");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::yAxisParameterString() const
{
    QStringList yAxisParams;
    for (auto curveSet : m_crossPlotCurveSets)
    {
        yAxisParams.push_back(curveSet->yAxisName());
    }

    yAxisParams.removeDuplicates();

    if (yAxisParams.size() > 5)
    {
        return QString("%1 parameters").arg(yAxisParams.size());
    }

    return yAxisParams.join(", ");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateAxisInQwt(RiaDefines::PlotAxis axisType)
{
    RimPlotAxisProperties* axisProperties      = m_xAxisProperties();
    QString                axisParameterString = xAxisParameterString();

    if (axisType == RiaDefines::PLOT_AXIS_LEFT)
    {
        axisProperties      = m_yAxisProperties();
        axisParameterString = yAxisParameterString();
    }

    QwtPlot::Axis qwtAxisId = axisProperties->qwtPlotAxisType();

    if (axisProperties->isActive())
    {
        m_qwtPlot->enableAxis(qwtAxisId, true);
        
        QwtText axisTitle(axisParameterString);
        QFont font = m_qwtPlot->axisFont(qwtAxisId);
        font.setBold(true);
        font.setPixelSize(axisProperties->titleFontSize);
        axisTitle.setFont(font);

        switch (axisProperties->titlePositionEnum())
        {
            case RimPlotAxisProperties::AXIS_TITLE_CENTER:
                axisTitle.setRenderFlags(Qt::AlignCenter);
                break;
            case RimPlotAxisProperties::AXIS_TITLE_END:
                axisTitle.setRenderFlags(Qt::AlignRight);
                break;
        }

        m_qwtPlot->setAxisTitle(qwtAxisId, axisTitle);

        if (axisProperties->isLogarithmicScaleEnabled)
        {
            QwtLogScaleEngine* currentScaleEngine =
                dynamic_cast<QwtLogScaleEngine*>(m_qwtPlot->axisScaleEngine(axisProperties->qwtPlotAxisType()));
            if (!currentScaleEngine)
            {
                m_qwtPlot->setAxisScaleEngine(axisProperties->qwtPlotAxisType(), new QwtLogScaleEngine);
                m_qwtPlot->setAxisMaxMinor(axisProperties->qwtPlotAxisType(), 5);
            }
        }
        else
        {
            QwtLinearScaleEngine* currentScaleEngine =
                dynamic_cast<QwtLinearScaleEngine*>(m_qwtPlot->axisScaleEngine(axisProperties->qwtPlotAxisType()));
            if (!currentScaleEngine)
            {
                m_qwtPlot->setAxisScaleEngine(axisProperties->qwtPlotAxisType(), new QwtLinearScaleEngine);
                m_qwtPlot->setAxisMaxMinor(axisProperties->qwtPlotAxisType(), 3);
            }
        }

        if (axisProperties->isAutoZoom())
        {
            m_qwtPlot->setAxisAutoScale(qwtAxisId);
        }
        else
        {
            m_qwtPlot->setAxisScale(qwtAxisId, axisProperties->visibleRangeMin, axisProperties->visibleRangeMax);
        }


    }
    else
    {
        m_qwtPlot->enableAxis(qwtAxisId, false);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateAxisFromQwt(RiaDefines::PlotAxis axisType)
{
    CVF_ASSERT(m_qwtPlot);
    
    QwtInterval xAxisRange = m_qwtPlot->currentAxisRange(QwtPlot::xBottom);
    QwtInterval yAxisRange = m_qwtPlot->currentAxisRange(QwtPlot::yLeft);

    RimPlotAxisProperties* axisProperties = m_xAxisProperties();
    QwtInterval            axisRange      = xAxisRange;

    if (axisType == RiaDefines::PLOT_AXIS_LEFT)
    {
        axisProperties = m_yAxisProperties();
        axisRange      = yAxisRange;
    }

    axisProperties->visibleRangeMin = axisRange.minValue();
    axisProperties->visibleRangeMax = axisRange.maxValue();

    axisProperties->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateCurveNames()
{
    m_curveNameHelper.reset();

    for (auto curveSet : m_crossPlotCurveSets())
    {
        m_curveNameHelper.addCurveSet(curveSet);
    }

    m_curveNameHelper.applyCurveNames();
}

//--------------------------------------------------------------------------------------------------
/// Name Configuration
/// 
//--------------------------------------------------------------------------------------------------

CAF_PDM_SOURCE_INIT(RimGridCrossPlotNameConfig, "RimGridCrossPlotNameConfig");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotNameConfig::RimGridCrossPlotNameConfig(RimNameConfigHolderInterface* holder /*= nullptr*/)
    : RimNameConfig(holder)
{
    CAF_PDM_InitObject("Cross Plot Name Generator", "", "", "");

    CAF_PDM_InitField(&addDataSetNames, "AddDataSetNames", true, "Add Data Set Names", "", "", "");

    setCustomName("Cross Plot");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotNameConfig::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimNameConfig::defineUiOrdering(uiConfigName, uiOrdering);
    uiOrdering.add(&addDataSetNames);
}
