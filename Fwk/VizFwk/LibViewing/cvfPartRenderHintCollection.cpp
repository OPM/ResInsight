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
#include "cvfPartRenderHintCollection.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::PartRenderHint
/// \ingroup Viewing
///
/// Render hints for parts
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
PartRenderHint::PartRenderHint()
:   m_projectedAreaPixels(-1.0),
    m_centerDistance(-1.0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartRenderHint::set(float projectedAreaPixels, float centerDistance)
{
    m_projectedAreaPixels = projectedAreaPixels;
    m_centerDistance = centerDistance;
}



//==================================================================================================
///
/// \class cvf::PartRenderHintCollection
/// \ingroup Viewing
///
/// A collection of parts with optional render hints
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PartRenderHintCollection::PartRenderHintCollection()
{
    m_itemCount = 0;
}


//--------------------------------------------------------------------------------------------------
/// Add a part a and optionally a render hint to the part collection
//--------------------------------------------------------------------------------------------------
void PartRenderHintCollection::add(Part* part, float projectedAreaPixels, float distance)
{
    CVF_TIGHT_ASSERT(part);

    size_t arrSize = m_parts.size();
    CVF_TIGHT_ASSERT(arrSize == m_renderHints.size());

    PartRenderHint* hint = NULL;

    if (arrSize > m_itemCount)
    {
        m_parts[m_itemCount] = part;

        hint = m_renderHints[m_itemCount].p();
        if (!hint)
        {
            hint = new PartRenderHint();
            m_renderHints[m_itemCount] = hint;
        }
    }
    else
    {
        m_parts.push_back(part);
        
        hint = new PartRenderHint();
        m_renderHints.push_back(hint);
    }

    hint->set(projectedAreaPixels, distance);

    m_itemCount++;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartRenderHintCollection::add(Part* part)
{
    CVF_TIGHT_ASSERT(part);

    size_t arrSize = m_parts.size();
    CVF_TIGHT_ASSERT(arrSize == m_renderHints.size());

    if (arrSize > m_itemCount)
    {
        m_parts[m_itemCount] = part;
        m_renderHints[m_itemCount] = NULL;
    }
    else
    {
        m_parts.push_back(part);
        m_renderHints.push_back(NULL);
    }

    m_itemCount++;
}


//--------------------------------------------------------------------------------------------------
/// Get the number of parts in the collection
//--------------------------------------------------------------------------------------------------
size_t PartRenderHintCollection::count() const
{
    CVF_TIGHT_ASSERT(m_itemCount <= m_parts.size());
    CVF_TIGHT_ASSERT(m_itemCount <= m_renderHints.size());

    return m_itemCount;
}


//--------------------------------------------------------------------------------------------------
/// Sets the count to zero, but does not necessarily delete the actual contained items
//--------------------------------------------------------------------------------------------------
void PartRenderHintCollection::setCountZero()
{
    m_itemCount = 0;
}


//--------------------------------------------------------------------------------------------------
/// Removes all entries and releases all memory
//--------------------------------------------------------------------------------------------------
void PartRenderHintCollection::removeAll()
{
    m_parts.clear();
    m_renderHints.clear();
    m_itemCount = 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Part* PartRenderHintCollection::part(size_t index)
{
    CVF_TIGHT_ASSERT(index < m_itemCount);
    CVF_TIGHT_ASSERT(index < m_parts.size());

    return m_parts[index];
}


//--------------------------------------------------------------------------------------------------
/// Get the render hint for the part (if available) 
/// 
/// \param index    The index to use in this function.
///
/// \return  The render hint for the part.
///
/// Use this method to get the render hint for the part with the given index
//--------------------------------------------------------------------------------------------------
PartRenderHint* PartRenderHintCollection::renderHint(size_t index)
{
    CVF_TIGHT_ASSERT(index < m_itemCount);
    CVF_TIGHT_ASSERT(index < m_renderHints.size());

    return m_renderHints[index].p();
}


} // namespace cvf

