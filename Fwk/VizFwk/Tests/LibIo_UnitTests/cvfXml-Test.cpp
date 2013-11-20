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
#include "cvfXml.h"
#include "cvfMath.h"

#include "gtest/gtest.h"

using namespace cvf;


// Helper to delete test files
static void DeleteMyTestFile(const String& fileName)
{
#ifdef WIN32
    _wremove(fileName.c_str());
#else
    remove(fileName.toUtf8().ptr());
#endif
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, BoolValues)
{
    ref<XmlDocument> doc = XmlDocument::create();
    EXPECT_TRUE(doc.notNull());

    XmlElement* root = doc->createRootElement("TestRoot");
    EXPECT_TRUE(root != NULL);

    XmlElement* child1 = root->addChildElement("FirstChild");
    child1->setAttributeBool("trueBool", true);
    child1->setAttributeBool("falseBool", false);

    XmlElement* child2 = root->addChildElement("SecondChild");
    child2->setAttributeBool("trueBool", true);
    child2->setAttributeBool("falseBool", false);

    String filename = "testBool.xml";
    doc->saveFile(filename);

    ref<XmlDocument> doc2 = XmlDocument::create();
    doc2->loadFile(filename);

    XmlElement* root2 = doc2->getRootElement("");
    EXPECT_TRUE(root2 != NULL);

    XmlElement* elem = root2->firstChildElement();
    String attrName("trueBool");
    bool valueRead = elem->getAttributeBool(attrName, false);
    EXPECT_TRUE(valueRead);

    attrName = "falseBool";
    valueRead = elem->getAttributeBool(attrName, false);
    EXPECT_TRUE(!valueRead);

    bool result = false;
    attrName = "falseBool";
    valueRead = elem->getAttributeBool(attrName, false, &result);
    EXPECT_TRUE(!valueRead);
    EXPECT_TRUE(result);

    attrName = "doesNotExits";
    valueRead = elem->getAttributeBool(attrName, false, &result);
    EXPECT_TRUE(!valueRead);
    EXPECT_TRUE(!result);

    // Read second child
    elem = root->firstChildElement("SecondChild");
    attrName = "falseBool";
    valueRead = elem->getAttributeBool(attrName, false, &result);
    EXPECT_TRUE(!valueRead);
    EXPECT_TRUE(result);

    attrName = "";
    valueRead = elem->getAttributeBool(attrName, false, &result);
    EXPECT_TRUE(!valueRead);
    EXPECT_TRUE(!result);

    DeleteMyTestFile(filename);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, IntValues)
{
    ref<XmlDocument> doc = XmlDocument::create();
    EXPECT_TRUE(doc.notNull());

    XmlElement* root = doc->createRootElement("TestRoot");
    EXPECT_TRUE(root != NULL);

    {
        int val = 0;
        XmlElement* child1 = root->addChildElement("IntValues");

        val = std::numeric_limits<int>::max();
        child1->setAttributeInt("max", val);

        val = std::numeric_limits<int>::min();
        child1->setAttributeInt("min", val);
    }

    {
        // Int 64
        int64 val64 = 0;
        XmlElement* child2 = root->addChildElement("Int64Values");

        val64 = std::numeric_limits<int64>::max();
        child2->setAttributeInt64("max", val64);

        val64 = std::numeric_limits<int64>::min();
        child2->setAttributeInt64("min", val64);
    }


    String filename = "testInt.xml";
    doc->saveFile(filename);

    ref<XmlDocument> doc2 = XmlDocument::create();
    doc2->loadFile(filename);

    XmlElement* root2 = doc2->getRootElement("");
    EXPECT_TRUE(root2 != NULL);

    XmlElement* elem = root2->firstChildElement();
    String attrName("max");
    int valueRead = elem->getAttributeInt(attrName, false);
    EXPECT_EQ(valueRead, std::numeric_limits<int>::max());

    attrName = "min";
    valueRead = elem->getAttributeInt(attrName, false);
    EXPECT_EQ(valueRead, std::numeric_limits<int>::min());

    elem = elem->nextSiblingElement();
    attrName = "max";
    int64 value64Read = elem->getAttributeInt64(attrName, false);
    EXPECT_EQ(value64Read, std::numeric_limits<int64>::max());

    attrName = "min";
    value64Read = elem->getAttributeInt64(attrName, false);
    EXPECT_EQ(value64Read, std::numeric_limits<int64>::min());

    DeleteMyTestFile(filename);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, FloatValues)
{
    ref<XmlDocument> doc = XmlDocument::create();
    EXPECT_TRUE(doc.notNull());

    XmlElement* root = doc->createRootElement("TestRoot");
    EXPECT_TRUE(root != NULL);

    {
        float val = 0;
        XmlElement* child1 = root->addChildElement("FloatValues");

        val = std::numeric_limits<float>::max();
        child1->setAttributeFloat("max", val);

        val = std::numeric_limits<float>::min();
        child1->setAttributeFloat("min", val);
    }


    String filename = "testFloat.xml";
    doc->saveFile(filename);

    ref<XmlDocument> doc2 = XmlDocument::create();
    doc2->loadFile(filename);

    XmlElement* root2 = doc2->getRootElement("");
    EXPECT_TRUE(root2 != NULL);

    XmlElement* elem = root2->firstChildElement();
    String attrName("max");
    float valueRead = elem->getAttributeFloat(attrName, false);
    EXPECT_FLOAT_EQ(valueRead, UNDEFINED_FLOAT);

    attrName = "min";
    valueRead = elem->getAttributeFloat(attrName, false);
    EXPECT_NEAR(valueRead, std::numeric_limits<float>::min(), 1.0e-6);

    elem = elem->nextSiblingElement();

    DeleteMyTestFile(filename);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, DoubleValues)
{
    ref<XmlDocument> doc = XmlDocument::create();
    EXPECT_TRUE(doc.notNull());

    XmlElement* root = doc->createRootElement("TestRoot");
    EXPECT_TRUE(root != NULL);

    {
        double val = 0;
        XmlElement* child1 = root->addChildElement("DoubleValues");

        val = std::numeric_limits<double>::max();
        child1->setAttributeDouble("max", val);

        val = std::numeric_limits<double>::min();
        child1->setAttributeDouble("min", val);

        val = 0.0;
        child1->setAttributeDouble("zero", val);
    }


    String filename = "testDouble.xml";
    doc->saveFile(filename);

    ref<XmlDocument> doc2 = XmlDocument::create();
    doc2->loadFile(filename);

    XmlElement* root2 = doc2->getRootElement("");
    EXPECT_TRUE(root2 != NULL);

    XmlElement* elem = root2->firstChildElement();
    String attrName("max");
    double valueRead = elem->getAttributeDouble(attrName, false);
    EXPECT_DOUBLE_EQ(valueRead, UNDEFINED_DOUBLE);

    attrName = "min";
    valueRead = elem->getAttributeDouble(attrName, false);
    EXPECT_NEAR(valueRead, std::numeric_limits<double>::min(), 1.0e-6);

    attrName = "zero";
    valueRead = elem->getAttributeDouble(attrName, false);
    EXPECT_EQ(valueRead, 0.0);

    attrName = "doesNotExist";
    bool found = false;
    valueRead = elem->getAttributeDouble(attrName, 10.1, &found);
    EXPECT_EQ(valueRead, 10.1);
    EXPECT_TRUE(!found);

    DeleteMyTestFile(filename);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, ColorValues)
{
    ref<XmlDocument> doc = XmlDocument::create();
    EXPECT_TRUE(doc.notNull());

    XmlElement* root = doc->createRootElement("TestRoot");
    EXPECT_TRUE(root != NULL);

    {
        Color3f col(Color3f::BLACK);
        XmlElement* child1 = root->addChildElement("ColorValues");
        child1->setAttributeColor("black", col);

        col = Color3f::WHITE;
        child1->setAttributeColor("white", col);

        col = Color3f(0.0f, 0.4f, 1.0f);
        child1->setAttributeColor("bluish", col);
    }


    String filename = "testColor.xml";
    doc->saveFile(filename);

    ref<XmlDocument> doc2 = XmlDocument::create();
    doc2->loadFile(filename);

    XmlElement* root2 = doc2->getRootElement("");
    EXPECT_TRUE(root2 != NULL);

    Color3f defaultColor = Color3f::PURPLE;

    XmlElement* elem = root2->firstChildElement();
    String attrName("black");
    Color3f valueRead = elem->getAttributeColor(attrName, defaultColor);
    EXPECT_TRUE(valueRead == Color3f::BLACK);

    attrName = "white";
    valueRead = elem->getAttributeColor(attrName, defaultColor);
    EXPECT_TRUE(valueRead == Color3f::WHITE);

    attrName = "bluish";
    valueRead = elem->getAttributeColor(attrName, defaultColor);
    EXPECT_TRUE(valueRead == Color3f(0.0f, 0.4f, 1.0f));

    attrName = "doesNotExist";
    bool found = false;
    valueRead = elem->getAttributeColor(attrName, defaultColor, &found);
    EXPECT_TRUE(valueRead == Color3f::PURPLE);
    EXPECT_TRUE(!found);

    DeleteMyTestFile(filename);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, VectorValues)
{
    ref<XmlDocument> doc = XmlDocument::create();
    EXPECT_TRUE(doc.notNull());

    XmlElement* root = doc->createRootElement("TestRoot");
    EXPECT_TRUE(root != NULL);

    {
        Vec3d vec(0,1,2);
        XmlElement* child1 = root->addChildElement("VectorValues");
        child1->setAttributeVector("first", vec);

        vec.set(3,4,5);
        child1->setAttributeVector("second", vec);

        vec.set(-1,2,-3);
        child1->setAttributeVector("Neg", vec);
    }

    String filename = "testVector.xml";
    ASSERT_TRUE(doc->saveFile(filename));

    ref<XmlDocument> doc2 = XmlDocument::create();
    ASSERT_TRUE(doc2->loadFile(filename));

    XmlElement* root2 = doc2->getRootElement("");
    ASSERT_TRUE(root2 != NULL);

    Vec3d defaultVal = Vec3d(999,999,999);

    XmlElement* elem = root2->firstChildElement();
    String attrName("first");
    Vec3d valueRead = elem->getAttributeVector(attrName, defaultVal);
    EXPECT_TRUE(valueRead == Vec3d(0,1,2));

    attrName = "second";
    valueRead = elem->getAttributeVector(attrName, defaultVal);
    EXPECT_TRUE(valueRead == Vec3d(3,4,5));

    attrName = "Neg";
    valueRead = elem->getAttributeVector(attrName, defaultVal);
    EXPECT_TRUE(valueRead == Vec3d(-1,2,-3));

    attrName = "doesNotExist";
    bool found = false;
    valueRead = elem->getAttributeVector(attrName, defaultVal, &found);
    EXPECT_TRUE(valueRead == Vec3d(999,999,999));
    EXPECT_TRUE(!found);

    DeleteMyTestFile(filename);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, TestHierarchy)
{
    ref<XmlDocument> doc = XmlDocument::create();
    EXPECT_TRUE(doc.notNull());

    XmlElement* root = doc->createRootElement("TestRoot");
    EXPECT_TRUE(root != NULL);

    XmlElement* child1 = root->addChildElement("Level1");
    XmlElement* child2 = child1->addChildElement("Level2");
    XmlElement* child3 = child2->addChildElement("Level3");
    XmlElement* child4 = child3->addChildElement("Level4");
    XmlElement* child5 = child4->addChildElement("Level5");
    XmlElement* child6 = child5->addChildElement("Level6");
    XmlElement* child7 = child6->addChildElement("Level7");
    child7->setAttributeString("test", "dette er en test");

    String filename = "testHierarchy.xml";
    doc->saveFile(filename);

    ref<XmlDocument> doc2 = XmlDocument::create();
    doc2->loadFile(filename);

    XmlElement* root2 = doc2->getRootElement("");
    EXPECT_TRUE(root2 != NULL);

    XmlElement* elem = root2->firstChildElement();
    EXPECT_STREQ("Level1", elem->name().toAscii().ptr());
    elem = elem->firstChildElement();
    EXPECT_STREQ("Level2", elem->name().toAscii().ptr());
    elem = elem->firstChildElement();
    EXPECT_STREQ("Level3", elem->name().toAscii().ptr());
    elem = elem->firstChildElement();
    EXPECT_STREQ("Level4", elem->name().toAscii().ptr());
    elem = elem->firstChildElement();
    EXPECT_STREQ("Level5", elem->name().toAscii().ptr());
    elem = elem->firstChildElement();
    EXPECT_STREQ("Level6", elem->name().toAscii().ptr());
    elem = elem->firstChildElement();
    EXPECT_STREQ("Level7", elem->name().toAscii().ptr());
    String valueRead = elem->getAttributeString("test");
    EXPECT_STREQ("dette er en test", valueRead.toAscii().ptr());

    elem = elem->firstChildElement();
    EXPECT_TRUE(!elem);

    DeleteMyTestFile(filename);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, SaveToNonASCIIFileName)
{
    // זרו.xml
   const wchar_t* uniStr = L"\x00e6\x00f8\x00e5.xml";
    String filename(uniStr);
    {
        ref<XmlDocument> doc = XmlDocument::create();
        EXPECT_TRUE(doc.notNull());

        XmlElement* root = doc->createRootElement("TestRoot");
        EXPECT_TRUE(root != NULL);

        root->addChildElement("Child1");
        root->addChildElement("Child2");

        ASSERT_TRUE(doc->saveFile(filename));
    }

    {
        ref<XmlDocument> doc = XmlDocument::create();
        ASSERT_TRUE(doc->loadFile(filename));
    }

    DeleteMyTestFile(filename);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, NonASCIICharacters)
{
    const String filename = "testNonAsciiContents.xml";

    const String uniStr1(L"abc");

    // זרו
    const String uniStr2(L"\x00e6\x00f8\x00e5");

    // Greek small, alfa, beta, gamma, delta and epsilon
    const String uniStr3(L"\x03B1\x03B2\x03B3\x03B4\x03B5");

    {
        ref<XmlDocument> doc = XmlDocument::create();
        EXPECT_TRUE(doc.notNull());

        XmlElement* root = doc->createRootElement("TestRoot");
        EXPECT_TRUE(root != NULL);

        XmlElement* child1 = root->addChildElement("Child1");
        child1->setAttributeString("str1", uniStr1);
        child1->setAttributeString("str2", uniStr2);
        child1->setAttributeString("str3", uniStr3);


        XmlElement* child2 = root->addChildElement("Child2");
        child2->setAttributeString("str3", uniStr3);
        child2->setAttributeString("str2", uniStr2);
        child2->setAttributeString("str1", uniStr1);

        ASSERT_TRUE(doc->saveFile(filename));
    }

    {
        ref<XmlDocument> doc = XmlDocument::create();
        ASSERT_TRUE(doc->loadFile(filename));

        XmlElement* root = doc->getRootElement("TestRoot");
        EXPECT_TRUE(root != NULL);

        XmlElement* child1 = root->firstChildElement();
        ASSERT_TRUE(child1 != NULL);
        EXPECT_STREQ("Child1", child1->name().toAscii().ptr());


        String str;
        str = child1->getAttributeString("str1");
        EXPECT_TRUE(uniStr1 == str);
        
        str = child1->getAttributeString("str2");
        EXPECT_TRUE(uniStr2 == str);

        str = child1->getAttributeString("str3");
        EXPECT_TRUE(uniStr3 == str);


        XmlElement* child2 = child1->nextSiblingElement();
        ASSERT_TRUE(child2 != NULL);
        EXPECT_STREQ("Child2", child2->name().toAscii().ptr());

        str = child2->getAttributeString("str1");
        EXPECT_TRUE(uniStr1 == str);

        str = child2->getAttributeString("str2");
        EXPECT_TRUE(uniStr2 == str);

        str = child2->getAttributeString("str3");
        EXPECT_TRUE(uniStr3 == str);
    }

    DeleteMyTestFile(filename);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, WhiteSpace)
{
    String filename = "testWhiteSpace.xml";
    {
        ref<XmlDocument> doc = XmlDocument::create();
        doc->preserveWhiteSpace(true);

        XmlElement* root = doc->createRootElement("TestRoot");
        EXPECT_TRUE(root != NULL);

        XmlElement* child1 = root->addChildElement("FirstChild");
        child1->setValueText("  space inside text  ");

        XmlElement* child2 = root->addChildElement("SecondChild");
        child2->setValueText(" ");

        XmlElement* child3 = root->addChildElement("ThirdChild");
        child3->setValueText("    ");

        doc->saveFile(filename);
    }

    // Skip whitespace when loading XML file
    {
        ref<XmlDocument> doc = XmlDocument::create();
        doc->preserveWhiteSpace(false);
        doc->loadFile(filename);

        XmlElement* root = doc->getRootElement("TestRoot");
        EXPECT_TRUE(root != NULL);

        XmlElement* child1 = root->firstChildElement();
        ASSERT_TRUE(child1 != NULL);

        String str;
        str = child1->valueText();
        EXPECT_STREQ("space inside text", str.toAscii().ptr());

        XmlElement* child2 = child1->nextSiblingElement();
        ASSERT_TRUE(child2 != NULL);

        str = child2->valueText();
        EXPECT_TRUE(str.isEmpty());

        XmlElement* child3 = child2->nextSiblingElement();
        ASSERT_TRUE(child3 != NULL);

        str = child3->valueText();
        EXPECT_TRUE(str.isEmpty());
    }

    // Preserve whitespace when loading XML file, default behaviour
    {
        ref<XmlDocument> doc = XmlDocument::create();
        doc->loadFile(filename);

        XmlElement* root = doc->getRootElement("TestRoot");
        EXPECT_TRUE(root != NULL);

        XmlElement* child1 = root->firstChildElement();
        ASSERT_TRUE(child1 != NULL);

        String str;
        str = child1->valueText();
        EXPECT_STREQ("  space inside text  ", str.toAscii().ptr());

        XmlElement* child2 = child1->nextSiblingElement();
        ASSERT_TRUE(child2 != NULL);

        str = child2->valueText();
        EXPECT_STREQ(" ", str.toAscii().ptr());

        XmlElement* child3 = child2->nextSiblingElement();
        ASSERT_TRUE(child3 != NULL);

        str = child3->valueText();
        EXPECT_STREQ("    ", str.toAscii().ptr());

        String filenameOut = "testWhiteSpacePreserved.xml";
        doc->saveFile(filenameOut);
        DeleteMyTestFile(filenameOut);
    }

    // Compact XML file output
    {
        ref<XmlDocument> doc = XmlDocument::create();
        doc->loadFile(filename);

        String filenameOut = "testWhiteSpaceCompact.xml";
        doc->saveCompactFile(filenameOut);

        DeleteMyTestFile(filenameOut);
    }


    DeleteMyTestFile(filename);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, RootWithAttributes)
{
    String filename = "testWhiteSpace.xml";

    {
        ref<XmlDocument> doc = XmlDocument::create();

        XmlElement* root = doc->createRootElement("TestRoot");
        EXPECT_TRUE(root != NULL);

        root->setValueText("Value text");
        root->setAttributeString("AttribTest", "This is a test");
        doc->saveFile(filename);
    }

    {
        ref<XmlDocument> doc = XmlDocument::create();
        doc->loadFile(filename);

        XmlElement* root = doc->getRootElement("TestRoot");
        EXPECT_TRUE(root != NULL);

        cvf::String str = root->getAttributeString("AttribTest");
        EXPECT_STREQ("This is a test", str.toAscii().ptr());

        cvf::String valText = root->valueText();
        EXPECT_STREQ("Value text", valText.toAscii().ptr());
    }

    DeleteMyTestFile(filename);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, GetAttributes)
{
    ref<XmlDocument> doc = XmlDocument::create();
    EXPECT_TRUE(doc.notNull());

    XmlElement* root = doc->createRootElement("TestRoot");
    EXPECT_TRUE(root != NULL);

    XmlElement* child1 = root->addChildElement("FirstChild");
    child1->setAttributeBool("trueBool", true);
    child1->setAttributeBool("falseBool", false);

    std::vector<cvf::String> names, values;
    child1->getAttributes(&names, &values);

    EXPECT_EQ(2, names.size());
    EXPECT_EQ(2, values.size());

    EXPECT_STREQ("trueBool", names[0].toAscii().ptr());
    EXPECT_STREQ("falseBool", names[1].toAscii().ptr());

    EXPECT_STREQ("true", values[0].toAscii().ptr());
    EXPECT_STREQ("false", values[1].toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, RemoveChildElement)
{
    ref<XmlDocument> doc = XmlDocument::create();
    EXPECT_TRUE(doc.notNull());

    XmlElement* root = doc->createRootElement("TestRoot");
    EXPECT_TRUE(root != NULL);

    XmlElement* child1 = root->addChildElement("FirstChild");
    XmlElement* child2 = root->addChildElement("SecondChild");
    XmlElement* child3 = root->addChildElement("ThirdChild");

    root->removeChildElement(child2);

    EXPECT_TRUE(child1 == root->firstChildElement());
    EXPECT_TRUE(child3 == root->firstChildElement()->nextSiblingElement());

    root->removeChildElement(child3);

    EXPECT_TRUE(child1 == root->firstChildElement());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, Clear)
{
    ref<XmlDocument> doc = XmlDocument::create();
    EXPECT_TRUE(doc.notNull());

    XmlElement* root = doc->createRootElement("TestRoot");
    EXPECT_TRUE(root != NULL);

    root->addChildElement("FirstChild");
    root->addChildElement("SecondChild");
    root->addChildElement("ThirdChild");

    doc->clear();

    EXPECT_TRUE(NULL == doc->getRootElement("TestRoot"));
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(XmlTest, ToOrFromRawData)
{
    ref<XmlDocument> doc = XmlDocument::create();
    EXPECT_TRUE(doc.notNull());

    XmlElement* root = doc->createRootElement("TestRoot");
    EXPECT_TRUE(root != NULL);

    root->addChildElement("FirstChild");
    root->addChildElement("SecondChild");
    root->addChildElement("ThirdChild");

    cvf::UByteArray buffer;
    doc->getAsRawData(&buffer);

    ref<XmlDocument> doc2 = XmlDocument::create();
    EXPECT_TRUE(doc2.notNull());

    doc2->setFromRawData(buffer);
    XmlElement* root2 = doc2->getRootElement("TestRoot");
    ASSERT_TRUE(root2 != NULL);

    ASSERT_TRUE(root2->firstChildElement() != NULL);
    ASSERT_TRUE(root2->firstChildElement()->nextSiblingElement() != NULL);
    ASSERT_TRUE(root2->firstChildElement()->nextSiblingElement()->nextSiblingElement() != NULL);

    EXPECT_STREQ("FirstChild",  root2->firstChildElement()->name().toAscii().ptr());
    EXPECT_STREQ("SecondChild", root2->firstChildElement()->nextSiblingElement()->name().toAscii().ptr());
    EXPECT_STREQ("ThirdChild",  root2->firstChildElement()->nextSiblingElement()->nextSiblingElement()->name().toAscii().ptr());
}


