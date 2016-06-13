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
#include "RiuLineSegmentQwtPlotCurve.h"

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

    CAF_PDM_InitField(&m_curveThickness, "Thickness", 1.0f, "Thickness", "", "", "");
    m_curveThickness.uiCapability()->setUiEditorTypeName(caf::PdmUiComboBoxEditor::uiEditorTypeName());

    caf::AppEnum< RimPlotCurve::LineStyleEnum > lineStyle = STYLE_SOLID;
    CAF_PDM_InitField(&m_lineStyle, "LineStyle", lineStyle, "Line style", "", "", "");

    caf::AppEnum< RimPlotCurve::PointSymbolEnum > pointSymbol = SYMBOL_NONE;
    CAF_PDM_InitField(&m_pointSymbol, "PointSymbol", pointSymbol, "Point style", "", "", "");

    m_qwtPlotCurve = new RiuLineSegmentQwtPlotCurve;

    m_parentQwtPlot = NULL;
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
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showCurve)
    {
        this->updateCurveVisibility();
    }
    else if (changedField == &m_curveName)
    {
        m_customCurveName = m_curveName;
        updateCurveName();
    }
    else if (&m_curveColor == changedField
             || &m_curveThickness == changedField
             || &m_pointSymbol == changedField
             || &m_lineStyle == changedField)
    {
        updateCurveAppearance();
    }
    else if (changedField == &m_isUsingAutoName)
    {
        if (!m_isUsingAutoName)
        {
            m_customCurveName = createCurveAutoName();
        }

        updateOptionSensitivity();
        updateCurveName();
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
void RimPlotCurve::updateCurveVisibility()
{
    if (m_showCurve() && m_parentQwtPlot)
    {
        m_qwtPlotCurve->attach(m_parentQwtPlot);
    }
    else
    {
        m_qwtPlotCurve->detach();
    }

    zoomAllParentPlot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurvePresentation()
{
    this->updateCurveVisibility();
    this->updateCurveName();

    updateCurveAppearance();
    // Todo: Rest of the curve setup controlled from this class
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setParentQwtPlot(QwtPlot* plot)
{
    m_parentQwtPlot = plot;
    if (m_showCurve)
    {
        m_qwtPlotCurve->attach(m_parentQwtPlot);
        m_parentQwtPlot->replot();
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
void RimPlotCurve::initAfterRead()
{
    updateOptionSensitivity();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveName()
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
void RimPlotCurve::updateCurveAppearance()
{
    CVF_ASSERT(m_qwtPlotCurve);

    QColor curveColor(m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte());

    QwtSymbol* symbol = NULL;

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
        curveStyle = QwtPlotCurve::Lines;

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

    m_qwtPlotCurve->setPen(curveColor, m_curveThickness, penStyle);
    m_qwtPlotCurve->setStyle(curveStyle);
    m_qwtPlotCurve->setSymbol(symbol);
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
void RimPlotCurve::loadDataAndUpdate()
{
    this->onLoadDataAndUpdate();
}



