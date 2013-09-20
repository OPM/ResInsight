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
#include "cvfPropertySetCollection.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::PropertySetCollection
/// \ingroup Core
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PropertySetCollection::PropertySetCollection()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PropertySetCollection::~PropertySetCollection()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t PropertySetCollection::count() const
{
    return m_propertySets.size();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PropertySet* PropertySetCollection::propertySet(size_t index)
{
    CVF_ASSERT(index < m_propertySets.size());
    return m_propertySets.at(index);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const PropertySet* PropertySetCollection::propertySet(size_t index) const
{
    CVF_ASSERT(index < m_propertySets.size());
    return m_propertySets.at(index);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PropertySetCollection::addPropertySet(PropertySet* propertySet)
{
    CVF_ASSERT(propertySet);
    m_propertySets.push_back(propertySet);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t PropertySetCollection::countOfType(const String& classType) const
{
    size_t classCount = 0;
    const size_t totNumSets = m_propertySets.size();
    for (size_t i = 0; i < totNumSets; i++)
    {
        const PropertySet* ps = m_propertySets.at(i);
        if (ps->classType() == classType)
        {
            classCount++;
        }
    }

    return classCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PropertySet* PropertySetCollection::propertySetOfType(const String& classType, size_t index)
{
    size_t classCount = 0;
    const size_t totNumSets = m_propertySets.size();
    for (size_t i = 0; i < totNumSets; i++)
    {
        PropertySet* ps = m_propertySets.at(i);
        if (ps->classType() == classType)
        {
            if (classCount == index)
            {
                return ps;
            }

            classCount++;
        }
    }

    CVF_FAIL_MSG("Specified index is of of range");

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PropertySet* PropertySetCollection::firstPropertySetOfType(const String& classType)
{
    const size_t totNumSets = m_propertySets.size();
    for (size_t i = 0; i < totNumSets; i++)
    {
        PropertySet* ps = m_propertySets.at(i);
        if (ps->classType() == classType)
        {
            return ps;
        }
    }

    return NULL;
}


}  // namespace gc
