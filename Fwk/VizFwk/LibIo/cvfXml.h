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
#include "cvfString.h"
#include "cvfColor3.h"
#include "cvfArray.h"

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class XmlElement
{
public:
    virtual ~XmlElement() {}

    virtual void                setAttributeBool(const cvf::String& attributeName, bool val) = 0;
    virtual void                setAttributeString(const cvf::String& attributeName, const cvf::String& val) = 0;
    virtual void                setAttributeInt(const cvf::String& attributeName, int val) = 0;
    virtual void                setAttributeInt64(const cvf::String& attributeName, cvf::int64 val) = 0;
    virtual void                setAttributeFloat(const cvf::String& attributeName, float val) = 0;
    virtual void                setAttributeDouble(const cvf::String& attributeName, double val) = 0;
    virtual void                setAttributeVector(const cvf::String& attributeName, const cvf::Vec3d& val) = 0;
    virtual void                setAttributeColor(const cvf::String& attributeName, const cvf::Color3f& color) = 0;

    virtual bool                getAttributeBool(const cvf::String& attributeName, bool defaultVal, bool* found = NULL) const = 0;
    virtual cvf::String         getAttributeString(const cvf::String& attributeName, const cvf::String& defaultVal = "", bool* found = NULL) const = 0;
    virtual int                 getAttributeInt(const cvf::String& attributeName, int defaultVal, bool* found = NULL) const = 0;
    virtual cvf::int64          getAttributeInt64(const cvf::String& attributeName, cvf::int64 defaultVal, bool* found = NULL) const = 0;
    virtual float               getAttributeFloat(const cvf::String& attributeName, float defaultVal, bool* found = NULL) const = 0;
    virtual double              getAttributeDouble(const cvf::String& attributeName, double defaultVal, bool* found = NULL) const = 0;
    virtual cvf::Vec3d          getAttributeVector(const cvf::String& attributeName, const cvf::Vec3d& defaultVal, bool* found = NULL) const = 0;
    virtual cvf::Color3f        getAttributeColor(const cvf::String& attributeName, const cvf::Color3f& defaulColor, bool* found = NULL) const = 0;

    virtual void                getAttributes(std::vector<cvf::String>* names, std::vector<cvf::String>* values) = 0;

    virtual XmlElement*         firstChildElement() = 0;
    virtual const XmlElement*   firstChildElement() const = 0;
    virtual XmlElement*         firstChildElement(const cvf::String& elementName) = 0;
    virtual const XmlElement*   firstChildElement(const cvf::String& elementName) const = 0;

    virtual XmlElement*         nextSiblingElement() = 0;
    virtual const XmlElement*   nextSiblingElement() const = 0;
    virtual XmlElement*         nextSiblingElement(const cvf::String& elementName) = 0;
    virtual const XmlElement*   nextSiblingElement(const cvf::String& elementName) const = 0;

    virtual XmlElement*         addChildElement(const cvf::String& elementName, const cvf::String& stringValue = cvf::String()) = 0;
    virtual bool                removeChildElement(XmlElement* element) = 0; 

    virtual cvf::String         name() const = 0;
    virtual cvf::String         valueText() const = 0;
    virtual bool                setValueText(const cvf::String& text) = 0;
};



//==================================================================================================
//
// 
//
//==================================================================================================
class XmlDocument : public cvf::Object
{
public:
    static ref<XmlDocument>     create();

    static void                 preserveWhiteSpace(bool preserve);

    virtual void                clear() = 0;
    virtual bool                error() const = 0;

    virtual XmlElement*         createRootElement(const cvf::String& rootName, int iID = -1, const cvf::String& namespaceString = "http://ceetron.com") = 0;
    virtual XmlElement*         getRootElement(const cvf::String& rootName) = 0;
    virtual const XmlElement*   getRootElement(const cvf::String& rootName) const = 0;

    virtual bool                loadFile(const cvf::String& fileName) = 0;
    virtual bool                saveFile(const cvf::String& fileName) = 0;
    virtual bool                saveCompactFile(const cvf::String& fileName) = 0;

    virtual bool                setFromRawData(const cvf::UByteArray& buffer) = 0;
    virtual void                getAsRawData(cvf::UByteArray* buffer) const = 0;
};

}
