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

#include "cvfRendering.h"
#include "cvfRenderQueueSorter.h"

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class ConstantFrameRate : public Object
{
public:
    ConstantFrameRate();

    void        setTargetFrameRate(double targetFrameRate);
    void        setMinNumPartsToDraw(int minNumParts);
    void        enableDistanceSorting(bool enableDistanceSorting);

    double      targetFrameRate() const;
    int         minNumPartsToDraw() const;
    bool        isDistanceSortingEnabled() const;

    void        attachRendering(Rendering* rendering);
    Rendering*  attachedRendering();

    void        adjust(double currentFrameRate);

private:
    double                  m_targetFrameRate;          // Target frame rate in frames per second
    int                     m_minNumPartsToDraw;        // The lower bound on the number of parts we're allowed to draw
    bool                    m_enableDistanceSorting;    // If true, we'll try and do distance sorting for the colest 10% of the parts

    double                  m_numPartsToDraw;           // Calculated number of parts to draw. This is the value we'll adjust
    ref<Rendering>          m_rendering;                // The attached rendering

    bool                    m_pixelSizeCullingWasOn;    
    ref<RenderQueueSorter>  m_previousSorter;           // The sorter that was current in the rendering when it was attached
};

}
