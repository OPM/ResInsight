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

#include "cvfObject.h"
#include "cvfHitItem.h"

#include <vector>

namespace cvf {



//==================================================================================================
//
// 
//
//==================================================================================================
class HitItemCollection : public Object
{
public:
    HitItemCollection();
    ~HitItemCollection();

    void            add(HitItem* item);
    size_t          count() const;
    HitItem*        item(size_t index);
    const HitItem*  item(size_t index) const;
    HitItem*        firstItem();
    const HitItem*  firstItem() const;

    void            sort();

private:
    static bool compareFunc(const ref<HitItem>& a, const ref<HitItem>& b);

private:
    std::vector<ref<HitItem> > m_items;
};

}
