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

#include "RifEclipseDataTableFormatter.h"
#include "RiuGridCrossQwtPlot.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotTools.h"

#include "RimGridCrossPlotCurve.h"
#include "RimGridCrossPlotDataSet.h"
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

CAF_PDM_SOURCE_INIT(RimGridCrossPlot, "RimGridCrossPlot");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlot::RimGridCrossPlot()
{
    CAF_PDM_InitObject("Grid Cross Plot", ":/SummaryXPlotLight16x16.png", "", "");

    CAF_PDM_InitField(&m_showInfoBox, "ShowInfoBox", true, "Show Info Box", "", "", "");
    CAF_PDM_InitField(&m_showLegend, "ShowLegend", true, "Show Legend", "", "", "");
    CAF_PDM_InitField(&m_legendFontSize, "LegendFontSize", 10, "Legend and Info Font Size", "", "", "");
    m_legendFontSize = RiaFontCache::pointSizeFromFontSizeEnum(RiaApplication::instance()->preferences()->defaultPlotFontSize());

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

    CAF_PDM_InitFieldNoDefault(&m_crossPlotDataSets, "CrossPlotCurve", "Cross Plot Data Set", "", "", "");
    m_crossPlotDataSets.uiCapability()->setUiHidden(true);

    m_nameConfig = new RimGridCrossPlotNameConfig(this);
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
RimGridCrossPlotDataSet* RimGridCrossPlot::createDataSet()
{
    RimGridCrossPlotDataSet* dataSet = new RimGridCrossPlotDataSet();
    m_crossPlotDataSets.push_back(dataSet);
    return dataSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCrossPlot::indexOfDataSet(const RimGridCrossPlotDataSet* dataSetToCheck) const
{
    int index = 0;
    for (auto dataSet : m_crossPlotDataSets())
    {
        if (dataSet == dataSetToCheck)
        {
            return index;
        }
        if (dataSet->isChecked() && dataSet->visibleCurveCount() > 0u)
        {
            index++;
        }
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::addDataSet(RimGridCrossPlotDataSet* dataSet)
{
    m_crossPlotDataSets.push_back(dataSet);
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
    if (!m_qwtPlot) return;

    setAutoZoomForAllAxes(true);
    updateAxisInQwt(RiaDefines::PLOT_AXIS_LEFT);
    updateAxisInQwt(RiaDefines::PLOT_AXIS_BOTTOM);

    m_qwtPlot->replot();

    updateAxisFromQwt(RiaDefines::PLOT_AXIS_LEFT);
    updateAxisFromQwt(RiaDefines::PLOT_AXIS_BOTTOM);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::calculateZoomRangeAndUpdateQwt()
{
    if (m_qwtPlot)
    {
        m_qwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::reattachCurvesToQwtAndReplot()
{
    if (m_qwtPlot)
    {
        for (auto dataSet : m_crossPlotDataSets)
        {
            dataSet->detachAllCurves();
            if (dataSet->isChecked())
            {
                dataSet->setParentQwtPlotNoReplot(m_qwtPlot);
            }
        }
        updateAxisDisplay();
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
        QStringList                                                           dataSetStrings;
        std::map<RimGridCrossPlotDataSet::NameComponents, std::set<QString>> allNameComponents;
        for (auto dataSet : m_crossPlotDataSets)
        {
            if (dataSet->isChecked())
            {
                QStringList componentList;
                auto dataSetNameComponents = dataSet->nameComponents();

                for (auto dataSetNameComponent : dataSetNameComponents)
                {
                    if (!dataSetNameComponent.second.isEmpty())
                    {
                        if (allNameComponents[dataSetNameComponent.first].count(dataSetNameComponent.second) == 0u)
                        {
                            componentList += dataSetNameComponent.second;
                            allNameComponents[dataSetNameComponent.first].insert(dataSetNameComponent.second);
                        }
                    }
                }
                if (!componentList.isEmpty())
                {
                    dataSetStrings += componentList.join(", ");
                }
            }
        }

        dataSetStrings.removeDuplicates();
        if (dataSetStrings.size() > 2)
        {
            autoName += QString("(%1 Data Sets)").arg(dataSetStrings.size());
        }
        if (!dataSetStrings.isEmpty())
        {
            autoName += QString("(%1)").arg(dataSetStrings.join("; "));
        }
    }

    return autoName.join(" ");
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
caf::PdmFieldHandle* RimGridCrossPlot::userDescriptionField()
{
    return m_nameConfig->nameField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::detachAllCurves()
{
    for (auto dataSet : m_crossPlotDataSets())
    {
        dataSet->detachAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateAxisScaling()
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateAxisDisplay()
{
    if (!m_qwtPlot) return;

    updateAxisInQwt(RiaDefines::PLOT_AXIS_BOTTOM);
    updateAxisInQwt(RiaDefines::PLOT_AXIS_LEFT);

    m_qwtPlot->updateAnnotationObjects(m_xAxisProperties);
    m_qwtPlot->updateAnnotationObjects(m_yAxisProperties);

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
    setAutoZoomForAllAxes(false);
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
    for (auto dataSet : m_crossPlotDataSets)
    {
        for (auto curve : dataSet->curves())
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
        m_qwtPlot = new RiuGridCrossQwtPlot(this, mainWindowParent);

        for (auto dataSet : m_crossPlotDataSets)
        {
            dataSet->setParentQwtPlotNoReplot(m_qwtPlot);
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

    for (auto dataSet : m_crossPlotDataSets)
    {
        dataSet->loadDataAndUpdate(false);
        dataSet->updateConnectedEditors();
    }

    updateCurveNamesAndPlotTitle();
    updatePlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_showInfoBox);
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

    uiTreeOrdering.add(&m_crossPlotDataSets);

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue)
{
    if (changedField == &m_legendFontSize)
    {
        for (auto dataSet : m_crossPlotDataSets)
        {
            dataSet->updateLegendIcons();
        }
    }
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

        for (auto dataSet : m_crossPlotDataSets)
        {
            dataSet->setParentQwtPlotNoReplot(m_qwtPlot);
        }

        if (m_showLegend())
        {
            // Will be released in plot destructor or when a new legend is set
            QwtLegend* legend = new QwtLegend(m_qwtPlot);

            auto font = legend->font();
            font.setPointSize(m_legendFontSize());
            legend->setFont(font);
            m_qwtPlot->insertLegend(legend, QwtPlot::BottomLegend);
        }
        else
        {
            m_qwtPlot->insertLegend(nullptr);
        }
        m_qwtPlot->updateLegendSizesToMatchPlot();
        m_qwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::updateCurveNamesAndPlotTitle()
{
    for (size_t i = 0; i < m_crossPlotDataSets.size(); ++i)
    {
        m_crossPlotDataSets[i]->updateCurveNames(i, m_crossPlotDataSets.size());
    }

    if (m_qwtPlot)
    {
        m_qwtPlot->setTitle(this->createAutoName());
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
    xAxisProperties->setNameAndAxis(yAxisProperties->name(), yAxisProperties->qwtPlotAxisType());
    yAxisProperties->setNameAndAxis(tmpName, tmpAxis);

    m_xAxisProperties.removeChildObject(xAxisProperties);
    m_yAxisProperties.removeChildObject(yAxisProperties);
    m_yAxisProperties = xAxisProperties;
    m_xAxisProperties = yAxisProperties;

    for (auto dataSet : m_crossPlotDataSets)
    {
        dataSet->swapAxisProperties(false);
    }

    loadDataAndUpdate();

    updateAxisDisplay();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::asciiTitleForPlotExport(int dataSetIndex) const
{
    if ((size_t)dataSetIndex < m_crossPlotDataSets.size())
    {
        return m_crossPlotDataSets[dataSetIndex]->createAutoName();
    }
    return "Data invalid";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::asciiDataForPlotExport(int dataSetIndex) const
{
    if ((size_t)dataSetIndex < m_crossPlotDataSets.size())
    {
        QString     asciiData;
        QTextStream stringStream(&asciiData);

        RifEclipseDataTableFormatter formatter(stringStream);
        formatter.setCommentPrefix("");
        formatter.setTableRowPrependText("");
        formatter.setTableRowLineAppendText("");
        formatter.setColumnSpacing(3);

        m_crossPlotDataSets[dataSetIndex]->exportFormattedData(formatter);
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
RiuGridCrossQwtPlot* RimGridCrossPlot::qwtPlot() const
{
    return m_qwtPlot;
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
void RimGridCrossPlot::setYAxisInverted(bool inverted)
{
    m_yAxisProperties->setAxisInverted(inverted);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCrossPlot::legendFontSize() const
{
    return m_legendFontSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlot::hasCustomFontSizes(RiaDefines::FontSettingType fontSettingType, int defaultFontSize) const
{
    if (fontSettingType != RiaDefines::PLOT_FONT) return false;

    for (auto plotAxis : allPlotAxes())
    {
        if (plotAxis->titleFontSize() != defaultFontSize || plotAxis->valuesFontSize() != defaultFontSize)
        {
            return true;
        }
    }

    if (legendFontSize() != defaultFontSize)
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlot::applyFontSize(RiaDefines::FontSettingType fontSettingType, int oldFontSize, int fontSize, bool forceChange /*= false*/)
{
    if (fontSettingType != RiaDefines::PLOT_FONT) return false;

    bool anyChange = false;
    for (auto plotAxis : allPlotAxes())
    {
        if (forceChange || plotAxis->titleFontSize() == oldFontSize)
        {
            plotAxis->setTitleFontSize(fontSize);
            anyChange = true;
        }
        if (forceChange || plotAxis->valuesFontSize() == oldFontSize)
        {
            plotAxis->setValuesFontSize(fontSize);
            anyChange = true;
        }
    }

    if (forceChange || legendFontSize() == oldFontSize)
    {
        m_legendFontSize = fontSize;
        anyChange = true;
    }

    if (anyChange) loadDataAndUpdate();

    return anyChange;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlot::xAxisParameterString() const
{
    QStringList xAxisParams;
    for (auto dataSet : m_crossPlotDataSets)
    {
        if (dataSet->isChecked() && dataSet->sampleCount() > 0u)
        {
            xAxisParams.push_back(dataSet->xAxisName());
        }
    }

    xAxisParams.removeDuplicates();

    if (xAxisParams.size() > 4)
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
    for (auto dataSet : m_crossPlotDataSets)
    {
        if (dataSet->isChecked() && dataSet->sampleCount() > 0u)
        {
            yAxisParams.push_back(dataSet->yAxisName());
        }
    }

    yAxisParams.removeDuplicates();

    if (yAxisParams.size() > 4)
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
    if (!m_qwtPlot) return;

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
        QFont   titleFont = m_qwtPlot->axisTitle(qwtAxisId).font();
        titleFont.setBold(true);
        titleFont.setPointSize(axisProperties->titleFontSize());
        axisTitle.setFont(titleFont);

        QFont   valuesFont = m_qwtPlot->axisFont(qwtAxisId);
        valuesFont.setPointSize(axisProperties->valuesFontSize());
        m_qwtPlot->setAxisFont(qwtAxisId, valuesFont);

        switch (axisProperties->titlePosition())
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

            if (axisProperties->isAutoZoom())
            {
                std::vector<const QwtPlotCurve*> plotCurves = visibleQwtCurves();

                double                        min, max;
                RimPlotAxisLogRangeCalculator logRangeCalculator(qwtAxisId, plotCurves);
                logRangeCalculator.computeAxisRange(&min, &max);
                if (axisProperties->isAxisInverted())
                {
                    std::swap(min, max);
                }

                m_qwtPlot->setAxisScale(qwtAxisId, min, max);
            }
            else
            {
                m_qwtPlot->setAxisScale(qwtAxisId, axisProperties->visibleRangeMin, axisProperties->visibleRangeMax);
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

            if (axisProperties->isAutoZoom())
            {
                m_qwtPlot->setAxisAutoScale(qwtAxisId);
            }
            else
            {
                m_qwtPlot->setAxisScale(qwtAxisId, axisProperties->visibleRangeMin, axisProperties->visibleRangeMax);
            }
        }
        m_qwtPlot->axisScaleEngine(axisProperties->qwtPlotAxisType())
            ->setAttribute(QwtScaleEngine::Inverted, axisProperties->isAxisInverted());
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

    axisProperties->visibleRangeMin = std::min(axisRange.minValue(), axisRange.maxValue());
    axisProperties->visibleRangeMax = std::max(axisRange.minValue(), axisRange.maxValue());

    axisProperties->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const QwtPlotCurve*> RimGridCrossPlot::visibleQwtCurves() const
{
    std::vector<const QwtPlotCurve*> plotCurves;
    for (auto dataSet : m_crossPlotDataSets)
    {
        if (dataSet->isChecked())
        {
            for (auto curve : dataSet->curves())
            {
                if (curve->isCurveVisible())
                {
                    plotCurves.push_back(curve->qwtPlotCurve());
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
void RimGridCrossPlot::setShowInfoBox(bool enable)
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
