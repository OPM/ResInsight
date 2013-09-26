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

namespace cvf {

class RenderQueue;
class RenderItem;


//==================================================================================================
//
// 
//
//==================================================================================================
class RenderQueueSorter : public Object
{
public:
    RenderQueueSorter();

    virtual void    sort(RenderQueue* renderQueue) const = 0;

    virtual bool    requireDistance() const = 0;    
    virtual bool    requirePixelArea() const = 0;    
};



//==================================================================================================
//
// 
//
//==================================================================================================
class RenderQueueSorterBasic : public RenderQueueSorter
{
public:
    enum SortStrategy
    {
        MINIMAL,		// Does a minimum of sorting, currently only sorts on priority
        EFFECT_ONLY,    // A simple render queue sorter ordering the items by effect object (and priority)
        STANDARD,       // Currently experimental, but should eventually become the default sorter
        BACK_TO_FRONT   // Sorts parts in a back to front order (useful for e.g. transparency)
    };

public:
    RenderQueueSorterBasic(SortStrategy strategy);

    SortStrategy    strategy() const;
    virtual void    sort(RenderQueue* renderQueue) const;

    virtual bool    requireDistance() const;    
    virtual bool    requirePixelArea() const;    

private:
    SortStrategy    m_strategy;
};



//==================================================================================================
//
// 
//
//==================================================================================================
class RenderQueueSorterTargetFramerate : public RenderQueueSorter
{
public:
    RenderQueueSorterTargetFramerate();

    void            setMaxNumPartsToDraw(size_t maxNumPartsToDraw);
    void            clearMaxNumPartsToDraw();
    void            setNumPartsToDistanceSort(size_t numPartsToDistanceSort);

    virtual void    sort(RenderQueue* renderQueue) const;

    virtual bool    requireDistance() const;    
    virtual bool    requirePixelArea() const;    

private:
    size_t m_maxNumPartsToDraw;        // The maximum number of parts we will draw. Will do as little work as possible for the remaining parts. 
    size_t m_numPartsToDistanceSort;   // The number of parts that will be sorted according to distance. 
};


}
