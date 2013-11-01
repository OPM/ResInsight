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

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static bool ArePropertySetCollectionsEqual(const PropertySetCollection& psc1, const PropertySetCollection& psc2)
{
    size_t numPropSets = psc1.count();
    if (numPropSets != psc2.count())
    {
        return false;
    }

    for (size_t i = 0; i < numPropSets; i++)
    {
        const PropertySet* ps1 = psc1.propertySet(i);
        const PropertySet* ps2 = psc2.propertySet(i);
        if (!(*ps1 == *ps2))
        {
            return false;
        }
    }

    return true;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PropertyXmlSerializationTest, SaveLoadSimplePropertySets)
{
    ref<PropertySetCollection> psc0 = new PropertySetCollection;
    
    // Create and save the prop set collection
    {
        {
            ref<PropertySet> ps = new PropertySet("classTypeA");
            ps->setValue("myInt", Variant(-123));
            ps->setValue("myUInt", Variant(456u));
            ps->setValue("myDouble", Variant(1.234));
            ps->setValue("myFloat", Variant(1.567f));
            ps->setValue("myBool1", Variant(true));
            ps->setValue("myBool2", Variant(false));
            ps->setValue("myString", Variant("stringInA"));
            ps->setValue("myVec3d", Variant(Vec3d(1, 2, 3)));
            ps->setValue("myColor3f", Variant(Color3f(0.1f, 0.2f, 0.3f)));

            psc0->addPropertySet(ps.p());
        }

        {
            ref<PropertySet> ps = new PropertySet("classTypeB");
            ps->setValue("myInt", Variant(101));
            ps->setValue("myString", Variant("stringInB"));
            ps->setValue("myString", Variant("string  with  double  spaces"));

            psc0->addPropertySet(ps.p());
        }

        ref<XmlDocument> doc = XmlDocument::create();
        XmlElement* root = doc->createRootElement("myRoot");
        PropertyXmlSerializer::toXml(*psc0, root);
        doc->saveFile("testfile_propertiesSimple.xml");
    }

    // Read from the xml file, create property set and compare
    {
        ref<PropertySetCollection> psc = new PropertySetCollection;
        {
            ref<XmlDocument> doc = XmlDocument::create();
            doc->loadFile("testfile_propertiesSimple.xml");
            
            const cvf::XmlElement* root = doc->getRootElement("");
            ASSERT_TRUE(root != NULL);
            const XmlElement* propertySetCollectionElem = root->firstChildElement("PropertySetCollection");
            ASSERT_TRUE(propertySetCollectionElem != NULL);

            PropertyXmlSerializer::toPropertySetCollection(*propertySetCollectionElem, psc.p());
        }
       
        // For debug/inspection, write out the property set collection
        {
            ref<XmlDocument> doc = XmlDocument::create();
            XmlElement* root = doc->createRootElement("myRoot");
            PropertyXmlSerializer::toXml(*psc, root);
            doc->saveFile("testfile_propertiesSimple_rewrite.xml");
        }

        ASSERT_TRUE(psc.notNull());
        EXPECT_TRUE(ArePropertySetCollectionsEqual(*psc, *psc0));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PropertyXmlSerializationTest, SaveLoadComplexPropertySet)
{
    ref<PropertySetCollection> psc0 = new PropertySetCollection;

    {
        ref<PropertySet> ps = new PropertySet("myClassType");

        {
            std::vector<Variant> variantArr;
            variantArr.push_back(Variant(123));
            variantArr.push_back(Variant(1.23));
            variantArr.push_back(Variant("a string"));
            ps->setValue("myFlatArray", Variant(variantArr));
        }
        {
            std::vector<Variant> variantArr;
            variantArr.push_back(Variant(123));
            variantArr.push_back(Variant(1.23));
            variantArr.push_back(Variant("a string"));

            {
                std::vector<Variant> variantSubArr;
                variantSubArr.push_back(Variant(101));
                variantSubArr.push_back("another string");
                variantArr.push_back(Variant(variantSubArr));
            }

            ps->setValue("myNestedArray", Variant(variantArr));
        }

        psc0->addPropertySet(ps.p());

        ref<XmlDocument> doc = XmlDocument::create();
        XmlElement* root = doc->createRootElement("myRoot");
        PropertyXmlSerializer::toXml(*psc0, root);
        doc->saveFile("testfile_propertiesComplex.xml");
    }

    // Read from the xml file, create property set and compare
    {
        ref<PropertySetCollection> psc = new PropertySetCollection;
        {
            ref<XmlDocument> doc = XmlDocument::create();
            doc->loadFile("testfile_propertiesComplex.xml");

            const cvf::XmlElement* root = doc->getRootElement("");
            ASSERT_TRUE(root != NULL);
            const XmlElement* propertySetCollectionElem = root->firstChildElement("PropertySetCollection");
            ASSERT_TRUE(propertySetCollectionElem != NULL);

            PropertyXmlSerializer::toPropertySetCollection(*propertySetCollectionElem, psc.p());
        }

        // For debug/inspection, write out the property set collection
        {
            ref<XmlDocument> doc = XmlDocument::create();
            XmlElement* root = doc->createRootElement("myRoot");
            PropertyXmlSerializer::toXml(*psc, root);
            doc->saveFile("testfile_propertiesComplex_rewrite.xml");
        }

        ASSERT_TRUE(psc.notNull());
        EXPECT_TRUE(ArePropertySetCollectionsEqual(*psc, *psc0));
    }
}


