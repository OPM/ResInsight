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
#include "cvfMath.h"

#include "cvfXml.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4365 )
#endif

#include "cvfTinyXmlFused.hpp"

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <iostream>
#include <fstream>

namespace cvf {
   
using namespace cvf_tinyXML;

using cvf::String;



//==================================================================================================
///
/// \class cvf::TinyXmlElement
/// \ingroup Io
///
/// 
///
//==================================================================================================
class XmlElementImpl : public XmlElement, public TiXmlElement
{
public:
    XmlElementImpl(const String& name);

    void            setAttributeBool(const String& attributeName, bool val);
    void            setAttributeString(const String& attributeName, const String& val);
    void            setAttributeInt(const String& attributeName, int val);
    void            setAttributeInt64(const String& attributeName, cvf::int64 val);
    void            setAttributeFloat(const String& attributeName, float val);
    void            setAttributeDouble(const String& attributeName, double val);
    void            setAttributeVector(const String& attributeName, const cvf::Vec3d& val);
    void            setAttributeColor(const String& attributeName, const cvf::Color3f& color);

    bool            getAttributeBool(const String& attributeName, bool defaultVal, bool* found = NULL) const;
    String          getAttributeString(const String& attributeName, const String& defaultVal = "", bool* found = NULL) const;
    int             getAttributeInt(const String& attributeName, int defaultVal, bool* found = NULL) const;
    cvf::int64      getAttributeInt64(const String& attributeName, cvf::int64 defaultVal, bool* found = NULL) const;
    float           getAttributeFloat(const String& attributeName, float defaultVal, bool* found = NULL) const;
    double          getAttributeDouble(const String& attributeName, double defaultVal, bool* found = NULL) const;
    cvf::Vec3d      getAttributeVector(const cvf::String& attributeName, const cvf::Vec3d& defaultVal, bool* found = NULL) const;
    cvf::Color3f    getAttributeColor(const String& attributeName, const cvf::Color3f& defaultColor, bool* found = NULL) const;

    void            getAttributes(std::vector<cvf::String>* names, std::vector<cvf::String>* values);

    XmlElement*         firstChildElement();
    const XmlElement*   firstChildElement() const;
    XmlElement*         firstChildElement(const cvf::String& elementName);
    const XmlElement*   firstChildElement(const cvf::String& elementName) const;
    bool                removeChildElement(XmlElement* element); 

    XmlElement*         nextSiblingElement();
    const XmlElement*   nextSiblingElement() const;
    XmlElement*         nextSiblingElement(const cvf::String& elementName);
    const XmlElement*   nextSiblingElement(const cvf::String& elementName) const;

    XmlElement*         addChildElement(const String& elementName, const cvf::String& stringValue);

    String      name() const;
    String      valueText() const;
    bool        setValueText(const String& text);

