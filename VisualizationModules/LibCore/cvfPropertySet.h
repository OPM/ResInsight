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
#include "cvfVariant.h"
#include "cvfString.h"

#include <map>

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class PropertySet : public Object
{
public:
    PropertySet(const String& classType);
    ~PropertySet();

    bool                    operator==(const PropertySet& rhs) const;

    String                  classType() const;
    bool                    isEmpty() const;

    Variant                 value(const String& key) const;
    void                    setValue(const String& key, const Variant& data);
    bool                    contains(const String& key) const;

    std::vector<String>     allKeys() const;
    std::vector<Variant>    allValues() const;

private:
    String                      m_classType;
    std::map<String, Variant>   m_propMap;
};


}  // namespace cvf
