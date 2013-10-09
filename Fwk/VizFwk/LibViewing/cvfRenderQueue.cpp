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
#include "cvfRenderQueue.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderItem
/// \ingroup Viewing
///
/// Represents one item in the RenderQueue
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
RenderItem::RenderItem()
:   m_part(NULL),
    m_drawable(NULL),
    m_effect(NULL),
    m_projectedAreaPixels(0.0f),
    m_distance(0.0f)
{
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void RenderItem::set(Part* part, Drawable* drawable, Effect* effect, float projectedAreaPixels, float distance)
{
    m_part = part;
    m_drawable = drawable;
    m_effect = effect;
    m_projectedAreaPixels = projectedAreaPixels;
    m_distance = distance;
}



//==================================================================================================
///
/// \class cvf::RenderQueue
/// \ingroup Viewing
///
/// A class for storing a queue of render items.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderQueue::hintNumEntriesToAdd(size_t numNewEntries)
{
    m_renderItemsRefCounted.reserve(numNewEntries);
    m_renderItemsRaw.reserve(numNewEntries);
}


//--------------------------------------------------------------------------------------------------
/// Add a RenderItem to the queue
//--------------------------------------------------------------------------------------------------
void RenderQueue::add(Part* part, Drawable* drawable, Effect* effect, float projectedAreaPixels, float distance)
{
    RenderItem* item = new RenderItem;
    item->set(part, drawable, effect, projectedAreaPixels, distance);
    m_renderItemsRefCounted.push_back(item);
    m_renderItemsRaw.push_back(item);
}


//--------------------------------------------------------------------------------------------------
/// Get the number of items in the render queue
//--------------------------------------------------------------------------------------------------
size_t RenderQueue::count() const
{
    return m_renderItemsRaw.size();
}


//--------------------------------------------------------------------------------------------------
/// Sets the count to zero, but does not necessarily delete the actual contained items
//--------------------------------------------------------------------------------------------------
void RenderQueue::setCountZero()
{
    removeAll();
}


//--------------------------------------------------------------------------------------------------
/// Removes all entries and releases all memory
//--------------------------------------------------------------------------------------------------
void RenderQueue::removeAll()
{
    m_renderItemsRefCounted.clear();
    m_renderItemsRaw.clear();
}


//--------------------------------------------------------------------------------------------------
/// Get the RenderItem at the given index
//--------------------------------------------------------------------------------------------------
RenderItem* RenderQueue::item(size_t index)
{
    CVF_ASSERT(index < count());
    return m_renderItemsRaw[index];
}


//--------------------------------------------------------------------------------------------------
/// Get the native rendering queue
//--------------------------------------------------------------------------------------------------
std::vector<RenderItem*>* RenderQueue::renderItemsForSorting()
{
    return &m_renderItemsRaw;
}


} // namespace cvf

