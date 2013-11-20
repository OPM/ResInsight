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
#include "cvfOverlayItem.h"
#include "cvfRect.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::OverlayItem
/// \ingroup Render
///
/// A base class for all overlay items
/// 
/// The default layout scheme is OverlayItem::HORIZONTAL and the default anchor corner is 
/// OverlayItem::BOTTOM_LEFT. Note that when the items are laid out by a Rendering, items with
/// the OverlayItem::HORIZONTAL layout scheme will be placed first.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OverlayItem::OverlayItem()
:   m_layoutScheme(HORIZONTAL),
    m_anchorCorner(BOTTOM_LEFT),
    m_fixedPosition(0, 0)
{
}


//--------------------------------------------------------------------------------------------------
/// Specify how this overlay item should be laid out
/// 
/// The default value for \a layoutScheme is OverlayItem::HORIZONTAL, and the default value
/// for \a anchorCorner is OverlayItem::BOTTOM_LEFT.
//--------------------------------------------------------------------------------------------------
void OverlayItem::setLayout(LayoutScheme layoutScheme, AnchorCorner anchorCorner)
{
    CVF_ASSERT(layoutScheme == HORIZONTAL || layoutScheme == VERTICAL);
    m_layoutScheme = layoutScheme;
    m_anchorCorner = anchorCorner;
    m_fixedPosition.set(0, 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayItem::setLayoutFixedPosition(const Vec2i& fixedPosition)
{
    m_layoutScheme = FIXED_POSITION;
    m_anchorCorner = BOTTOM_LEFT;
    m_fixedPosition = fixedPosition;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OverlayItem::LayoutScheme OverlayItem::layoutScheme() const
{
    return m_layoutScheme;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OverlayItem::AnchorCorner OverlayItem::anchorCorner() const
{
    return m_anchorCorner;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2i OverlayItem::fixedPosition() const
{
    return m_fixedPosition;
}


//--------------------------------------------------------------------------------------------------
/// \fn virtual Vec2ui OverlayItem::sizeHint() = 0;
/// 
/// Returns the the size hint of this overlay item.
/// 
/// The returned size should be in pixels. Derived classes must implement this function.
/// The size returned by this function is currently the exact same size that will be passed in as
/// the \a size parameter to the render() or pick() member functions.
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/// Do hit test on the overlay item. 
/// 
/// Base class only does a check against the bounding box represented by \position and \size.
//--------------------------------------------------------------------------------------------------
bool OverlayItem::pick(int x, int y, const Vec2i& position, const Vec2ui& size) 
{
    Recti oglRect(position, static_cast<int>(size.x()), static_cast<int>(size.y()));

    return oglRect.contains(Vec2i(x, y));
}



} // namespace cvf
