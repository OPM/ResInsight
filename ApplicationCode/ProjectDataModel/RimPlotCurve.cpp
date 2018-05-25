/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimPlotCurve.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"

#include "RiuRimQwtPlotCurve.h"

#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

#include "qwt_symbol.h"
#include "qwt_plot.h"

// NB! Special macro for pure virtual class
CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimPlotCurve, "PlotCurve");

namespace caf
{
template<>
void caf::AppEnum< RimPlotCurve::LineStyleEnum >::setUp()
{
    addItem(RimPlotCurve::STYLE_NONE, "STYLE_NONE", "None");
    addItem(RimPlotCurve::STYLE_SOLID, "STYLE_SOLID", "Solid");
    addItem(RimPlotCurve::STYLE_DASH, "STYLE_DASH", "Dashes");
    addItem(RimPlotCurve::STYLE_DOT, "STYLE_DOT", "Dots");
    addItem(RimPlotCurve::STYLE_DASH_DOT, "STYLE_DASH_DOT", "Dashes and Dots");

    setDefault(RimPlotCurve::STYLE_SOLID);
}


template<>
void caf::AppEnum< RimPlotCurve::PointSymbolEnum >::setUp()
{
    addItem(RimPlotCurve::SYMBOL_NONE, "SYMBOL_NONE", "None");
    addItem(RimPlotCurve::SYMBOL_ELLIPSE, "SYMBOL_ELLIPSE", "Ellipse");
    addItem(RimPlotCurve::SYMBOL_RECT, "SYMBOL_RECT", "Rect");
    addItem(RimPlotCurve::SYMBOL_DIAMOND, "SYMBOL_DIAMOND", "Diamond");
    addItem(RimPlotCurve::SYMBOL_TRIANGLE, "SYMBOL_TRIANGLE", "Triangle");
    addItem(RimPlotCurve::SYMBOL_CROSS, "SYMBOL_CROSS", "Cross");
    addItem(RimPlotCurve::SYMBOL_XCROSS, "SYMBOL_XCROSS", "X Cross");

    setDefault(RimPlotCurve::SYMBOL_NONE);
}

template<>
void RimPlotCurve::CurveInterpolation::setUp()
{
    addItem(RimPlotCurve::INTERPOLATION_POINT_TO_POINT, "INTERPOLATION_POINT_TO_POINT", "Point to Point");
    addItem(RimPlotCurve::INTERPOLATION_STEP_LEFT,      "INTERPOLATION_STEP_LEFT",      "Step Left");

    setDefault(RimPlotCurve::INTERPOLATION_POINT_TO_POINT);
}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPlotCurve::RimPlotCurve()
{
    CAF_PDM_InitObject("Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitField(&m_showCurve, "Show", true, "Show curve", "", "", "");
    m_showCurve.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_curveName, "CurveName", "Curve Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_customCurveName, "CurveDescription", "Custom Name", "", "", "");
    m_customCurveName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_isUsingAutoName, "AutoName", true, "Auto Name", "", "", "");

    CAF_PDM_InitField(&m_curveColor, "Color", cvf::Color3f(cvf::Color3::BLACK), "Color", "", "", "");

    CAF_PDM_InitField(&m_curveThickness, "Thickness", 1, "Line Thickness", "", "", "");
    m_curveThickness.uiCapability()->setUiEditorTypeName(caf::PdmUiComboBoxEditor::uiEditorTypeName());

    caf::AppEnum< RimPlotCurve::LineStyleEnum > lineStyle = STYLE_SOLID;
    CAF_PDM_InitField(&m_lineStyle, "LineStyle", lineStyle, "Line Style", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_curveInterpolation, "CurveInterpolation", "Interpolation", "", "", "");

    caf::AppEnum< RimPlotCurve::PointSymbolEnum > pointSymbol = SYMBOL_NONE;
    CAF_PDM_InitField(&m_pointSymbol, "PointSymbol", pointSymbol, "Symbol", "", "", "");

    CAF_PDM_InitField(&m_symbolSkipPixelDistance, "SymbolSkipPxDist", 0.0f, "Symbol Skip Distance", "", "Minimum pixel distance between symbols", "");

    CAF_PDM_InitField(&m_showLegend, "ShowLegend", true, "Contribute To Legend", "", "", "");

    CAF_PDM_InitField(&m_showErrorBars, "ShowErrorBars", true, "Show Error Bars", "", "", "");

    m_qwtPlotCurve = new RiuRimQwtPlotCurve(this);

    m_parentQwtPlot = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPlotCurve::~RimPlotCurve()
{
    if (m_qwtPlotCurve)
    {
        m_qwtPlotCurve->detach();
        delete m_qwtPlotCurve;
        m_qwtPlotCurve = nullptr;
    }

    if (m_parentQwtPlot)
    {
        m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showCurve)
    {
        this->updateCurveVisibility(true);
        if (m_showCurve()) loadDataAndUpdate(true);
    }
    else if (changedField == &m_curveName)
    {
        m_customCurveName = m_curveName;
        updateCurveNameAndUpdatePlotLegend();
    }
    else if (&m_curveColor == changedField
             || &m_curveThickness == changedField
             || &m_pointSymbol == changedField
             || &m_lineStyle == changedField
             || &m_symbolSkipPixelDistance == changedField
             || &m_curveInterpolation == changedField)
    {
        updateCurveAppearance();
    }
    else if (changedField == &m_isUsingAutoName)
    {
        if (!m_isUsingAutoName)
        {
            m_customCurveName = createCurveAutoName();
        }

        updateCurveNameAndUpdatePlotLegend();
    }
    else if (changedField == &m_showLegend)
    {
        updateLegendEntryVisibilityAndPlotLegend();
    }
    else if (changedField == &m_showErrorBars)
    {
        m_qwtPlotCurve->showErrorBars(m_showErrorBars);
        updateCurveAppearance();
    }
    if (m_parentQwtPlot) m_parentQwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPlotCurve::objectToggleField()
{
    return &m_showCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveVisibility(bool updateParentPlot)
{
    bool isVisibleInPossibleParent = true;
    
    {
        RimSummaryCurveCollection* summaryCurveCollection = nullptr;
        this->firstAncestorOrThisOfType(summaryCurveCollection);
        if (summaryCurveCollection) isVisibleInPossibleParent = summaryCurveCollection->isCurvesVisible();

        RimEnsembleCurveSet* ensembleCurveSet = nullptr;
        firstAncestorOrThisOfType(ensembleCurveSet);
        if (ensembleCurveSet) isVisibleInPossibleParent = ensembleCurveSet->isCurvesVisible();
    }

    if (m_showCurve() && m_parentQwtPlot && isVisibleInPossibleParent)
    {
        m_qwtPlotCurve->attach(m_parentQwtPlot);
    }
    else
    {
        m_qwtPlotCurve->detach();
    }

    if (updateParentPlot)
    {
        updateZoomInParentPlot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurvePresentation(bool updatePlotLegend)
{
    this->updateCurveVisibility(updatePlotLegend);

    if (updatePlotLegend)
    {
        this->updateCurveNameAndUpdatePlotLegend();
    }
    else
    {
        this->updateCurveNameNoLegendUpdate();
    }

    updateCurveAppearance();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setParentQwtPlotAndReplot(QwtPlot* plot)
{
    m_parentQwtPlot = plot;
    if (m_showCurve && m_parentQwtPlot)
    {
        m_qwtPlotCurve->attach(m_parentQwtPlot);
        m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setParentQwtPlotNoReplot(QwtPlot* plot)
{
    m_parentQwtPlot = plot;
    if (m_showCurve && m_parentQwtPlot)
    {
        m_qwtPlotCurve->attach(m_parentQwtPlot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPlotCurve::userDescriptionField()
{
    return &m_curveName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setColor(const cvf::Color3f& color)
{
    m_curveColor = color;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::detachQwtCurve()
{
    m_qwtPlotCurve->detach();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QwtPlotCurve* RimPlotCurve::qwtPlotCurve() const
{
    return m_qwtPlotCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::isCurveVisible() const
{
    return m_showCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setCurveVisiblity(bool visible)
{
    m_showCurve = visible;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveNameAndUpdatePlotLegend()
{
    if (m_isUsingAutoName)
    {
        m_curveName = this->createCurveAutoName();
    }
    else
    {
        m_curveName = m_customCurveName;
    }

    m_qwtPlotCurve->setTitle(m_curveName);
    updateLegendEntryVisibilityAndPlotLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveNameNoLegendUpdate()
{
    if (m_isUsingAutoName)
    {
        m_curveName = this->createCurveAutoName();
    }
    else
    {
        m_curveName = m_customCurveName;
    }

    m_qwtPlotCurve->setTitle(m_curveName);
    updateLegendEntryVisibilityNoPlotUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateOptionSensitivity()
{
    m_curveName.uiCapability()->setUiReadOnly(m_isUsingAutoName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::appearanceUiOrdering(caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_curveColor);
    uiOrdering.add(&m_pointSymbol);
    uiOrdering.add(&m_symbolSkipPixelDistance);
    uiOrdering.add(&m_curveThickness);
    uiOrdering.add(&m_lineStyle);
    uiOrdering.add(&m_curveInterpolation);

    if(isCrossPlotCurve()) m_showErrorBars = false; 
    else                   uiOrdering.add(&m_showErrorBars);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::curveNameUiOrdering(caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_isUsingAutoName);
    uiOrdering.add(&m_curveName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveAppearance()
{
    CVF_ASSERT(m_qwtPlotCurve);

    QColor curveColor(m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte());

    QwtSymbol* symbol = nullptr;

    if (m_pointSymbol() != SYMBOL_NONE)
    {
        QwtSymbol::Style style = QwtSymbol::NoSymbol;

        switch (m_pointSymbol())
        {
            case SYMBOL_ELLIPSE:
            style = QwtSymbol::Ellipse;
            break;
            case SYMBOL_RECT:
            style = QwtSymbol::Rect;
            break;
            case SYMBOL_DIAMOND:
            style = QwtSymbol::Diamond;
            break;
            case SYMBOL_TRIANGLE:
            style = QwtSymbol::Triangle;
            break;
            case SYMBOL_CROSS:
            style = QwtSymbol::Cross;
            break;
            case SYMBOL_XCROSS:
            style = QwtSymbol::XCross;
            break;

            default:
            break;
        }

        // QwtPlotCurve will take ownership of the symbol
        symbol = new QwtSymbol(style);

        symbol->setSize(6, 6);
        symbol->setColor(curveColor);
    }

    QwtPlotCurve::CurveStyle curveStyle = QwtPlotCurve::NoCurve;
    Qt::PenStyle penStyle = Qt::SolidLine;

    if (m_lineStyle() != STYLE_NONE)
    {
        switch (m_curveInterpolation())
        {
        case INTERPOLATION_STEP_LEFT:
            curveStyle = QwtPlotCurve::Steps;
            m_qwtPlotCurve->setCurveAttribute(QwtPlotCurve::Inverted, false);
            break;
        case INTERPOLATION_POINT_TO_POINT: // Fall through
        default:
            curveStyle = QwtPlotCurve::Lines;
            break;
        }

        switch (m_lineStyle())
        {
            case STYLE_SOLID:
            penStyle = Qt::SolidLine;
            break;
            case STYLE_DASH:
            penStyle = Qt::DashLine;
            break;
            case STYLE_DOT:
            penStyle = Qt::DotLine;
            break;
            case STYLE_DASH_DOT:
            penStyle = Qt::DashDotLine;
            break;

            default:
            break;
        }
    }
    QPen curvePen(curveColor);
    curvePen.setWidth(m_curveThickness);
    curvePen.setStyle(penStyle);

    m_qwtPlotCurve->setPen(curvePen);
    m_qwtPlotCurve->setStyle(curveStyle);
    m_qwtPlotCurve->setSymbol(symbol);
    m_qwtPlotCurve->setSymbolSkipPixelDistance(m_symbolSkipPixelDistance());

    m_qwtPlotCurve->setErrorBarsColor(curveColor);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::isCrossPlotCurve() const
{
    RimSummaryCrossPlot* crossPlot = nullptr;
    this->firstAncestorOrThisOfType(crossPlot);
    if (crossPlot) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPlotCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_curveThickness)
    {
        for (size_t i = 0; i < 10; i++)
        {
            options.push_back(caf::PdmOptionItemInfo(QString::number(i + 1), QVariant::fromValue(i + 1)));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::loadDataAndUpdate(bool updateParentPlot)
{
    this->onLoadDataAndUpdate(updateParentPlot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setLineStyle(LineStyleEnum lineStyle)
{
    m_lineStyle = lineStyle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbol(PointSymbolEnum symbolStyle)
{
    m_pointSymbol = symbolStyle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPlotCurve::PointSymbolEnum RimPlotCurve::symbol()
{
    return m_pointSymbol();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbolSkipDinstance(float distance)
{
    m_symbolSkipPixelDistance = distance;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setLineThickness(int thickness)
{
    m_curveThickness = thickness;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::resetAppearance()
{
    setColor(cvf::Color3f(cvf::Color3::BLACK));
    setLineThickness(2);
    setLineStyle(STYLE_SOLID);
    setSymbol(SYMBOL_NONE);
    setSymbolSkipDinstance(10);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::showLegend(bool show)
{
    m_showLegend = show;
    updateLegendEntryVisibilityNoPlotUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setZOrder(double z)
{
    if (m_qwtPlotCurve != nullptr)
    {
        m_qwtPlotCurve->setZ(z);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateLegendEntryVisibilityAndPlotLegend()
{
    updateLegendEntryVisibilityNoPlotUpdate();

    if (m_parentQwtPlot != nullptr)
    {
        m_parentQwtPlot->updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateLegendEntryVisibilityNoPlotUpdate()
{
    RimEnsembleCurveSet* ensembleCurveSet = nullptr;
    this->firstAncestorOrThisOfType(ensembleCurveSet);
    if (ensembleCurveSet)
    {
        return;
    }

    RimSummaryPlot* summaryPlot = nullptr;
    this->firstAncestorOrThisOfType(summaryPlot);

    if (summaryPlot)
    {
        bool showLegendInQwt = m_showLegend();

        if (summaryPlot->ensembleCurveSetCollection()->curveSets().empty() && summaryPlot->curveCount() == 1)
        {
            // Disable display of legend if the summary plot has only one single curve
            showLegendInQwt = false;
        }

        m_qwtPlotCurve->setItemAttribute(QwtPlotItem::Legend, showLegendInQwt);
    }
}
