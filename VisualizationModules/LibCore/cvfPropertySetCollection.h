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
#include "cvfPropertySet.h"
#include "cvfCollection.h"

#include <map>

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class PropertySetCollection : public Object
{
public:
    PropertySetCollection();
    ~PropertySetCollection();

    size_t              count() const;
    PropertySet*        propertySet(size_t index);
    const PropertySet*  propertySet(size_t index) const;
    void                addPropertySet(PropertySet* propertySet);

    size_t              countOfType(const String& classType) const;
    PropertySet*        propertySetOfType(const String& classType, size_t index);
    PropertySet*        firstPropertySetOfType(const String& classType);

private:
    Collection<PropertySet> m_propertySets;
};


}  // namespace cvf
