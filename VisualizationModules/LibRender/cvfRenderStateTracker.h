//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include "cvfRenderState.h"

namespace cvf {

class RenderStateSet;
class OpenGLContext;


//==================================================================================================
//
// RenderStateTracker
//
//==================================================================================================
class RenderStateTracker
{
public:
    RenderStateTracker();

    void    resetAndApplyDefaultRenderStates(OpenGLContext* oglContext);
    void    applyRenderStates(OpenGLContext* oglContext, const RenderStateSet* newRSS, const RenderStateSet* prevRSS);

private:
    void    setupDefaultRenderStates();

private:
    int                 m_renderStateUsageTable[RenderState::COUNT];
    const RenderState*  m_currentRenderStates[RenderState::COUNT];  // Pointers to the render states that are current
    ref<RenderState>    m_defaultRenderStates[RenderState::COUNT];  // Default render states, contains all known render states initialized to their defaults
};

}
