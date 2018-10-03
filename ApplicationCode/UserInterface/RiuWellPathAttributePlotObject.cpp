/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiuWellPathAttributePlotObject.h"

#include "RiaColorTools.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimPerforationInterval.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPathAttribute.h"
#include "RimWellPath.h"

#include "RigWellPath.h"
#include "RiuQwtPlotCurve.h"

#include "qwt_plot.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_shapeitem.h"

#include <QBrush>
#include <Qt>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathAttributePlotObject::RiuWellPathAttributePlotObject(const RimWellPath* wellPath)
    : m_wellPath(wellPath)
    , m_attributeType(RimWellPathAttribute::AttributeWellTube)
    , m_depthType(RimWellLogPlot::MEASURED_DEPTH)
    , m_baseColor(cvf::Color4f(cvf::Color3::BLACK))
    , m_showLabel(false)
{
    CVF_ASSERT(wellPath);
    double wellStart = wellPath->wellPathGeometry()->measureDepths().front();
    double wellEnd = wellPath->wellPathGeometry()->measureDepths().back();
    m_startMD = wellStart;
    m_endMD = wellEnd;
    m_label = wellPath->name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathAttributePlotObject::RiuWellPathAttributePlotObject(const RimWellPath* wellPath,
                                                               const RimWellPathAttribute* wellPathAttribute)
    : m_wellPath(wellPath) 
    , m_attributeType(RimWellPathAttribute::AttributeCasing)
    , m_startMD(0.0)
    , m_endMD(0.0)
    , m_depthType(RimWellLogPlot::MEASURED_DEPTH)
    , m_baseColor(cvf::Color4f(cvf::Color3::BLACK))
    , m_showLabel(false)
{
    CVF_ASSERT(wellPathAttribute);
    if (wellPathAttribute)
    {
        m_attributeType  = wellPathAttribute->type();
        m_startMD        = wellPathAttribute->depthStart();
        m_endMD          = wellPathAttribute->depthEnd();
        m_label          = wellPathAttribute->label();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathAttributePlotObject::RiuWellPathAttributePlotObject(
    const RimWellPath*            wellPath,
    const RimPerforationInterval* perforationInterval)
    : m_wellPath(wellPath)
    , m_attributeType(RimWellPathAttribute::AttributePerforationInterval)
    , m_depthType(RimWellLogPlot::MEASURED_DEPTH)
    , m_baseColor(cvf::Color4f(cvf::Color3::BLACK))
    , m_showLabel(false)
{
    CVF_ASSERT(wellPath && perforationInterval);

    m_startMD = perforationInterval->startMD();
    m_endMD   = perforationInterval->endMD();
    m_label   = QString("Perforations: %1").arg(perforationInterval->name());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathAttributePlotObject::RiuWellPathAttributePlotObject(
    const RimWellPath*              wellPath,
    const RimFishbonesMultipleSubs* fishbones)
    : m_wellPath(wellPath)
    , m_attributeType(RimWellPathAttribute::AttributeFishbonesInterval)
    , m_depthType(RimWellLogPlot::MEASURED_DEPTH)
    , m_baseColor(cvf::Color4f(cvf::Color3::BLACK))
    , m_showLabel(false)
{
    CVF_ASSERT(wellPath && fishbones);

    m_startMD = fishbones->startOfSubMD();
    m_endMD   = fishbones->endOfSubMD();
    m_label   = fishbones->generatedName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathAttributePlotObject::RiuWellPathAttributePlotObject(
    const RimWellPath*            wellPath,
    const RimFracture*            fracture)
    : m_wellPath(wellPath)
    , m_attributeType(RimWellPathAttribute::AttributeFracture)
    , m_depthType(RimWellLogPlot::MEASURED_DEPTH)
    , m_baseColor(cvf::Color4f(cvf::Color3::BLACK))
    , m_showLabel(false)
{
    CVF_ASSERT(wellPath && fracture);

    if (fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH)
    {
        m_startMD = fracture->fractureMD() + 0.5*fracture->perforationLength();
        m_endMD = m_startMD + fracture->perforationLength();
    }
    else
    {
        m_startMD = fracture->fractureMD();
        m_endMD   = m_startMD + fracture->fractureTemplate()->computeFractureWidth(fracture);
    }
    m_label = fracture->name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathAttributePlotObject::~RiuWellPathAttributePlotObject()
{
    detachFromQwt();
    for (QwtPlotItem* plotFeature : m_plotFeatures)
    {
        delete plotFeature;
    }

    if (m_parentQwtPlot)
    {
        m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWellPathAttributePlotObject::label()
{
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::loadDataAndUpdate(bool updateParentPlot)
{
    onLoadDataAndUpdate(updateParentPlot);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttribute::AttributeType RiuWellPathAttributePlotObject::attributeType() const
{
    return m_attributeType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::onLoadDataAndUpdate(bool updateParentPlot)
{   
    double startDepth, endDepth;
    std::tie(startDepth, endDepth) = depthsOfDepthType();

    float columnAlpha = 0.9f;

    cvf::Color4f transparentBaseColor = m_baseColor;
    transparentBaseColor.a() = 0.0;
    if (m_attributeType == RimWellPathAttribute::AttributeWellTube)
    {
        addColumnFeature(-0.25, 0.25, startDepth, endDepth, m_baseColor);
    }
    else if (m_attributeType == RimWellPathAttribute::AttributeCasing)
    {
        addColumnFeature(-0.75, -0.5, startDepth, endDepth, m_baseColor);
        addColumnFeature(0.5, 0.75, startDepth, endDepth, m_baseColor);
        addMarker(-0.75, endDepth,10, RiuQwtSymbol::SYMBOL_LEFT_ANGLED_TRIANGLE,  m_baseColor);
        addMarker(0.75, endDepth, 10, RiuQwtSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE, m_baseColor);
        addMarker(0.625, endDepth, 10, RiuQwtSymbol::SYMBOL_NONE, m_baseColor, label());
    }
    else if (m_attributeType == RimWellPathAttribute::AttributeLiner)
    {            
        addColumnFeature(-0.5, -0.25, startDepth, endDepth, m_baseColor);
        addColumnFeature(0.25, 0.5, startDepth, endDepth, m_baseColor);
        addMarker(0.375, endDepth, 10, RiuQwtSymbol::SYMBOL_NONE, transparentBaseColor, label(), Qt::AlignTop);
    }
    else if (m_attributeType == RimWellPathAttribute::AttributePerforationInterval)
    {
        addColumnFeature(-0.75, -0.25, startDepth, endDepth, cvf::Color4f(cvf::Color3::WHITE, columnAlpha), Qt::Dense6Pattern);
        addColumnFeature(0.25, 0.75, startDepth, endDepth, cvf::Color4f(cvf::Color3::WHITE, columnAlpha), Qt::Dense6Pattern);
        addMarker(0.626, endDepth, 10, RiuQwtSymbol::SYMBOL_NONE, cvf::Color4f(cvf::Color3::WHITE, 0.0), label(), Qt::AlignTop);
        // Empirically a spacing of around 30 in depth between symbols looks good in the most relevant zoom levels.
        const double markerSpacing = 30;
        const int    markerSize    = 6;
        double markerDepth = startDepth + 0.5 * markerSpacing;
        while (markerDepth <= endDepth)
        {
            addMarker(-0.75, markerDepth, markerSize, RiuQwtSymbol::SYMBOL_LEFT_TRIANGLE, cvf::Color4f(cvf::Color3::BLACK, 1.0f));
            addMarker(0.75,  markerDepth, markerSize, RiuQwtSymbol::SYMBOL_RIGHT_TRIANGLE, cvf::Color4f(cvf::Color3::BLACK, 1.0f));

            markerDepth += markerSpacing;
        }
    }
    else if (m_attributeType == RimWellPathAttribute::AttributeFishbonesInterval)
    {
        addColumnFeature(-0.75, -0.25, startDepth, endDepth, cvf::Color4f(cvf::Color3::WHITE, columnAlpha), Qt::BDiagPattern);
        addColumnFeature(0.25, 0.75, startDepth, endDepth, cvf::Color4f(cvf::Color3::WHITE, columnAlpha), Qt::FDiagPattern);
        addMarker(0.625, endDepth, 10, RiuQwtSymbol::SYMBOL_NONE, cvf::Color4f(cvf::Color3::WHITE, 0.0f), label(), Qt::AlignTop);      
    }
    else if (m_attributeType == RimWellPathAttribute::AttributeFracture)
    {
        if (std::abs(m_endMD - m_startMD) < 20)
        {
            addMarker(0.625, endDepth, 10, RiuQwtSymbol::SYMBOL_NONE, cvf::Color4f(cvf::Color3::ORANGE_RED, 1.0f), label(), Qt::AlignTop, Qt::Horizontal, true, false);
        }
        else
        {
            addColumnFeature(-0.75, -0.25, startDepth, endDepth, cvf::Color4f(cvf::Color3::ORANGE_RED, columnAlpha), Qt::HorPattern);
            addColumnFeature(0.25, 0.75, startDepth, endDepth, cvf::Color4f(cvf::Color3::ORANGE_RED, columnAlpha), Qt::HorPattern);
            addMarker(0.625, endDepth, 10, RiuQwtSymbol::SYMBOL_NONE, cvf::Color4f(cvf::Color3::ORANGE_RED, 0.0f), label(), Qt::AlignTop);
        }
        
    }
    else if (m_attributeType == RimWellPathAttribute::AttributeICD)
    {
        addMarker(0.0, startDepth, 30, RiuQwtSymbol::SYMBOL_ELLIPSE, m_baseColor, label(), Qt::AlignCenter, Qt::Horizontal);
    }
    else if (m_attributeType == RimWellPathAttribute::AttributePacker)
    {
        addColumnFeature(-1.0, -0.25, startDepth, endDepth, cvf::Color4f(cvf::Color3::GRAY, 1.0f), Qt::SolidPattern);
        addColumnFeature(0.25, 1.0,   startDepth, endDepth, cvf::Color4f(cvf::Color3::GRAY, 1.0f), Qt::SolidPattern);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuWellPathAttributePlotObject::depthsOfDepthType() const
{
    double startDepth = m_startMD;
    double endDepth   = m_endMD;

    if (m_depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
    {
        cvf::Vec3d startPoint = m_wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(m_startMD);
        cvf::Vec3d endPoint   = m_wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(m_endMD);
        startDepth            = -startPoint.z();
        endDepth              = -endPoint.z();
    }
    return std::make_pair(startDepth, endDepth);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::addMarker(double                        posX,
                                               double                        depth,
                                               int                           size,
                                               RiuQwtSymbol::PointSymbolEnum symbolType,
                                               cvf::Color4f                  baseColor,
                                               QString                       label /*= QString("")*/,
                                               Qt::Alignment                 labelAlignment /*= Qt::AlignTop*/,
                                               Qt::Orientation               labelOrientation /*= Qt::Vertical*/,
                                               bool                          drawLine /*= false*/,
                                               bool                          contrastTextColor /*= true*/)
{
    QColor         bgColor       = RiaColorTools::toQColor(baseColor);
    QColor         textColor     = bgColor;
    if (contrastTextColor)
    {
        textColor = RiaColorTools::toQColor(RiaColorTools::constrastColor(baseColor.toColor3f()));
    }
    QwtPlotMarker* marker        = new QwtPlotMarker(label);
    RiuQwtSymbol*  symbol        = new RiuQwtSymbol(symbolType, "", RiuQwtSymbol::LabelRightOfSymbol);
    symbol->setSize(size);
    symbol->setColor(bgColor);
    marker->setSymbol(symbol);
    marker->setSpacing(2);
    marker->setXValue(posX);
    marker->setYValue(depth);

    if (m_showLabel)
    {
        QwtText labelText(label);
        labelText.setColor(textColor);
        QFont   font;
        font.setPointSize(8);
        labelText.setFont(font);
        marker->setLabel(labelText);
        marker->setLabelAlignment(labelAlignment);
        marker->setLabelOrientation(labelOrientation);
    }

    if (drawLine)
    {
        marker->setLineStyle(QwtPlotMarker::HLine);
        marker->setLinePen(bgColor, 5.0, Qt::DashLine);
    }
    m_plotFeatures.push_back(marker);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::addColumnFeature(double startX, double endX, double startDepth, double endDepth, cvf::Color4f baseColor, Qt::BrushStyle brushStyle)
{  
    drawColumnFeature(startX, endX, startDepth, endDepth, baseColor, Qt::SolidPattern);

    if (brushStyle != Qt::SolidPattern)
    {
        // If we're doing a special pattern, draw the pattern in black over the existing pattern
        drawColumnFeature(startX, endX, startDepth, endDepth, cvf::Color4f(cvf::Color3::BLACK), brushStyle);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::drawColumnFeature(double         startX,
                                                       double         endX,
                                                       double         startDepth,
                                                       double         endDepth,
                                                       cvf::Color4f   baseColor,
                                                       Qt::BrushStyle brushStyle)
{
    QwtPlotShapeItem* rightSide = new QwtPlotShapeItem(label());
    QPolygonF         polygon;
    QColor            color = RiaColorTools::toQColor(baseColor);

    polygon.push_back(QPointF(startX, startDepth));
    polygon.push_back(QPointF(endX, startDepth));
    polygon.push_back(QPointF(endX, endDepth));
    polygon.push_back(QPointF(startX, endDepth));
    polygon.push_back(QPointF(startX, startDepth));
    rightSide->setPolygon(polygon);
    rightSide->setXAxis(QwtPlot::xBottom);
    rightSide->setBrush(QBrush(color, brushStyle));
    m_plotFeatures.push_back(rightSide);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWellPathAttributePlotObject::xValueRange(double* minimumValue, double* maximumValue) const
{
    CVF_ASSERT(minimumValue && maximumValue);
    *maximumValue =  1.0;
    *minimumValue = -1.0;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWellPathAttributePlotObject::yValueRange(double* minimumValue, double* maximumValue) const
{
    CVF_ASSERT(minimumValue && maximumValue);

    if (minimumValue && maximumValue)
    {
        std::tie(*minimumValue, *maximumValue) = depthsOfDepthType();
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::setShowLabel(bool showLabel)
{
    m_showLabel = showLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::setDepthType(RimWellLogPlot::DepthTypeEnum depthType)
{
    m_depthType = depthType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::setBaseColor(const cvf::Color3f& baseColor)
{
    m_baseColor = cvf::Color4f(baseColor);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::setBaseColor(const cvf::Color4f& baseColor)
{
    m_baseColor = baseColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::setParentQwtPlotAndReplot(QwtPlot* plot)
{
    setParentQwtPlotNoReplot(plot);
    if (m_parentQwtPlot)
    {
        m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::setParentQwtPlotNoReplot(QwtPlot* plot)
{
    m_parentQwtPlot = plot;
    attachToQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::attachToQwt()
{    
    if (m_parentQwtPlot)
    {
        for (QwtPlotItem* plotFeature : m_plotFeatures)
        {
            plotFeature->attach(m_parentQwtPlot);            
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::detachFromQwt()
{
    for (QwtPlotItem* plotFeature : m_plotFeatures)
    {
        plotFeature->detach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathAttributePlotObject::reattachToQwt()
{
    detachFromQwt();
    attachToQwt();
}
