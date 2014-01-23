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
#include "cvfRegGrid2DImportXml.h"
#include "cvfRegGrid2DExportXml.h"
#include "cvfRegGrid2D.h"
#include "cvfString.h"

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
TEST(RegGrid2DExportTest, Basic)
{
    ref<RegGrid2D> regGridA = new RegGrid2D;
    ASSERT_TRUE(regGridA.notNull());

    Vec2d offset(10, 20);
    int gridCountI = 2;
    int gridCountJ = 3;
    Vec2d spacing(10, 200);

    regGridA->setOffset(offset);
    regGridA->setSpacing(spacing);
    regGridA->allocateGrid(gridCountI, gridCountJ);

    DoubleArray values;
    values.resize(2 * 3);
    values.set(0, 2);
    values.set(1, 5);
    values.set(2, 4);
    values.set(3, -2);
    values.set(4, -2);
    values.set(5, -2);
    regGridA->setElevations(values);

    String filename = "reggrid.xml";
    RegGrid2DExportXml::exportToXmlFile(*regGridA, 10, filename);

    ref<RegGrid2D> regGridB = new RegGrid2D;
    ASSERT_TRUE(regGridB.notNull());

    RegGrid2DImportXml::importFromXmlFile(filename, regGridB.p());
    EXPECT_EQ(regGridA->offset().x(), regGridB->offset().x());
    EXPECT_EQ(regGridA->offset().y(), regGridB->offset().y());
    EXPECT_EQ(regGridA->spacing().x(), regGridB->spacing().x());
    EXPECT_EQ(regGridA->spacing().y(), regGridB->spacing().y());

    EXPECT_EQ(regGridA->gridPointCountI(), regGridB->gridPointCountI());
    EXPECT_EQ(regGridA->gridPointCountJ(), regGridB->gridPointCountJ());


    DeleteMyTestFile(filename);
}