    void        cloneElementChild(const TiXmlElement* tiXmlElement);

};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
XmlElementImpl::XmlElementImpl(const String& name) 
:   TiXmlElement(name.toUtf8().ptr())
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlElementImpl::setAttributeBool(const String& attributeName, bool val)
{
    setAttributeString(attributeName, val ? "true" : "false");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlElementImpl::setAttributeString(const String& attributeName, const String& val)
{
    cvf::CharArray charArr = val.toUtf8();

    SetAttribute(attributeName.toUtf8().ptr(), charArr.ptr());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlElementImpl::setAttributeInt(const String& attributeName, int val)
{
    SetAttribute(attributeName.toUtf8().ptr(), val);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlElementImpl::setAttributeInt64(const String& attributeName, cvf::int64 val)
{
    std::wstringstream sstr;
    sstr << val;
    std::wstring tmp = sstr.str();
    String cvfString(tmp);

    setAttributeString(attributeName, cvfString);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool XmlElementImpl::getAttributeBool(const String& attributeName, bool defaultVal, bool* found /*= NULL*/) const
{
    if (found) *found = false;

    String val;
    const char* attrValue = Attribute(attributeName.toUtf8().ptr());
    if (attrValue)
    {
        val = attrValue;
    }
    
    if (val.isEmpty()) return defaultVal;

    if (found) *found = true;

    if (val[0] == '1') return true;
    if (val[0] == '0') return false;

    val.toLower();

    if (val == "yes" || val == "on" || val == "true") return true;
    if (val == "no"  || val == "off" || val == "false") return false;

    return defaultVal;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float XmlElementImpl::getAttributeFloat(const String& attributeName, float defaultVal, bool* found /*= NULL*/) const
{
    if (found) *found = false;

    String val;
    const char* attrValue = Attribute(attributeName.toUtf8().ptr());
    if (attrValue)
    {
        val = attrValue;
    }
    if (val.isEmpty()) return defaultVal;

    if (found) *found = true;

    if (val == "Undefined") return cvf::UNDEFINED_FLOAT;

    float floatValue = defaultVal;
    if (QueryFloatAttribute(attributeName.toUtf8().ptr(), &floatValue) != TIXML_SUCCESS) return defaultVal;

    return floatValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double XmlElementImpl::getAttributeDouble(const String& attributeName, double defaultVal, bool* found /*= NULL*/) const
{
    if (found) *found = false;

    String val;
    const char* attrValue = Attribute(attributeName.toUtf8().ptr());
    if (attrValue)
    {
        val = attrValue;
    }

    if (val.isEmpty()) return defaultVal;

    if (found) *found = true;

    if (val == "Undefined") return cvf::UNDEFINED_DOUBLE;

    double doubleValue = defaultVal;
    if (QueryDoubleAttribute(attributeName.toUtf8().ptr(), &doubleValue) != TIXML_SUCCESS) return defaultVal;

    return doubleValue;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d XmlElementImpl::getAttributeVector(const String& attributeName, const cvf::Vec3d& defaultVal, bool* found) const
{
    if (found) *found = false;

    String val;
    const char* attrValue = Attribute(attributeName.toUtf8().ptr());
    if (attrValue)
    {
        val = attrValue;
    }

    if (val.isEmpty()) return defaultVal;

    if (found) *found = true;

    std::vector<String> tokens = val.split();
    if (tokens.size() == 3)
    {
        cvf::Vec3d vec;
        vec.x() = atof(tokens[0].toAscii().ptr());
        vec.y() = atof(tokens[1].toAscii().ptr());
        vec.z() = atof(tokens[2].toAscii().ptr());
        return vec;
    }

    return defaultVal;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f XmlElementImpl::getAttributeColor(const String& attributeName, const cvf::Color3f& defaultColor, bool* found /*= NULL*/) const
{
    if (found) *found = false;

    String val;
    const char* attrValue = Attribute(attributeName.toUtf8().ptr());
    if (attrValue)
    {
        val = attrValue;
    }

    if (val.isEmpty()) return defaultColor;

    if (found) *found = true;

    std::vector<String> tokens = val.split();
    if (tokens.size() == 3)
    {
        cvf::Color3f color;
        color.r() = (float)atof(tokens[0].toAscii().ptr());
        color.g() = (float)atof(tokens[1].toAscii().ptr());
        color.b() = (float)atof(tokens[2].toAscii().ptr());
        return color;
    }

    return defaultColor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlElementImpl::getAttributes(std::vector<cvf::String>* names, std::vector<cvf::String>* values)
{
    TiXmlAttribute* pTiAttribute = FirstAttribute();

    while (pTiAttribute)
    {
        names->push_back(pTiAttribute->Name());
        values->push_back(pTiAttribute->Value());

        pTiAttribute = pTiAttribute->Next();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
XmlElement* XmlElementImpl::firstChildElement()
{
    return static_cast<XmlElementImpl*>(FirstChildElement());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const XmlElement* XmlElementImpl::firstChildElement() const
{
    return static_cast<const XmlElementImpl*>(FirstChildElement());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
XmlElement* XmlElementImpl::firstChildElement(const String& elementName)
{
    return static_cast<XmlElementImpl*>(FirstChildElement(elementName.toUtf8().ptr()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const XmlElement* XmlElementImpl::firstChildElement(const String& elementName) const
{
    return static_cast<const XmlElementImpl*>(FirstChildElement(elementName.toUtf8().ptr()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool XmlElementImpl::removeChildElement(XmlElement* element)
{
    return RemoveChild(static_cast<XmlElementImpl*>(element));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
XmlElement* XmlElementImpl::nextSiblingElement()
{
    return static_cast<XmlElementImpl*>(NextSiblingElement());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const XmlElement* XmlElementImpl::nextSiblingElement() const
{
    return static_cast<const XmlElementImpl*>(NextSiblingElement());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
XmlElement* XmlElementImpl::nextSiblingElement(const String& elementName)
{
    return static_cast<XmlElementImpl*>(NextSiblingElement(elementName.toUtf8().ptr()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const XmlElement* XmlElementImpl::nextSiblingElement(const String& elementName) const
{
    return static_cast<const XmlElementImpl*>(NextSiblingElement(elementName.toUtf8().ptr()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String XmlElementImpl::name() const
{
    String sVal = Value();
    return sVal;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String XmlElementImpl::valueText() const
{
    const TiXmlText* pText = NULL;
    const TiXmlNode* pNode = NULL;
    for (pNode = FirstChild(); pNode; pNode = pNode->NextSibling())
    {
        pText = pNode->ToText();

        if (pText) break;
    }

    if (!pText) return "";

    String sVal = String::fromUtf8(pText->Value());

    return sVal;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool XmlElementImpl::setValueText(const String& text)
{
    TiXmlText* pText = NULL;
    TiXmlNode* pNode = NULL;
    for (pNode = FirstChild(); pNode; pNode = pNode->NextSibling())
    {
        pText = pNode->ToText();
        if (pText) break;
    }

    cvf::CharArray charArr = text.toUtf8();

    if (pText) 
    {
       pText->SetValue(charArr.ptr());
    }
    else
    {
        pText = new TiXmlText(charArr.ptr());
        LinkEndChild(pText);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlElementImpl::setAttributeFloat(const String& attributeName, float val)
{
    String valString;
    
    if (val == cvf::UNDEFINED_FLOAT) valString = "Undefined";
    else                             valString = String::number(val);

    setAttributeString(attributeName, valString);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlElementImpl::setAttributeDouble(const String& attributeName, double val)
{
    String valString;

    if (val == cvf::UNDEFINED_DOUBLE) valString = "Undefined";
    else                              valString = String::number(val);

    setAttributeString(attributeName, valString);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlElementImpl::setAttributeVector(const String& attributeName, const cvf::Vec3d& val)
{
    String valString =  String::number(val.x()) + " " + String::number(val.y()) + " " + String::number(val.z());

    setAttributeString(attributeName, valString);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlElementImpl::setAttributeColor(const String& attributeName, const cvf::Color3f& color)
{
    String valString =  String::number(color.r()) + " " + String::number(color.g()) + " " + String::number(color.b());

    setAttributeString(attributeName, valString);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String XmlElementImpl::getAttributeString(const String& attributeName, const String& defaultVal /*= ""*/, bool* found /*= NULL*/) const
{
    if (found) *found = false;

    const char* attrValue = Attribute(attributeName.toUtf8().ptr());
    if (!attrValue)
    {
        return defaultVal;
    }

    String val = String::fromUtf8(attrValue);

    if (found) *found = true;

    return val;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int XmlElementImpl::getAttributeInt(const String& attributeName, int defaultVal, bool* found /*= NULL*/) const
{
    int intValue = defaultVal;
    if (QueryIntAttribute(attributeName.toUtf8().ptr(), &intValue) == TIXML_SUCCESS)
    {
        if (found) *found = true;

        return intValue;
    }
    else
    {
        if (found) *found = false;

        return defaultVal;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::int64 XmlElementImpl::getAttributeInt64(const String& attributeName, cvf::int64 defaultVal, bool* found /*= NULL*/) const
{
    cvf::int64 int64Value = defaultVal;
    if (QueryValueAttribute<cvf::int64>(attributeName.toUtf8().ptr(), &int64Value) == TIXML_SUCCESS)
    {
        if (found) *found = true;

        return int64Value;
    }
    else
    {
        if (found) *found = false;

        return defaultVal;
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
XmlElement* XmlElementImpl::addChildElement(const String& elementName, const cvf::String& stringValue)
{
    XmlElementImpl* elem = new XmlElementImpl(elementName);
    if (!elem) return NULL;

    if (!stringValue.isEmpty())
    {
        elem->setValueText(stringValue);
    }

    return (XmlElementImpl*)LinkEndChild(elem);
}

//--------------------------------------------------------------------------------------------------
/// See TiXmlElement::CopyTo()
//--------------------------------------------------------------------------------------------------
void XmlElementImpl::cloneElementChild(const TiXmlElement* source)
{
    XmlElementImpl* innerElem = new XmlElementImpl(source->Value());

    const TiXmlAttribute* attribute = 0;
    for(attribute = source->FirstAttribute(); attribute; attribute = attribute->Next() )
    {
        innerElem->SetAttribute( attribute->Name(), attribute->Value() );
    }

    // Copy all text elements
    const TiXmlNode* node = 0;
    for(node = source->FirstChild(); node; node = node->NextSibling())
    {
        const TiXmlText* text = node->ToText();

        if (text)
        {
            innerElem->InsertEndChild(*text);
        }
    }

    XmlElementImpl* child = (XmlElementImpl*)LinkEndChild(innerElem);

    const TiXmlElement* sourceChild = source->FirstChildElement();
    while (sourceChild != NULL)
    {
        child->cloneElementChild(sourceChild);
        sourceChild = sourceChild->NextSiblingElement();
    }
}



//==================================================================================================
///
/// \class cvf::XmlDocumentImpl
/// \ingroup Io
///
/// 
///
//==================================================================================================
class XmlDocumentImpl : public XmlDocument, public TiXmlDocument
{
public:
    XmlDocumentImpl();

    void setDeclaration(const String& version = "1.0", const String& encoding = "UTF-8", const String& standalone = "yes");

    void        clear();
    bool        error() const;

    XmlElement* createRootElement(const String& rootName, int iID = -1, const String& namespaceString = "http://ceetron.com");
    XmlElement* getRootElement(const String& rootName);
    const XmlElement* getRootElement(const String& rootName) const;

    bool        loadFile(const String& fileName);
    bool        saveFile(const String& fileName);
    bool        saveCompactFile(const cvf::String& fileName);

    bool        setFromRawData(const cvf::UByteArray& buffer);
    void        getAsRawData(cvf::UByteArray* buffer) const;

    bool        setFromTiXmlDoc(const TiXmlDocument& doc);
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
XmlDocumentImpl::XmlDocumentImpl() : TiXmlDocument()
{
    setDeclaration();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlDocumentImpl::setDeclaration(const String& version, const String& encoding, const String& standalone)
{
    TiXmlDeclaration* pDecl = NULL;

    // Must use FirstChild() as RootElement() cast to TiXmlElement, which is a different type to TiXmlDeclaration
    if (FirstChild())
    {
        pDecl = FirstChild()->ToDeclaration();
    }

    if (pDecl)
    {
        // Update the declaration
        TiXmlDeclaration newDecl(version.toUtf8().ptr(), encoding.toUtf8().ptr(), standalone.toUtf8().ptr());
        *pDecl = newDecl;
    }
    else
    {
        // Create a new declaration
        pDecl = new TiXmlDeclaration(version.toUtf8().ptr(), encoding.toUtf8().ptr(), standalone.toUtf8().ptr());
        LinkEndChild(pDecl);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlDocumentImpl::clear()
{
    Clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool XmlDocumentImpl::error() const
{
    return Error();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
XmlElement* XmlDocumentImpl::createRootElement(const String& rootName, int iID /*= -1*/, const String& namespaceString /*= "http://ceetron.com")*/)
{
    XmlElementImpl* pRoot = new XmlElementImpl(rootName);
    if (!pRoot) return NULL;

    LinkEndChild(pRoot);

    if (!namespaceString.isEmpty()) pRoot->setAttributeString("xmlns", namespaceString);
    if (iID >= 0) pRoot->setAttributeInt("ID", iID);

    return pRoot;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
XmlElement* XmlDocumentImpl::getRootElement(const String& rootName)
{
    TiXmlElement* root = RootElement();
    if (!root) return NULL;

    if (!rootName.isEmpty())
    {
        if (rootName != root->Value()) return NULL;
    }

    XmlElement* xmlElt = (XmlElementImpl*)root;

    return xmlElt;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const XmlElement* XmlDocumentImpl::getRootElement(const String& rootName) const
{
    const TiXmlElement* root = RootElement();
    if (!root) return NULL;

    if (!rootName.isEmpty())
    {
        if (rootName != root->Value()) return NULL;
    }

    return (const XmlElementImpl*)root;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool XmlDocumentImpl::loadFile(const String& fileName)
{
#ifdef WIN32
    FILE* filePtr = NULL;
    if (_wfopen_s(&filePtr, fileName.c_str(), L"rb") != 0)
    {
        return false;
    }
#else
    FILE* filePtr = fopen(fileName.toUtf8().ptr(), "rb");
    if (!filePtr) 
    {
        return false;
    }
#endif

    TiXmlDocument doc;

    bool loadOk = doc.LoadFile(filePtr);
    fclose(filePtr);
    filePtr = NULL;

    if (!loadOk)
    {
        return false;
    }

    return setFromTiXmlDoc(doc);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool XmlDocumentImpl::setFromTiXmlDoc(const TiXmlDocument& doc)
{
    const TiXmlElement* tiRoot = doc.RootElement();
    if (!tiRoot) return false;

    // Create a root of XmlElementImpl type, and copy name from other root
    XmlElementImpl* myRoot = (XmlElementImpl*)createRootElement(tiRoot->ValueStr().c_str());

    // Copy all root attributes
    const TiXmlAttribute* attribute = 0;
    for(attribute = tiRoot->FirstAttribute(); attribute; attribute = attribute->Next() )
    {
        myRoot->SetAttribute( attribute->Name(), attribute->Value() );
    }

    // Copy all text elements
    const TiXmlNode* node = 0;
    for(node = tiRoot->FirstChild(); node; node = node->NextSibling())
    {
        const TiXmlText* text = node->ToText();

        if (text)
        {
            myRoot->InsertEndChild(*text);
        }
    }

    // Clone all elements below root recursively
    const TiXmlElement* sourceChild = tiRoot->FirstChildElement();
    while (sourceChild)
    {
        myRoot->cloneElementChild(sourceChild);
        sourceChild = sourceChild->NextSiblingElement();
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool XmlDocumentImpl::saveFile(const String& fileName)
{
#ifdef WIN32
    FILE* filePtr = NULL;
    if (_wfopen_s(&filePtr, fileName.c_str(), L"w") != 0)
    {
        return false;
    }
#else
    FILE* filePtr = fopen(fileName.toUtf8().ptr(), "w");
    if (!filePtr) 
    {
        return false;
    }
#endif

    bool saveOk = SaveFile(filePtr);
    fclose(filePtr);
    filePtr = NULL;

    return saveOk;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool XmlDocumentImpl::saveCompactFile(const String& fileName)
{
    std::ofstream myfile;

#ifdef WIN32
    myfile.open(fileName.c_str());
#else
    myfile.open(fileName.toUtf8().ptr());
#endif

    if (!myfile.is_open())
    {
        return false;
    }
    
    myfile << *this;
    
    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool XmlDocumentImpl::setFromRawData(const cvf::UByteArray& buffer)
{
    if (buffer.size() == 0)
    {
        // empty, nothing read
        clear();
        return true;
    }

    TiXmlDocument doc;

    size_t uiNumBytes = buffer.size();

    if (buffer[uiNumBytes - 1] == '\0')
    {
        // HACK: Dette gikk ikke, hvorfor?
        //Parse(static_cast<const char*>(&buffer[0]));
        doc.Parse((const char*)(&buffer[0]));
    }
    else
    {
        UByteArray temp = buffer;

        // Parse() requires NULL terminated data, but ReadDataRaw does not guarantee that, so we need to tweak a bit
        if (temp[uiNumBytes - 1] != '>')
        {
            // last byte is not '>' terminating an Xml element, so just set it to '\0'
            temp[uiNumBytes - 1] = '\0';
        }
        else
        {
            temp.resize(uiNumBytes + 1);
            temp[uiNumBytes] = '\0';
        }

        // Create a Xml document from raw data
        doc.Parse((const char*)(&temp[0]));
    }

    if (doc.Error())
    {	
        return false;
    }

    return setFromTiXmlDoc(doc);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlDocumentImpl::getAsRawData(cvf::UByteArray* buffer) const
{
	TiXmlPrinter printer;

	// Use tab as indent marker
	printer.SetIndent( "\t" );

	// Link document to printer
	Accept(&printer);

    buffer->assign((cvf::ubyte*)printer.CStr(), (size_t)printer.Size());
}



//==================================================================================================
//
// Static factory methods
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Factory method
//--------------------------------------------------------------------------------------------------
ref<XmlDocument> XmlDocument::create()
{
    ref<XmlDocument> doc = new XmlDocumentImpl();
    doc->preserveWhiteSpace(true);

    return doc;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void XmlDocument::preserveWhiteSpace(bool preserve)
{
    TiXmlBase::SetCondenseWhiteSpace(!preserve);
}

} // namespace cvfu

