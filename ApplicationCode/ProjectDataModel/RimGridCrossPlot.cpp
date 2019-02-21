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

#include "RiuQwtPlotTools.h"

#include "RimGridCrossPlotCurveSet.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cvfAssert.h"

#include "qwt_legend.h"
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

    CAF_PDM_InitFieldNoDefault(&m_crossPlotCurveSets, "CrossPlotCurve", "Cross Plot Data Set", "", "", "");
    m_crossPlotCurveSets.uiCapability()->setUiHidden(true);

    m_nameConfig        = new RimGridCrossPlotNameConfig(this);
    
    createCurveSet();
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
    for (auto curveSet : m_crossPlotCurveSets)
    {
        curveSet->setParentQwtPlotNoReplot(m_qwtPlot);
    }
    m_qwtPlot->replot();
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
            autoName += QString("(%1)").arg(dataSets.join(", "));
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
QWidget* RimGridCrossPlot::createViewWidget(QWidget* mainWindowParent)
{
    if (!m_qwtPlot)
    {
        m_qwtPlot = new QwtPlot(QString("Grid Cross Plot"), mainWindowParent);
    }

    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::deleteViewWidget()
{
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
    CVF_ASSERT(m_qwtPlot);

    for (auto curveSet : m_crossPlotCurveSets)
    {
        curveSet->loadDataAndUpdate(false);
        curveSet->setParentQwtPlotNoReplot(m_qwtPlot);
    }

    performAutoNameUpdate();

    m_qwtPlot->setAxisAutoScale(QwtPlot::xBottom);
    m_qwtPlot->setAxisAutoScale(QwtPlot::yLeft);
    m_qwtPlot->setAxisTitle(QwtPlot::xBottom, QwtText(xAxisParameterString()));
    m_qwtPlot->setAxisTitle(QwtPlot::yLeft, QwtText(yAxisParameterString()));

    RiuQwtPlotTools::setCommonPlotBehaviour(m_qwtPlot);
    RiuQwtPlotTools::setDefaultAxes(m_qwtPlot);
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
    m_qwtPlot->show();
    this->updateAllRequiredEditors();
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
