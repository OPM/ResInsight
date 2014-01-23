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

#include "cvfuSnippetPropertyPublisher.h"
#include "cvfuProperty.h"

namespace cvfu {



//==================================================================================================
///
/// \class cvfu::SnippetPropertyPublisher
/// \ingroup Utilities
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SnippetPropertyPublisher::SnippetPropertyPublisher()
:   m_publishedPropertySet(new PropertySet),
    m_callback(NULL)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SnippetPropertyPublisher::~SnippetPropertyPublisher()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SnippetPropertyPublisher::publishProperty(Property* property)
{
    m_publishedPropertySet->add(property);
    if (m_callback)
    {
        m_callback->onPublishedPropertySetChanged();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PropertySet* SnippetPropertyPublisher::publishedPropertySet()
{
    return m_publishedPropertySet.p();
}


//--------------------------------------------------------------------------------------------------
/// Should be called when a snippet modifies a property in some way (including the value and config)
//--------------------------------------------------------------------------------------------------
void SnippetPropertyPublisher::notifyPropertyChangedBySnippet(Property* property)
{
    if (m_callback)
    {
        m_callback->onPropertyChangedBySnippet(property);
    }
}


//--------------------------------------------------------------------------------------------------
/// Should be called when a snippet changes the value of a property
//--------------------------------------------------------------------------------------------------
void SnippetPropertyPublisher::notifyPropertyValueChangedBySnippet(Property* property)
{
    if (m_callback)
    {
        m_callback->onPropertyValueChangedBySnippet(property);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SnippetPropertyPublisher::registerCallback(SnippetPropertyPublisherCallback* callback)
{
    m_callback = callback;
}

} // namespace cvfu




