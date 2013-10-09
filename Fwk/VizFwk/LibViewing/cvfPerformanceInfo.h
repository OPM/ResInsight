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


#pragma once

namespace cvf {


//==================================================================================================
//
// PerformanceInfo
//
//==================================================================================================
class PerformanceInfo
{
public:
    double  totalDrawTime;              ///< Total amount of time (in seconds) used to draw the last frame

    double  computeVisiblePartsTime;    ///< Time (in seconds) used to create the visible part collection
    double  buildRenderQueueTime;       ///< Time (in seconds) used to build the rendering queue
    double  sortRenderQueueTime;        ///< Time (in seconds) used to sort the render queue
    double  renderEngineTime;           ///< Time (in seconds) used to render the pre-processed rendering queue

    size_t  visiblePartsCount;          ///< Number of visible parts
    size_t  renderedPartsCount;         ///< Number of parts that was drawn
    size_t  vertexCount;                ///< Number of vertices (nodes, points) used to draw.
    size_t  triangleCount;              ///< Number of triangles (GL_TRIANGELS) 
    size_t  openGLPrimitiveCount;       ///< Total number of GL primitives drawn (lines, points, polygons, etc). For strips the number of resulting triangles.
    size_t  applyRenderStateCount;      ///< Number of render state changes
    size_t  shaderProgramChangesCount;  ///< Number of shader program changes

public:
    PerformanceInfo();
    
    double  averageTotalDrawTime() const;

    void    clear();
    void    resetCurrentTimers();

    void    update(const PerformanceInfo& perf);

private:
    static const int NUM_PERFORMANCE_HISTORY_ITEMS = 10;
    double           m_totalDrawTimeHistory[NUM_PERFORMANCE_HISTORY_ITEMS];
    int              m_nextHistoryItem;
};

}
