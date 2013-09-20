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
#include "cvfAssert.h"
#include "cvfRenderStateSet.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStateSet
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateSet::RenderStateSet()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint RenderStateSet::count() const
{
    return static_cast<uint>(m_renderStates.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderState* RenderStateSet::renderState(uint index)
{
    CVF_ASSERT(index < count());

    return m_renderStates[index].p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RenderState* RenderStateSet::renderState(uint index) const
{
    CVF_ASSERT(index < count());

    return m_renderStates[index].p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateSet::setRenderState(RenderState* renderState)
{
    CVF_ASSERT(renderState);

    RenderState::Type incomingType = renderState->type();

    size_t numStates = m_renderStates.size();
    size_t i;
    for (i = 0; i < numStates; i++)
    {
        if (m_renderStates[i]->type() == incomingType)
        {
            m_renderStates[i] = renderState;
            return;
        }
    }

    m_renderStates.push_back(renderState);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderState* RenderStateSet::renderStateOfType(RenderState::Type type)
{
    size_t numStates = m_renderStates.size();
    size_t i;
    for (i = 0; i < numStates; i++)
    {
        if (m_renderStates[i]->type() == type)
        {
            return m_renderStates[i].p();
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RenderState* RenderStateSet::renderStateOfType(RenderState::Type type) const
{
    size_t numStates = m_renderStates.size();
    size_t i;
    for (i = 0; i < numStates; i++)
    {
        if (m_renderStates[i]->type() == type)
        {
            return m_renderStates[i].p();
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateSet::removeRenderState(const RenderState* renderState)
{
    m_renderStates.erase(renderState);
}


} // namespace cvf

