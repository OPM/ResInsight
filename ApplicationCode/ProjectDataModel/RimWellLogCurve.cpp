/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogCurve.h"

#include "RimWellLogPlot.h"

#include "RimWellLogTrack.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuWellLogTrack.h"

#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

#include "qwt_symbol.h"

// NB! Special macro for pure virtual class
CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimWellLogCurve, "WellLogPlotCurve");

namespace caf
{
    template<>
    void caf::AppEnum< RimWellLogCurve::LineStyleEnum >::setUp()
    {
        addItem(RimWellLogCurve::STYLE_NONE,    "STYLE_NONE",       "None");
        addItem(RimWellLogCurve::STYLE_SOLID,   "STYLE_SOLID",      "Solid");
        addItem(RimWellLogCurve::STYLE_DASH,    "STYLE_DASH",       "Dashes");
        addItem(RimWellLogCurve::STYLE_DOT,     "STYLE_DOT",        "Dots");
        addItem(RimWellLogCurve::STYLE_DASH_DOT,"STYLE_DASH_DOT",   "Dashes and Dots");

        setDefault(RimWellLogCurve::STYLE_SOLID);
    }


    template<>
    void caf::AppEnum< RimWellLogCurve::PointSymbolEnum >::setUp()
    {
        addItem(RimWellLogCurve::SYMBOL_NONE,       "SYMBOL_NONE",      "None");
        addItem(RimWellLogCurve::SYMBOL_ELLIPSE,    "SYMBOL_ELLIPSE",   "Ellipse");
        addItem(RimWellLogCurve::SYMBOL_RECT,       "SYMBOL_RECT",      "Rect");
        addItem(RimWellLogCurve::SYMBOL_DIAMOND,    "SYMBOL_DIAMOND",   "Diamond");
        addItem(RimWellLogCurve::SYMBOL_TRIANGLE,   "SYMBOL_TRIANGLE",  "Triangle");
        addItem(RimWellLogCurve::SYMBOL_CROSS,      "SYMBOL_CROSS",     "Cross");
        addItem(RimWellLogCurve::SYMBOL_XCROSS,     "SYMBOL_XCROSS",    "X Cross");

        setDefault(RimWellLogCurve::SYMBOL_NONE);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogCurve::RimWellLogCurve()
{
    CAF_PDM_InitObject("Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitField(&m_showCurve, "Show", true, "Show curve", "", "", "");
    m_showCurve.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_curveName,        "CurveName",        "Curve Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_customCurveName,  "CurveDescription", "Custom Name", "", "", "");
    m_customCurveName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_autoName, "AutoName", true, "Auto Name", "", "", "");

    CAF_PDM_InitField(&m_curveColor, "Color", cvf::Color3f(cvf::Color3::BLACK), "Color", "", "", "");

    CAF_PDM_InitField(&m_curveThickness, "Thickness", 1.0f, "Thickness", "", "", "");
    m_curveThickness.uiCapability()->setUiEditorTypeName(caf::PdmUiComboBoxEditor::uiEditorTypeName());

    caf::AppEnum< RimWellLogCurve::LineStyleEnum > lineStyle = STYLE_SOLID;
    CAF_PDM_InitField(&m_lineStyle, "LineStyle", lineStyle, "Line style", "", "", "");

    caf::AppEnum< RimWellLogCurve::PointSymbolEnum > pointSymbol = SYMBOL_NONE;
    CAF_PDM_InitField(&m_pointSymbol, "PointSymbol", pointSymbol, "Point style", "", "", "");

    m_qwtPlotCurve = new RiuLineSegmentQwtPlotCurve;
    m_qwtPlotCurve->setXAxis(QwtPlot::xTop);
    m_qwtPlotCurve->setYAxis(QwtPlot::yLeft);

    m_ownerQwtTrack = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogCurve::~RimWellLogCurve()
{
    m_qwtPlotCurve->detach();    
    delete m_qwtPlotCurve;

    if (m_ownerQwtTrack)
    {
        m_ownerQwtTrack->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showCurve)
    {
       this->updateCurveVisibility();
    }
    else if (changedField == &m_curveName)
    {
        m_customCurveName = m_curveName;
        updatePlotTitle();
    }
    else if (&m_curveColor == changedField
            || &m_curveThickness == changedField
            || &m_pointSymbol == changedField
            || &m_lineStyle == changedField)
    {
        updateCurveAppearance();
    }
    else if (changedField == &m_autoName)
    {
        if (!m_autoName)
        {
            m_customCurveName = createCurveName();
        }

        updateOptionSensitivity();
        updateCurveName();
        updatePlotTitle();
    }

    if (m_ownerQwtTrack) m_ownerQwtTrack->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogCurve::objectToggleField()
{
    return &m_showCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updateCurveVisibility()
{
    if (m_showCurve() && m_ownerQwtTrack)
    {
        m_qwtPlotCurve->attach(m_ownerQwtTrack);
    }
    else
    {
        m_qwtPlotCurve->detach();
    }

    RimWellLogPlot* wellLogPlot;
    this->firstAnchestorOrThisOfType(wellLogPlot);
    if (wellLogPlot)
    {
        wellLogPlot->calculateAvailableDepthRange();
    }

    RimWellLogTrack* wellLogPlotTrack;
    this->firstAnchestorOrThisOfType(wellLogPlotTrack);
    if (wellLogPlotTrack)
    {
        wellLogPlotTrack->zoomAllXAndZoomAllDepthOnOwnerPlot();
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updatePlotConfiguration()
{
    this->updateCurveVisibility();
    this->updateCurveName();
    this->updatePlotTitle();

    updateCurveAppearance();
    // Todo: Rest of the curve setup controlled from this class
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setQwtTrack(RiuWellLogTrack* plot)
{
    m_ownerQwtTrack = plot;
    if (m_showCurve)
    {
        m_qwtPlotCurve->attach(m_ownerQwtTrack);
        m_ownerQwtTrack->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogCurve::userDescriptionField()
{
    return &m_curveName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurve::depthRange(double* minimumDepth, double* maximumDepth) const
{
    CVF_ASSERT(minimumDepth && maximumDepth);
    CVF_ASSERT(m_qwtPlotCurve);
    
    if (m_qwtPlotCurve->data()->size() < 1)
    {
        return false;
    }

    *minimumDepth = m_qwtPlotCurve->minYValue();
    *maximumDepth = m_qwtPlotCurve->maxYValue();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurve::valueRange(double* minimumValue, double* maximumValue) const
{
    CVF_ASSERT(minimumValue && maximumValue);
    CVF_ASSERT(m_qwtPlotCurve);

    if (m_qwtPlotCurve->data()->size() < 1)
    {
        return false;
    }

    *minimumValue = m_qwtPlotCurve->minXValue();
    *maximumValue = m_qwtPlotCurve->maxXValue();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setColor(const cvf::Color3f& color)
{
    m_curveColor = color;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::detachQwtCurve()
{
    m_qwtPlotCurve->detach();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QwtPlotCurve* RimWellLogCurve::plotCurve() const
{
    return m_qwtPlotCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updatePlotTitle()
{
    m_qwtPlotCurve->setTitle(m_curveName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurve::isCurveVisible() const
{
    return m_showCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::initAfterRead()
{
    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::zoomAllOwnerTrackAndPlot()
{
    RimWellLogPlot* wellLogPlot;
    firstAnchestorOrThisOfType(wellLogPlot);
    if (wellLogPlot)
    {
        wellLogPlot->calculateAvailableDepthRange();
        wellLogPlot->updateDepthZoom();
    }

    RimWellLogTrack* plotTrack;
    firstAnchestorOrThisOfType(plotTrack);
    if (plotTrack)
    {
        plotTrack->zoomAllXAndZoomAllDepthOnOwnerPlot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updateCurveName()
{
    if (m_autoName)
    {
        m_curveName = this->createCurveName();
    }
    else
    {
        m_curveName = m_customCurveName;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updateOptionSensitivity()
{
    m_curveName.uiCapability()->setUiReadOnly(m_autoName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigWellLogCurveData* RimWellLogCurve::curveData() const
{
    return m_curveData.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updateCurveAppearance()
{
    CVF_ASSERT(m_qwtPlotCurve);

    QColor curveColor(m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte());

    QwtSymbol* symbol = NULL;

    if (m_pointSymbol() != SYMBOL_NONE)
    {
        QwtSymbol::Style style = QwtSymbol::NoSymbol;
    
        switch (m_pointSymbol())
        {
            case SYMBOL_ELLIPSE :
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
QList<caf::PdmOptionItemInfo> RimWellLogCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
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

