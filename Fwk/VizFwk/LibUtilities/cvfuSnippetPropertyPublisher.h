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

namespace cvfu {

class Property;
class PropertySet;
class SnippetPropertyPublisherCallback;


//==================================================================================================
//
// SnippetPropertyPublisher
//
//==================================================================================================
class SnippetPropertyPublisher : public cvf::Object
{
public:
    SnippetPropertyPublisher();
    virtual ~SnippetPropertyPublisher();

    void            publishProperty(Property* property);
    PropertySet*    publishedPropertySet();
    
    void            notifyPropertyChangedBySnippet(Property* property);
    void            notifyPropertyValueChangedBySnippet(Property* property);

    void            registerCallback(SnippetPropertyPublisherCallback* callback);

private:
    cvf::ref<PropertySet>               m_publishedPropertySet; // The set of properties that are currently published
    SnippetPropertyPublisherCallback*   m_callback;             // Should really hold a ref here, but prefer not to derive from Object
};



//==================================================================================================
//
// SnippetPropertyPublisherCallback
//
//==================================================================================================
class SnippetPropertyPublisherCallback
{
public:
    virtual ~SnippetPropertyPublisherCallback() {}

    virtual void    onPublishedPropertySetChanged() = 0;
    virtual void    onPropertyChangedBySnippet(cvfu::Property* property) = 0;
    virtual void    onPropertyValueChangedBySnippet(cvfu::Property* property) = 0;
};


}

