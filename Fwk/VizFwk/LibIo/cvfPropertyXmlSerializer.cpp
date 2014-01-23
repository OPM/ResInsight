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
#include "cvfPropertyXmlSerializer.h"
#include "cvfPropertySetCollection.h"
#include "cvfXml.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::PropertyXmlSerializer
/// \ingroup Io
///
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PropertyXmlSerializer::toXml(const PropertySetCollection& propertySetCollection, XmlElement* parent)
{
    CVF_ASSERT(parent);

    XmlElement* xmlPropSetColl = parent->addChildElement("PropertySetCollection");
    CVF_ASSERT(xmlPropSetColl);

    size_t numPropSets = propertySetCollection.count();
    for (size_t i = 0; i < numPropSets; i++)
    {
        const PropertySet* ps = propertySetCollection.propertySet(i);
        CVF_ASSERT(ps);

        createAddXmlElementFromPropertySet(*ps, xmlPropSetColl);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PropertyXmlSerializer::createAddXmlElementFromPropertySet(const PropertySet& propertySet, XmlElement* parent)
{
    CVF_ASSERT(parent);

    XmlElement* xmlPropSet = parent->addChildElement("PropertySet");
    CVF_ASSERT(xmlPropSet);

    xmlPropSet->setAttributeString("classType", propertySet.classType());

    std::vector<String> keys = propertySet.allKeys();
    std::vector<Variant> values = propertySet.allValues();
    size_t numKeyValues = keys.size();
    CVF_ASSERT(numKeyValues == values.size());

    for (size_t i = 0; i < numKeyValues; i++)
    {
        const String& key = keys[i];
        const Variant& value = values[i];

        if (value.isValid())
        {
            XmlElement* xmlKeyValueElement = createAddXmlElementFromVariant(value, xmlPropSet);
            CVF_ASSERT(xmlKeyValueElement);

            xmlKeyValueElement->setAttributeString("key", key);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Populate the PropertySet from the specified XML element
///
/// The XML element passed as a parameter is assumed to be a PropertySet element
//--------------------------------------------------------------------------------------------------
void PropertyXmlSerializer::toPropertySetCollection(const XmlElement& xmlPropertySetCollectionElement, PropertySetCollection* propertySetCollection)
{
    CVF_ASSERT(xmlPropertySetCollectionElement.name() == "PropertySetCollection");
    CVF_ASSERT(propertySetCollection);

    const XmlElement* xmlElem = xmlPropertySetCollectionElement.firstChildElement("PropertySet");
    while (xmlElem)
    {
        ref<PropertySet> ps = createPropertySetFromXmlElement(*xmlElem);
        propertySetCollection->addPropertySet(ps.p());

        xmlElem = xmlElem->nextSiblingElement("PropertySet");
    }
}


//--------------------------------------------------------------------------------------------------
/// Add an XML element representing the variant to \a parent
///
/// Note that invalid variants will not be added
//--------------------------------------------------------------------------------------------------
XmlElement* PropertyXmlSerializer::createAddXmlElementFromVariant(const Variant& variant, XmlElement* parent)
{
    CVF_ASSERT(variant.isValid());
    CVF_ASSERT(parent);

    Variant::Type variantType = variant.type();
    switch (variantType)
    {
        case Variant::INT:      return parent->addChildElement("Int",        String(variant.getInt()));
        case Variant::UINT:     return parent->addChildElement("UInt",       String(variant.getUInt()));
        case Variant::DOUBLE:   return parent->addChildElement("Double",     String::number(variant.getDouble()));
        case Variant::FLOAT:    return parent->addChildElement("Float",      String::number(variant.getFloat()));
        case Variant::BOOL:     return parent->addChildElement("Bool",       variant.getBool() ? "true" : "false");
        case Variant::VEC3D:    return parent->addChildElement("Vec3d",      valueTextFromVec3dVariant(variant));
        case Variant::COLOR3F:  return parent->addChildElement("Color3f",    valueTextFromColor3fVariant(variant));
        case Variant::STRING:   return parent->addChildElement("String",     variant.getString());
        case Variant::ARRAY:    return createAddXmlElementFromArrayVariant(variant, parent);
        case Variant::INVALID:  return NULL;
    }

    CVF_FAIL_MSG("Unhandled variant type");
    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
XmlElement* PropertyXmlSerializer::createAddXmlElementFromArrayVariant(const Variant& arrayVariant, XmlElement* parent)
{
    XmlElement* xmlArrayElement = parent->addChildElement("Array");

    CVF_ASSERT(arrayVariant.type() == Variant::ARRAY);
    std::vector<Variant> arr = arrayVariant.getArray();

    size_t numArrayItems = arr.size();
    for (size_t i = 0; i < numArrayItems; i++)
    {
        const Variant& variant = arr[i];
        if (variant.isValid())
        {
            createAddXmlElementFromVariant(variant, xmlArrayElement);
        }
    }

    return xmlArrayElement;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String PropertyXmlSerializer::valueTextFromVec3dVariant(const Variant& variant)
{
    CVF_ASSERT(variant.type() == Variant::VEC3D);
    Vec3d val = variant.getVec3d();

    String txt =  String::number(val.x()) + " " + String::number(val.y()) + " " + String::number(val.z());
    return txt;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String PropertyXmlSerializer::valueTextFromColor3fVariant(const Variant& variant)
{
    CVF_ASSERT(variant.type() == Variant::COLOR3F);
    Color3f val = variant.getColor3f();

    String txt =  String::number(val.r()) + " " + String::number(val.g()) + " " + String::number(val.b());
    return txt;
}


//--------------------------------------------------------------------------------------------------
/// Create a new PropertySet from the specified XML element
///
/// The XML element passed as a parameter is assumed to be a PropertySet element
//--------------------------------------------------------------------------------------------------
ref<PropertySet> PropertyXmlSerializer::createPropertySetFromXmlElement(const XmlElement& xmlPropertySetElement)
{
    CVF_ASSERT(xmlPropertySetElement.name() == "PropertySet");

    String propSetClassType = xmlPropertySetElement.getAttributeString("classType");
    ref<PropertySet> ps = new PropertySet(propSetClassType);

    const XmlElement* xmlElem = xmlPropertySetElement.firstChildElement();
    while (xmlElem)
    {
        Variant variant = variantFromXmlElement(*xmlElem);
        if (variant.isValid())
        {
            String key = xmlElem->getAttributeString("key");
            if (!key.isEmpty())
            {
                ps->setValue(key, variant);
            }
        }

        xmlElem = xmlElem->nextSiblingElement();
    }

    return ps;
}


//--------------------------------------------------------------------------------------------------
/// Create variant from the specified XML element
///
/// If the passed XML element is not recognized as a variant, an invalid variant will be returned
//--------------------------------------------------------------------------------------------------
cvf::Variant PropertyXmlSerializer::variantFromXmlElement(const XmlElement& xmlVariantElement)
{
    const String elementName = xmlVariantElement.name().toLower();
    const String valueText = xmlVariantElement.valueText();

    if (elementName == "int")
    {
        bool ok = false;
        int val = valueText.toInt(&ok);
        if (ok)
        {
            return Variant(val);
        }
    }
    else if (elementName == "uint")
    {
        bool ok = false;
        uint val = valueText.toUInt(&ok);
        if (ok)
        {
            return Variant(val);
        }
    }
    else if (elementName == "double")
    {
        bool ok = false;
        double val = valueText.toDouble(&ok);
        if (ok)
        {
            return Variant(val);
        }
    }
    else if (elementName == "float")
    {
        bool ok = false;
        float val = valueText.toFloat(&ok);
        if (ok)
        {
            return Variant(val);
        }
    }
    else if (elementName == "bool")
    {
        String valStr = valueText.toLower();
        bool val = (valStr == "true") ? true : false;
        return Variant(val);
    }
    else if (elementName == "vec3d")
    {
        return variantFromVec3dValueText(valueText);
    }
    else if (elementName == "color3f")
    {
        return variantFromColor3fValueText(valueText);
    }
    else if (elementName == "string")
    {
        return Variant(valueText);
    }
    else if (elementName == "array")
    {
        return arrayVariantFromXmlElement(xmlVariantElement);
    }

    return Variant();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Variant PropertyXmlSerializer::arrayVariantFromXmlElement(const XmlElement& xmlArrayVariantElement)
{
    CVF_ASSERT(xmlArrayVariantElement.name().toLower() == "array");

    std::vector<Variant> arr;

    const XmlElement* xmlElem = xmlArrayVariantElement.firstChildElement();
    while (xmlElem)
    {
        Variant variant = variantFromXmlElement(*xmlElem);
        if (variant.isValid())
        {
            arr.push_back(variant);
        }

        xmlElem = xmlElem->nextSiblingElement();
    }

    return Variant(arr);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Variant PropertyXmlSerializer::variantFromVec3dValueText(const String& vec3dValueText)
{
    std::vector<String> components = vec3dValueText.split();
    if (components.size() == 3)
    {
        Vec3d val(0, 0, 0);
        bool ok0 = false;
        bool ok1 = false;
        bool ok2 = false;
        val.x() = components[0].toDouble(&ok0);
        val.y() = components[1].toDouble(&ok1);
        val.z() = components[2].toDouble(&ok2);
        if (ok0 && ok1 && ok2)
        {
            return Variant(val);
        }
    }

    return Variant();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Variant PropertyXmlSerializer::variantFromColor3fValueText(const String& color3fValueText)
{
    std::vector<String> components = color3fValueText.split();
    if (components.size() == 3)
    {
        Color3f val(0, 0, 0);
        bool ok0 = false;
        bool ok1 = false;
        bool ok2 = false;
        val.r() = components[0].toFloat(&ok0);
        val.g() = components[1].toFloat(&ok1);
        val.b() = components[2].toFloat(&ok2);
        if (ok0 && ok1 && ok2)
        {
            return Variant(val);
        }
    }

    return Variant();
}


}  // namespace cvf
