//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfCamera.h"
#include "cvfPerformanceInfo.h"

#include "cvfqtPerformanceInfoHud.h"
#include "cvfOpenGLResourceManager.h"

#include <QtCore/QString>
#include <QPainter>

#include <vector>

namespace cvfqt {



//==================================================================================================
///
/// \class cvfqt::PerformanceInfoHud 
/// \ingroup GuiQt
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PerformanceInfoHud::PerformanceInfoHud()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PerformanceInfoHud::addStrings(const cvf::PerformanceInfo& performanceInfo)
{
    double fps = 0.0;
    if (performanceInfo.totalDrawTime > 0.0) fps = 1.0/performanceInfo.totalDrawTime;
    double millPrimPerSecond = fps*static_cast<double>(performanceInfo.openGLPrimitiveCount)/1000000.0;

    double avgTotalDrawTime = performanceInfo.averageTotalDrawTime();
    double avgFPS = 0.0;
    if (avgTotalDrawTime > 0.0) avgFPS = 1.0/avgTotalDrawTime;
    double avgMillPrimPerSecond = avgFPS*static_cast<double>(performanceInfo.openGLPrimitiveCount)/1000000.0;

    double openGLMemSize = (performanceInfo.vertexCount*24.0 + 4.0*(performanceInfo.triangleCount*3.0))/(1024*1024);

    // Add spacing if we already have contents
    if (!m_drawStrings.empty()) m_drawStrings.push_back("");

    m_drawStrings.push_back(QString("FPS: %1 (last: %2)")                .arg(avgFPS, 0, 'f', 2).arg(fps, 0, 'f', 2));
    m_drawStrings.push_back(QString("Mill. OpenGL prim/s: %1 (last: %2)").arg(avgMillPrimPerSecond, 0, 'f', 2).arg(millPrimPerSecond, 0, 'f', 2));
    m_drawStrings.push_back("");
    m_drawStrings.push_back(QString("Total draw time: %1 ms (last: %2)") .arg(avgTotalDrawTime*1000.0, 0, 'f', 3).arg(performanceInfo.totalDrawTime*1000.0, 0, 'f', 3));
    m_drawStrings.push_back(QString("Compute Vis Parts: %1 ms")          .arg(performanceInfo.computeVisiblePartsTime*1000.0, 0, 'f', 3));
    m_drawStrings.push_back(QString("Build RenderQueue: %1 ms")          .arg(performanceInfo.buildRenderQueueTime*1000.0, 0, 'f', 3));
    m_drawStrings.push_back(QString("Sort RenderQueue: %1 ms")           .arg(performanceInfo.sortRenderQueueTime*1000.0, 0, 'f', 3));
    m_drawStrings.push_back(QString("Render: %1 ms")                     .arg(performanceInfo.renderEngineTime*1000.0, 0, 'f', 3));

    double other = performanceInfo.totalDrawTime - performanceInfo.computeVisiblePartsTime - performanceInfo.buildRenderQueueTime - performanceInfo.sortRenderQueueTime - performanceInfo.renderEngineTime;
    m_drawStrings.push_back(QString("Other: %1 ms")              .arg(other*1000.0, 0, 'f', 3));
    //!!!!  strArray.push_back(QString("Last pick time used: %1 ms").arg(m_lastPickTimeUsed*1000.0, 0, 'f', 3));
    m_drawStrings.push_back("");
    m_drawStrings.push_back(QString("Num visible parts: %1")     .arg((int)performanceInfo.visiblePartsCount));
    m_drawStrings.push_back(QString("Num rendered parts: %1")    .arg((int)performanceInfo.renderedPartsCount));

    if (performanceInfo.vertexCount > 0)
    {
        m_drawStrings.push_back(QString("Num vertices: %1")          .arg((int)performanceInfo.vertexCount));
        m_drawStrings.push_back(QString("Num triangles: %1")         .arg((int)performanceInfo.triangleCount));
        m_drawStrings.push_back(QString("OpenGL primitives: %1 mill").arg((double)performanceInfo.openGLPrimitiveCount/1000000.0, 0, 'f', 3));
        m_drawStrings.push_back(QString("Total OpenGL array size: %1 MB").arg(openGLMemSize, 0, 'f', 2));
    }

    m_drawStrings.push_back(QString("Apply render state count: %1").arg((int)performanceInfo.applyRenderStateCount));
    m_drawStrings.push_back(QString("Change shader program count: %1").arg((int)performanceInfo.shaderProgramChangesCount));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PerformanceInfoHud::addStrings(const cvf::OpenGLResourceManager& resourceManager)
{
    double vboMemUsageMB = static_cast<double>(resourceManager.bufferObjectMemoryUsage())/(1024.0*1024.0);
    m_drawStrings.push_back(QString("Num VBOs: %1").arg((int)resourceManager.bufferObjectCount()));
    m_drawStrings.push_back(QString("VBO memory usage: %1 MB").arg(vboMemUsageMB, 0, 'f', 2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PerformanceInfoHud::addStrings(const cvf::Camera& camera)
{
    // Add spacing if we already have contents
    if (!m_drawStrings.empty()) m_drawStrings.push_back("");

    cvf::Vec3d eye, vrp, vup, viewdir;
    camera.toLookAt(&eye, &vrp, &vup);
    viewdir = (vrp - eye).getNormalized();

    m_drawStrings.push_back(QString("Clip planes: %1 -> %2").arg(camera.nearPlane()).arg(camera.farPlane()));
    m_drawStrings.push_back(QString("Eye: <%1, %2, %3>").arg(eye.x()).arg(eye.y()).arg(eye.z()));
    m_drawStrings.push_back(QString("ViewDir: <%1, %2, %3>").arg(viewdir.x()).arg(viewdir.y()).arg(viewdir.z()));
    m_drawStrings.push_back(QString("Up: <%1, %2, %3>").arg(vup.x()).arg(vup.y()).arg(vup.z()));

    if (camera.fieldOfViewYDeg() != cvf::UNDEFINED_DOUBLE)
    {
        m_drawStrings.push_back(QString("FieldOfView: %1").arg(camera.fieldOfViewYDeg()));
    }
    else
    {
        m_drawStrings.push_back(QString("FieldOfView: N/A"));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PerformanceInfoHud::addString(const QString& str)
{
    m_drawStrings.push_back(str);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PerformanceInfoHud::draw(QPainter *painter, int widgetWidth, int widgetHeight)
{

/*
//!!!!  strArray.push_back(QString("Last pick time used: %1 ms").arg(m_lastPickTimeUsed*1000.0, 0, 'f', 3));
//!!!!    m_useDisplayLists ? strArray.push_back("Display lists: ON") : strArray.push_back("Display lists: OFF");
*/

    // Draw the strings
    QFontMetrics metrics = painter->fontMetrics();
    int border = qMax(4, metrics.leading());

    painter->setRenderHint(QPainter::TextAntialiasing);

    int x = 10;
    int y = 10;
    int spacing = 5;

    std::vector<QRect> rects;

    int maxWidth = 0;
    int totalHeight = 0;

    int i;
    for (i = 0; i < m_drawStrings.size(); i++)
    {
        QString text = m_drawStrings[i];
        QRect rect = metrics.boundingRect(0, 0, widgetWidth - 2*border, int(widgetHeight*0.125), Qt::AlignCenter | Qt::TextWordWrap, text);

        if (rect.width() > maxWidth) maxWidth = rect.width();

        if (text.isEmpty())
        {
            totalHeight += spacing;
        }
        else
        {
            totalHeight += rect.height();
        }

        rects.push_back(rect);
    }

    QRect r1(x - border, y - border, maxWidth + 2*border, totalHeight + spacing*m_drawStrings.size());

    painter->fillRect(r1, QColor(0, 0, 0, 127));
    painter->setPen(Qt::white);
    painter->fillRect(r1, QColor(0, 0, 0, 127));

    for (i = 0; i < m_drawStrings.size(); i++)
    {
        QString text = m_drawStrings[i];
        QRect rect = rects[i];

        if (text.isEmpty())
        {
            y += spacing;
        }
        else
        {
            painter->drawText(x, y, rect.width(), rect.height(), Qt::AlignCenter | Qt::TextWordWrap, text);
            y += rect.height() + spacing;
        }
    }

    // Clear all contents
    m_drawStrings.clear();
}


} // namespace cvfqt


