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

#include "RiuWellPathComponentPlotItem.h"

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

#include "qwt_plot.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_shapeitem.h"

#include <QBrush>
#include <Qt>


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathComponentPlotItem::RiuWellPathComponentPlotItem(const RimWellPath* wellPath)
    : m_wellPath(wellPath)
    , m_completionType(RiaDefines::WELL_PATH)
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
RiuWellPathComponentPlotItem::RiuWellPathComponentPlotItem(const RimWellPath* wellPath, const RimWellPathComponentInterface* completion)
{
    CVF_ASSERT(wellPath && completion);

    m_completionType = completion->componentType();
    m_startMD       = completion->startMD();
    m_endMD         = completion->endMD();
    m_label         = completion->componentLabel();
    m_legendTitle   = completion->componentTypeLabel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathComponentPlotItem::~RiuWellPathComponentPlotItem()
{
    detachFromQwt();

    if (m_parentQwtPlot)
    {
        m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWellPathComponentPlotItem::label() const
{
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::loadDataAndUpdate(bool updateParentPlot)
{
    onLoadDataAndUpdate(updateParentPlot);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RiuWellPathComponentPlotItem::completionType() const
{
    return m_completionType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::onLoadDataAndUpdate(bool updateParentPlot)
{   
    double startDepth, endDepth;
    std::tie(startDepth, endDepth) = depthsOfDepthType();
    double midDepth = 0.5 * (startDepth + endDepth);

    float columnAlpha = 0.9f;

    cvf::Color4f transparentBaseColor = m_baseColor;
    transparentBaseColor.a() = 0.0;
    if (m_completionType == RiaDefines::WELL_PATH)
    {
        addColumnFeature(-0.25, 0.25, startDepth, endDepth, m_baseColor);
    }
    else if (m_completionType == RiaDefines::CASING)
    {
        addColumnFeature(-0.75, -0.5, startDepth, endDepth, m_baseColor);
        addColumnFeature(0.5, 0.75, startDepth, endDepth, m_baseColor);
        addMarker(-0.75, endDepth,10, RiuQwtSymbol::SYMBOL_LEFT_ANGLED_TRIANGLE,  m_baseColor);
        addMarker(0.75, endDepth, 10, RiuQwtSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE, m_baseColor, label());
    }
    else if (m_completionType == RiaDefines::LINER)
    {            
        addColumnFeature(-0.5, -0.25, startDepth, endDepth, m_baseColor);
        addColumnFeature(0.25, 0.5, startDepth, endDepth, m_baseColor);
        addMarker(0.75, endDepth, 10, RiuQwtSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE, transparentBaseColor, label());
    }
    else if (m_completionType == RiaDefines::PERFORATION_INTERVAL)
    {
        addColumnFeature(-0.75, -0.25, startDepth, endDepth, cvf::Color4f(cvf::Color3::WHITE, columnAlpha), Qt::Dense6Pattern);
        addColumnFeature(0.25, 0.75, startDepth, endDepth, cvf::Color4f(cvf::Color3::WHITE, columnAlpha), Qt::Dense6Pattern);
        // Empirically a spacing of around 30 in depth between symbols looks good in the most relevant zoom levels.
        const double markerSpacing = 30;
        const int    markerSize    = 6;
        double markerDepth = startDepth;
        while (markerDepth <= endDepth)
        {
            addMarker(-0.75, markerDepth, markerSize, RiuQwtSymbol::SYMBOL_LEFT_TRIANGLE, cvf::Color4f(cvf::Color3::BLACK, 1.0f));
            addMarker(0.75,  markerDepth, markerSize, RiuQwtSymbol::SYMBOL_RIGHT_TRIANGLE, cvf::Color4f(cvf::Color3::BLACK, 1.0f));

            markerDepth += markerSpacing;
        }
        addMarker(0.75, midDepth, 10, RiuQwtSymbol::SYMBOL_RIGHT_TRIANGLE, cvf::Color4f(cvf::Color3::BLACK, 0.0), label());

        QwtPlotItem* legendItem1 = createMarker(16.0, 0.0, 6, RiuQwtSymbol::SYMBOL_RIGHT_TRIANGLE, cvf::Color4f(cvf::Color3::BLACK, 1.0f));
        legendItem1->setLegendIconSize(QSize(4, 8));
        QwtPlotItem* legendItem2 = createMarker(16.0, 8.0, 6, RiuQwtSymbol::SYMBOL_RIGHT_TRIANGLE, cvf::Color4f(cvf::Color3::BLACK, 1.0f));
        legendItem2->setLegendIconSize(QSize(4, 8));
        m_combinedAttributeGroup.addLegendItem(legendItem1);
        m_combinedAttributeGroup.addLegendItem(legendItem2);
    }
    else if (m_completionType == RiaDefines::FISHBONES)
    {
        addColumnFeature(-0.75, -0.25, startDepth, endDepth, cvf::Color4f(cvf::Color3::WHITE, columnAlpha), Qt::BDiagPattern);
        addColumnFeature(0.25, 0.75, startDepth, endDepth, cvf::Color4f(cvf::Color3::WHITE, columnAlpha), Qt::FDiagPattern);
        addMarker(0.75, midDepth, 10, RiuQwtSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE, cvf::Color4f(cvf::Color3::BLACK, 0.0f), label());      
    }
    else if (m_completionType == RiaDefines::FRACTURE)
    {
        addColumnFeature(-0.75, -0.25, startDepth, endDepth, cvf::Color4f(cvf::Color3::ORANGE_RED, columnAlpha), Qt::SolidPattern);
        addColumnFeature(0.25, 0.75, startDepth, endDepth, cvf::Color4f(cvf::Color3::ORANGE_RED, columnAlpha), Qt::SolidPattern);
        addMarker(0.75, startDepth, 10, RiuQwtSymbol::SYMBOL_NONE, cvf::Color4f(cvf::Color3::ORANGE_RED, 1.0f), "", Qt::AlignTop | Qt::AlignRight, Qt::Horizontal, true);
        addMarker(0.75, endDepth, 10, RiuQwtSymbol::SYMBOL_NONE, cvf::Color4f(cvf::Color3::ORANGE_RED, 1.0f), "", Qt::AlignTop | Qt::AlignRight, Qt::Horizontal, true);
        addMarker(0.75, startDepth, 1, RiuQwtSymbol::SYMBOL_RIGHT_ANGLED_TRIANGLE, cvf::Color4f(cvf::Color3::ORANGE_RED, 0.0f), label(), Qt::AlignTop | Qt::AlignRight);
    }
    else if (m_completionType == RiaDefines::ICD)
    {
        addMarker(0.0, startDepth, 30, RiuQwtSymbol::SYMBOL_ELLIPSE, m_baseColor, label(), Qt::AlignCenter, Qt::Horizontal);
    }
    else if (m_completionType == RiaDefines::PACKER)
    {
        addColumnFeature(-1.0, -0.25, startDepth, endDepth, cvf::Color4f(cvf::Color3::GRAY, 1.0f), Qt::DiagCrossPattern);
        addColumnFeature(0.25, 1.0, startDepth,   endDepth, cvf::Color4f(cvf::Color3::GRAY, 1.0f), Qt::DiagCrossPattern);
    }
    m_combinedAttributeGroup.setTitle(legendTitle());
    m_combinedAttributeGroup.setLegendIconSize(QSize(20, 16));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RiuWellPathComponentPlotItem::depthsOfDepthType() const
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
void RiuWellPathComponentPlotItem::addMarker(double posX,
                                               double depth,
                                               int size,
                                               RiuQwtSymbol::PointSymbolEnum symbolType,
                                               cvf::Color4f baseColor,
                                               QString label /*= QString("")*/,
                                               Qt::Alignment labelAlignment /*= Qt::AlignTop*/,
                                               Qt::Orientation labelOrientation /*= Qt::Vertical*/,
                                               bool drawLine /*= false*/,
                                               bool contrastTextColor /*= true*/)
{
    QwtPlotItem* marker = createMarker(posX, depth, size, symbolType, baseColor, label, labelAlignment, labelOrientation, drawLine, contrastTextColor);
    m_combinedAttributeGroup.addPlotItem(marker);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotItem* RiuWellPathComponentPlotItem::createMarker(double posX, double depth, int size, RiuQwtSymbol::PointSymbolEnum symbolType, cvf::Color4f baseColor, QString label /*= QString("")*/, Qt::Alignment labelAlignment /*= Qt::AlignTop*/, Qt::Orientation labelOrientation /*= Qt::Vertical*/, bool drawLine /*= false*/, bool contrastTextColor /*= true*/)
{
    QColor         bgColor = RiaColorTools::toQColor(baseColor);
    QColor         textColor = RiaColorTools::toQColor(baseColor.toColor3f(), 1.0);
    if (contrastTextColor)
    {
        textColor = RiaColorTools::toQColor(RiaColorTools::constrastColor(baseColor.toColor3f()));
    }
    QwtPlotMarker* marker = new QwtPlotMarker(label);
    RiuQwtSymbol*  symbol = new RiuQwtSymbol(symbolType, "", RiuQwtSymbol::LabelRightOfSymbol);
    symbol->setSize(size);
    symbol->setColor(bgColor);
    marker->setSymbol(symbol);
    marker->setSpacing(6);
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
        marker->setLinePen(bgColor, 2.0, Qt::SolidLine);
    }
    return marker;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::addColumnFeature(double startX,
                                                      double endX,
                                                      double startDepth,
                                                      double endDepth,
                                                      cvf::Color4f baseColor,
                                                      Qt::BrushStyle brushStyle /*= Qt::SolidPattern*/)
{
    QwtPlotItem* backgroundShape = createColumnShape(startX, endX, startDepth, endDepth, baseColor, Qt::SolidPattern);
    m_combinedAttributeGroup.addPlotItem(backgroundShape);
    if (startX >= 0.0)
    {
        QwtPlotItem* legendShape = createColumnShape(0.0, 16.0, 0.0, 16.0, baseColor, Qt::SolidPattern);
        m_combinedAttributeGroup.addLegendItem(legendShape);
    }
    if (brushStyle != Qt::SolidPattern)
    {
        // If we're doing a special pattern, draw the pattern in black over the existing pattern
        QwtPlotItem* patternShape = createColumnShape(startX, endX, startDepth, endDepth, cvf::Color4f(cvf::Color3::BLACK), brushStyle);
        m_combinedAttributeGroup.addPlotItem(patternShape);
        if (startX >= 0.0)
        {
            QwtPlotItem* legendShape = createColumnShape(0.0, 16.0, 0.0, 16.0, cvf::Color4f(cvf::Color3::BLACK), brushStyle);
            m_combinedAttributeGroup.addLegendItem(legendShape);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotItem* RiuWellPathComponentPlotItem::createColumnShape(double         startX,
                                                                 double         endX,
                                                                 double         startDepth,
                                                                 double         endDepth,
                                                                 cvf::Color4f   baseColor,
                                                                 Qt::BrushStyle brushStyle)
{
    QwtPlotShapeItem* columnShape = new QwtPlotShapeItem(label());
    QPolygonF         polygon;
    QColor            color = RiaColorTools::toQColor(baseColor);

    polygon.push_back(QPointF(startX, startDepth));
    polygon.push_back(QPointF(endX, startDepth));
    polygon.push_back(QPointF(endX, endDepth));
    polygon.push_back(QPointF(startX, endDepth));
    polygon.push_back(QPointF(startX, startDepth));
    columnShape->setPolygon(polygon);
    columnShape->setXAxis(QwtPlot::xBottom);
    columnShape->setBrush(QBrush(color, brushStyle));
    columnShape->setLegendMode(QwtPlotShapeItem::LegendShape);
    columnShape->setLegendIconSize(QSize(16, 16));
    return columnShape;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWellPathComponentPlotItem::xValueRange(double* minimumValue, double* maximumValue) const
{
    CVF_ASSERT(minimumValue && maximumValue);
    *maximumValue =  1.0;
    *minimumValue = -1.0;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWellPathComponentPlotItem::yValueRange(double* minimumValue, double* maximumValue) const
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
void RiuWellPathComponentPlotItem::setShowLabel(bool showLabel)
{
    m_showLabel = showLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::setDepthType(RimWellLogPlot::DepthTypeEnum depthType)
{
    m_depthType = depthType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::setBaseColor(const cvf::Color3f& baseColor)
{
    m_baseColor = cvf::Color4f(baseColor);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::setBaseColor(const cvf::Color4f& baseColor)
{
    m_baseColor = baseColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::setContributeToLegend(bool contributeToLegend)
{
    bool actuallyContributeToLegend = contributeToLegend && (m_completionType == RiaDefines::FISHBONES ||
                                                             m_completionType == RiaDefines::FRACTURE ||
                                                             m_completionType == RiaDefines::PERFORATION_INTERVAL ||
                                                             m_completionType == RiaDefines::PACKER);
    m_combinedAttributeGroup.setItemAttribute(QwtPlotItem::Legend, actuallyContributeToLegend);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::setParentQwtPlotAndReplot(QwtPlot* plot)
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
void RiuWellPathComponentPlotItem::setParentQwtPlotNoReplot(QwtPlot* plot)
{
    m_parentQwtPlot = plot;
    attachToQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::attachToQwt()
{    
    if (m_parentQwtPlot)
    {
        m_combinedAttributeGroup.attach(m_parentQwtPlot);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::detachFromQwt()
{
    m_combinedAttributeGroup.detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellPathComponentPlotItem::reattachToQwt()
{
    detachFromQwt();
    attachToQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWellPathComponentPlotItem::legendTitle() const
{
    return m_legendTitle;
}
