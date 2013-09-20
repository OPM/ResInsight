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
#include "cvfRenderQueueSorter.h"
#include "cvfRenderQueue.h"
#include "cvfEffect.h"
#include "cvfPart.h"
#include "cvfMath.h"
#include "cvfTBBControl.h"

#include <algorithm>

#ifdef CVF_USE_TBB

#ifdef WIN32
#pragma warning (push)
#pragma warning (disable: 4668 4365)
#include "tbb/parallel_sort.h"
#pragma warning (pop)
#endif

#endif

namespace cvf {



//==================================================================================================
//
// Helper class that wraps the sorting algorithms and chooses between std and TBB impl
//
//==================================================================================================
class SortAlgorithms
{
public:
    /// Constructor, configures usage of TBB or not
    // -----------------------------------------------------
    SortAlgorithms(bool useTBB)
    {
        m_useTBB = useTBB;
#ifndef CVF_USE_TBB
        m_useTBB = false;
#endif
    }

    /// Wrapper for std::sort() and tbb::parallell_sort()
    // -----------------------------------------------------
    template<typename RandomAccessIterator, typename Compare>
    void sort(RandomAccessIterator begin, RandomAccessIterator end, const Compare& comp)
    {
#ifdef CVF_USE_TBB
        if (m_useTBB)
        {
            tbb::parallel_sort(begin, end, comp);
        }
        else
#endif    
        {
            std::sort(begin, end, comp);
        }
    }

