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
#include "cvfVariant.h"

namespace cvf {

class PropertySetCollection;
class PropertySet;
class XmlDocument;
class XmlElement;


//==================================================================================================
//
// 
//
//==================================================================================================
class PropertyXmlSerializer 
{
public:
    static void             toXml(const PropertySetCollection& propertySetCollection, XmlElement* parent);
    static void             toPropertySetCollection(const XmlElement& xmlPropertySetCollectionElement, PropertySetCollection* propertySetCollection);

private:
    static void             createAddXmlElementFromPropertySet(const PropertySet& propertySet, XmlElement* parent);
    static XmlElement*      createAddXmlElementFromVariant(const Variant& variant, XmlElement* parent);
    static XmlElement*      createAddXmlElementFromArrayVariant(const Variant& arrayVariant, XmlElement* parent);
    static String           valueTextFromVec3dVariant(const Variant& variant);
    static String           valueTextFromColor3fVariant(const Variant& variant);
    static ref<PropertySet> createPropertySetFromXmlElement(const XmlElement& xmlPropertySetElement);
    static Variant          variantFromXmlElement(const XmlElement& xmlVariantElement);
    static Variant          arrayVariantFromXmlElement(const XmlElement& xmlArrayVariantElement);
    static Variant          variantFromVec3dValueText(const String& vec3dValueText);
    static Variant          variantFromColor3fValueText(const String& color3fValueText);
};

} // namespace cvf
