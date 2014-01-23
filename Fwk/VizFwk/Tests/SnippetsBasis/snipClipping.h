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

#include "cvfuTestSnippet.h"
#include "cvfuProperty.h"
#include "cvfClipPlaneSet.h"

namespace snip {

using namespace cvf;
using namespace cvfu;


//==================================================================================================
//
// 
//
//==================================================================================================
class Clipping : public TestSnippet
{
    CVFU_DECLARE_SNIPPET("Clipping planes");

public:
    Clipping();
    ~Clipping();

    virtual bool        onInitialize();
    virtual void        onPaintEvent(PostEventAction* postEventAction);
    virtual void        onPropertyChanged(Property* property, PostEventAction* postEventAction);

private:
    void                createAndPublishProperties();
    void                configureClipPlaneSetFromProperties();
    ref<Effect>         createClippingFixedHeadlightEffect(Color3f objectColor);

private:
    ref<PropertyBool>   m_propEnableXClip;
    ref<PropertyBool>   m_propEnableYClip;
    ref<PropertyBool>   m_propEnableZClip;
    ref<PropertyBool>   m_propEnableUserClip;
    ref<PropertyDouble> m_propPosX;
    ref<PropertyDouble> m_propPosY;
    ref<PropertyDouble> m_propPosZ;
    ref<PropertyDouble> m_propNormalX;
    ref<PropertyDouble> m_propNormalY;
    ref<PropertyDouble> m_propNormalZ;

    ref<ClipPlaneSet>   m_clipPlaneSet;
};

}

