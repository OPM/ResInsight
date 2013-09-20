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

#include "cvfCollection.h"

namespace cvf {

class Part;


//==================================================================================================
//
// Render hints for parts
//
//==================================================================================================
class PartRenderHint : public Object
{
public:
    PartRenderHint();

    void    set(float projectedAreaPixels, float centerDistance);

    float   projectedAreaPixels() const     { return m_projectedAreaPixels; }
    float   centerDistance() const          { return m_centerDistance; }

private:
    float   m_projectedAreaPixels;      // Default -1, which means not set
    float   m_centerDistance;           // Default -1, which means not set
};



//==================================================================================================
//
// Collection of parts with optional render hints
//
//==================================================================================================
class PartRenderHintCollection : public Object
{
public:
    PartRenderHintCollection();

    void            add(Part* part);        
    void            add(Part* part, float projectedAreaPixels, float distance);
    size_t          count() const;
    void            setCountZero();
    void            removeAll();

    Part*           part(size_t index);
    PartRenderHint* renderHint(size_t index);

private:
    size_t                      m_itemCount;    // We keep our own item count in order to avoid resizing our contained collections
    std::vector<Part*>          m_parts;
    Collection<PartRenderHint>  m_renderHints;
};

}
