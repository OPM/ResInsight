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

#include "cvfObject.h"

#include <vector>

namespace cvf {

class Part;
class Drawable;
class Effect;


//==================================================================================================
//
// Represents one item in the RenderQueue
// 
//==================================================================================================
class RenderItem : public Object
{
public:
    RenderItem();

    void            set(Part* part, Drawable* drawable, Effect* effect, float projectedAreaPixels, float distance);

    Part*           part()                          { return m_part; }
    const Part*     part() const                    { return m_part; }
    Drawable*       drawable()                      { return m_drawable; }
    const Drawable* drawable() const                { return m_drawable; }
    Effect*         effect()                        { return m_effect; }
    const Effect*   effect() const                  { return m_effect; }

    float           projectedAreaPixels() const     { return m_projectedAreaPixels; }
    float           distance() const                { return m_distance; }

private:
    Part*       m_part;
    Drawable*   m_drawable;             // The drawable for the LOD level that should be used for rendering 
    Effect*     m_effect;               // Effect for the relevant LOD level
    float       m_projectedAreaPixels;  // 
    float       m_distance;             // 
};



//==================================================================================================
//
// RenderQueue
//
//==================================================================================================
class RenderQueue : public Object
{
public:
    void            hintNumEntriesToAdd(size_t numNewEntries);
    void            add(Part* part, Drawable* drawable, Effect* effect, float projectedAreaPixels, float distance);
    size_t          count() const;
    RenderItem*     item(size_t index);
    void            setCountZero();
    void            removeAll();

    std::vector<RenderItem*>*  renderItemsForSorting();

private:
    std::vector<ref<RenderItem> >   m_renderItemsRefCounted;
    std::vector<RenderItem*>        m_renderItemsRaw;
};

}
