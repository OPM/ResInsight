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

#include "cvfBase.h"
#include "cvfHitItemCollection.h"

#include <algorithm>

namespace cvf {



//==================================================================================================
///
/// \class cvf::HitItemCollection
/// \ingroup Viewing
///
/// This class is used to aggregate hits during picking. The items can be sorted by distance  
/// with the sort() method.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
HitItemCollection::HitItemCollection()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
HitItemCollection::~HitItemCollection()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void HitItemCollection::add(HitItem* item)
{
    m_items.push_back(item);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t HitItemCollection::count() const
{
    return m_items.size();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
HitItem* HitItemCollection::item(size_t index) 
{
    CVF_ASSERT(index < count());
    return m_items.at(index).p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const HitItem* HitItemCollection::item(size_t index) const 
{
    CVF_ASSERT(index < count());
    return m_items.at(index).p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
HitItem* HitItemCollection::firstItem() 
{
    if (m_items.size() > 0)
    {
        return m_items.at(0).p();
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const HitItem* HitItemCollection::firstItem() const
{
    if (m_items.size() > 0)
    {
        return m_items.at(0).p();
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void HitItemCollection::sort()
{
    std::sort(m_items.begin(), m_items.end(), compareFunc);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool HitItemCollection::compareFunc(const ref<HitItem>& a, const ref<HitItem>& b) 
{ 
    return a->distanceAlongRay() < b->distanceAlongRay(); 
}


} // namespace cvf