    /// Wrapper for std::partial_sort(), no corresponding TBB
    // -----------------------------------------------------
    template<typename RandomAccessIterator, typename Compare>
    void partial_sort(RandomAccessIterator begin, RandomAccessIterator middle, RandomAccessIterator end, const Compare& comp)
    {
        std::partial_sort(begin, middle, end, comp);
    }

private:
    bool    m_useTBB;
};



//==================================================================================================
//
// Class with compare function used by MINIMAL strategy
//
//==================================================================================================
class ComparatorMinimal
{
public:
    bool operator()(const RenderItem* a, const RenderItem* b) const
    {
        CVF_TIGHT_ASSERT(a && b);
        CVF_TIGHT_ASSERT(a->part());
        CVF_TIGHT_ASSERT(b->part());

        // Currently, only priority
		return (a->part()->priority() < b->part()->priority());
    }
};



//==================================================================================================
//
// Class with compare function used by EFFECT_ONLY strategy
//
//==================================================================================================
class ComparatorEffectOnly
{
public:
    bool operator()(const RenderItem* a, const RenderItem* b) const
    {
        CVF_TIGHT_ASSERT(a && b);
        CVF_TIGHT_ASSERT(a->part());
        CVF_TIGHT_ASSERT(b->part());

        // Always do priority first
        if (a->part()->priority() != b->part()->priority())
        {
            return (a->part()->priority() < b->part()->priority());
        }
        else
        {
            return (a->effect() < b->effect());
        }
    }
};



//==================================================================================================
//
// Class with compare function used by STANDARD strategy
//
//==================================================================================================
class ComparatorStandard
{
public:
    bool operator()(const RenderItem* a, const RenderItem* b) const
    {
        CVF_TIGHT_ASSERT(a && b);
        CVF_TIGHT_ASSERT(a->effect());
        CVF_TIGHT_ASSERT(b->effect());

		// This sorter is currently experimental!!

		// Sort order is:
		//  - priority
		//  - shader program
		//  - state set
		//  - effect
		//  - drawable

        if (a->part()->priority() != b->part()->priority())
        {
            return (a->part()->priority() < b->part()->priority());
        }
        else if (a->effect()->shaderProgram() != b->effect()->shaderProgram())
        {
            return (a->effect()->shaderProgram() < b->effect()->shaderProgram());
        }
        else if (a->effect()->renderStateSet() != b->effect()->renderStateSet())
        {
            return (a->effect()->renderStateSet() < b->effect()->renderStateSet());
        }
        else if (a->effect() != b->effect())
        {
            return (a->effect() < b->effect());
        }
        else
        {
            return (a->drawable() < b->drawable());
        }
    }
};


//==================================================================================================
//
// Class with compare function used by BACK_TO_FRONT strategy
//
//==================================================================================================
class ComparatorBackToFront
{
public:
    bool operator()(const RenderItem* a, const RenderItem* b) const
    {
        CVF_TIGHT_ASSERT(a && b);
        return (a->distance() > b->distance());
    }
};


//==================================================================================================
//
// Comparator class for comparing distance
//
//==================================================================================================
class ComparatorDistance
{
public:
    bool operator()(const RenderItem* a, const RenderItem* b) const
    {
        CVF_TIGHT_ASSERT(a && b);
        return (a->distance() < b->distance());
    }
};


//==================================================================================================
//
// Comparator class for comparing distance
//
//==================================================================================================
class ComparatorArea
{
public:
    bool operator()(const RenderItem* a, const RenderItem* b) const
    {
        CVF_TIGHT_ASSERT(a && b);
        return (a->projectedAreaPixels() > b->projectedAreaPixels());
    }
};



//==================================================================================================
///
/// \class cvf::RenderQueueSorter
/// \ingroup Viewing
///
/// Base class for all classes that sorts the render queue 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderQueueSorter::RenderQueueSorter()
{
}



//==================================================================================================
///
/// \class cvf::RenderQueueSorterBasic
/// \ingroup Viewing
///
/// Basic render queue sorter with an enumerated strategy. 
/// 
/// Does a single sorting pass with the specified strategy.
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderQueueSorterBasic::RenderQueueSorterBasic(SortStrategy strategy)
:   RenderQueueSorter()
{
    m_strategy = strategy;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderQueueSorterBasic::requireDistance() const
{
    if (m_strategy == BACK_TO_FRONT)
    {
        return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderQueueSorterBasic::requirePixelArea() const
{
    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderQueueSorterBasic::SortStrategy RenderQueueSorterBasic::strategy() const
{
    return m_strategy;
}


//--------------------------------------------------------------------------------------------------
/// Sort the render queue using the configured strategy
//--------------------------------------------------------------------------------------------------
void RenderQueueSorterBasic::sort(RenderQueue* renderQueue) const
{
    std::vector<RenderItem*>* renderItems = renderQueue->renderItemsForSorting();
    CVF_TIGHT_ASSERT(renderItems);

    bool useTBB = TBBControl::isEnabled();
    SortAlgorithms sa(useTBB);

	if (m_strategy == MINIMAL)
	{
		sa.sort(renderItems->begin(), renderItems->end(), ComparatorMinimal());
	}
    else if (m_strategy == EFFECT_ONLY)
    {
        sa.sort(renderItems->begin(), renderItems->end(), ComparatorEffectOnly());
    }
    else if (m_strategy == STANDARD)
    {
        sa.sort(renderItems->begin(), renderItems->end(), ComparatorStandard());
    }
    else if (m_strategy == BACK_TO_FRONT)
    {
        sa.sort(renderItems->begin(), renderItems->end(), ComparatorBackToFront());
    }
    else
    {
        CVF_FAIL_MSG("Unsupported sort strategy");
    }
}



//==================================================================================================
///
/// \class cvf::RenderQueueSorterTargetFramerate
/// \ingroup Viewing
///
/// 
///
//==================================================================================================
RenderQueueSorterTargetFramerate::RenderQueueSorterTargetFramerate()
:   RenderQueueSorter()
{
    m_maxNumPartsToDraw = std::numeric_limits<size_t>::max();
    m_numPartsToDistanceSort = 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderQueueSorterTargetFramerate::requireDistance() const
{
    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderQueueSorterTargetFramerate::requirePixelArea() const
{
    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderQueueSorterTargetFramerate::setMaxNumPartsToDraw(size_t maxNumPartsToDraw)
{
    m_maxNumPartsToDraw = maxNumPartsToDraw;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderQueueSorterTargetFramerate::clearMaxNumPartsToDraw()
{
    m_maxNumPartsToDraw = std::numeric_limits<size_t>::max();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderQueueSorterTargetFramerate::setNumPartsToDistanceSort(size_t numPartsToDistanceSort)
{
    m_numPartsToDistanceSort = numPartsToDistanceSort;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderQueueSorterTargetFramerate::sort(RenderQueue* renderQueue) const
{
    std::vector<RenderItem*>* renderItems = renderQueue->renderItemsForSorting();
    CVF_TIGHT_ASSERT(renderItems);

    std::vector<RenderItem*>::difference_type numItemsToDraw     = static_cast<std::vector<RenderItem*>::difference_type>(std::min(m_maxNumPartsToDraw, renderItems->size()));
    std::vector<RenderItem*>::difference_type numItemsToDistSort = static_cast<std::vector<RenderItem*>::difference_type>(std::min(m_numPartsToDistanceSort, renderItems->size()));

    bool useTBB = TBBControl::isEnabled();
    SortAlgorithms sa(useTBB);

    sa.partial_sort(renderItems->begin(), renderItems->begin() + numItemsToDistSort, renderItems->end(), ComparatorDistance());
    sa.sort(renderItems->begin(), renderItems->begin() + numItemsToDistSort, ComparatorEffectOnly());

    sa.sort(renderItems->begin() + numItemsToDistSort, renderItems->end(), ComparatorArea());
    sa.sort(renderItems->begin() + numItemsToDistSort, renderItems->begin() + static_cast<int>(numItemsToDraw), ComparatorEffectOnly());

    sa.sort(renderItems->begin() + numItemsToDraw, renderItems->end(), ComparatorEffectOnly());
}

} // namespace cvf
