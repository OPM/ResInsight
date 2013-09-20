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
#include "cvfPropertySet.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::PropertySet
/// \ingroup Core
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PropertySet::PropertySet(const String& classType)
    : m_classType(classType)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PropertySet::~PropertySet()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PropertySet::operator==(const PropertySet& rhs) const
{
    if (m_classType == rhs.m_classType)
    {
        return (m_propMap == rhs.m_propMap);
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String PropertySet::classType() const
{
    return m_classType;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PropertySet::isEmpty() const
{
    return m_propMap.empty();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Variant PropertySet::value(const String& key) const
{
    std::map<String, Variant>::const_iterator it = m_propMap.find(key);
    if (it != m_propMap.end())
    {
        return it->second;
    }
    else
    {
        return Variant();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PropertySet::setValue(const String& key, const Variant& data)
{
    m_propMap[key] = data;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PropertySet::contains(const String& key) const
{
    if (m_propMap.find(key) != m_propMap.end())
    {
        return true;
    }
    else
    {
        return false;
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<String> PropertySet::allKeys() const
{
    std::vector<String> all;
    std::map<String, Variant>::const_iterator it;
    for (it = m_propMap.begin(); it != m_propMap.end(); ++it)
    {
        all.push_back(it->first);
    }

    return all;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<Variant> PropertySet::allValues() const
{
    std::vector<Variant> all;
    std::map<String, Variant>::const_iterator it;
    for (it = m_propMap.begin(); it != m_propMap.end(); ++it)
    {
        all.push_back(it->second);
    }

    return all;
}




}  // namespace gc
