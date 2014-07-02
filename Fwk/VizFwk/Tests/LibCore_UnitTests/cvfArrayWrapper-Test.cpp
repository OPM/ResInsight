//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
#include "cvfArray.h"
#include "cvfArrayWrapperToEdit.h"
#include "cvfArrayWrapperConst.h"

#include "gtest/gtest.h"
#include <iostream>
#include <algorithm>

using namespace cvf;



template <typename ArrayType, typename ElmType>
void arrayWrapperConstTestFunction(const ArrayWrapperConst< ArrayType, ElmType> cinRefArray)
{
    ElmType e;
    size_t size;

    size = cinRefArray.size();
    e = cinRefArray[size-1];
    CVF_UNUSED(e);
    // cinRefArray[size-1] = e;
    {
        const ElmType& cre = cinRefArray[size-1];
        CVF_UNUSED(cre);

        //ElmType& re = cinRefArray[size-1];
        //re = e;
    }
}

template <typename ArrayType, typename ElmType>
void arrayWrapperConstRefTestFunction(const ArrayWrapperConst< ArrayType, ElmType>& cinRefArray)
{
    ElmType e;
    size_t size;

    size = cinRefArray.size();
    e = cinRefArray[size-1];
    CVF_UNUSED(e);
    // cinRefArray[size-1] = e;
    {
        const ElmType& cre = cinRefArray[size-1];
         CVF_UNUSED(cre);
        //ElmType& re = cinRefArray[size-1];
        //re = e;
    }
}


template <typename ArrayType, typename ElmType>
void arrayWrapperTestFunction(ArrayWrapperToEdit< ArrayType, ElmType> cinRefArray)
{
    ElmType e, e2;
    size_t size;

    size = cinRefArray.size();
    e = cinRefArray[size-1];
    e2 = cinRefArray[0];
    cinRefArray[0] = e;
    {
        const ElmType& cre = cinRefArray[size-1];
         CVF_UNUSED(cre);
        ElmType& re = cinRefArray[size-1];
        re = e2;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayWrapperTest, AllSpecializations)
{
    std::vector<cvf::Vec3d> vec3dStdVector;
    vec3dStdVector.push_back(Vec3d::ZERO);
    vec3dStdVector.push_back(Vec3d(1,1,1));

    const std::vector<cvf::Vec3d> &cvec3dStdVector = vec3dStdVector;

    cvf::Vec3dArray vec3dCvfArray(vec3dStdVector);
    const cvf::Vec3dArray& cvec3dCvfArray = vec3dCvfArray;

    cvf::Array<size_t> siztCvfArray(2);
    siztCvfArray[0] = 0;
    siztCvfArray[1] = 1;

    cvf::Array<cvf::uint> uintCvfArray(2);
    uintCvfArray[0] = 0;
    uintCvfArray[1] = 1;
    const cvf::Array<cvf::uint>& cuintCvfArray = uintCvfArray;

    size_t siztBarePtrArray[2] = {0, 1};

    size_t* siztBarePtr = new size_t[2];
    siztBarePtr[0] = 0;
    siztBarePtr[1] = 1;

    const size_t* csiztBarePtr = siztBarePtr;

    cvf::uint* uintBarePtr = new cvf::uint[2];
    uintBarePtr[0] = 0;
    uintBarePtr[1] = 1;

    double*  doubleBarePtr = new double[2];
    doubleBarePtr[0] = 0;
    doubleBarePtr[1] = 1;
    const double* cdoubleBarePtr = doubleBarePtr;

    arrayWrapperConstTestFunction(wrapArrayConst(&vec3dStdVector));
    arrayWrapperConstTestFunction(wrapArrayConst(&cvec3dStdVector));
    arrayWrapperConstTestFunction(wrapArrayConst(&vec3dCvfArray));
    arrayWrapperConstTestFunction(wrapArrayConst(&cvec3dCvfArray));
    arrayWrapperConstTestFunction(wrapArrayConst(&uintCvfArray));
    arrayWrapperConstTestFunction(wrapArrayConst(&cuintCvfArray));
    arrayWrapperConstTestFunction(wrapArrayConst(siztBarePtrArray, 2));
    arrayWrapperConstTestFunction(wrapArrayConst(siztBarePtr, 2));
    arrayWrapperConstTestFunction(wrapArrayConst(csiztBarePtr, 2));
    arrayWrapperConstTestFunction(wrapArrayConst(doubleBarePtr,2));
    arrayWrapperConstTestFunction(wrapArrayConst(cdoubleBarePtr, 2));

    arrayWrapperConstRefTestFunction(wrapArrayConst(&vec3dStdVector));
    arrayWrapperConstRefTestFunction(wrapArrayConst(&cvec3dStdVector));
    arrayWrapperConstRefTestFunction(wrapArrayConst(&vec3dCvfArray));
    arrayWrapperConstRefTestFunction(wrapArrayConst(&cvec3dCvfArray));
    arrayWrapperConstRefTestFunction(wrapArrayConst(&uintCvfArray));
    arrayWrapperConstRefTestFunction(wrapArrayConst(&cuintCvfArray));
    arrayWrapperConstRefTestFunction(wrapArrayConst(siztBarePtrArray, 2));
    arrayWrapperConstRefTestFunction(wrapArrayConst(siztBarePtr, 2));
    arrayWrapperConstRefTestFunction(wrapArrayConst(csiztBarePtr, 2));
    arrayWrapperConstRefTestFunction(wrapArrayConst(doubleBarePtr,2));
    arrayWrapperConstRefTestFunction(wrapArrayConst(cdoubleBarePtr, 2));

    arrayWrapperTestFunction(wrapArrayToEdit(&vec3dStdVector));
    //arrayWrapperTestFunction3(wrapArray(&cvec3dStdVector));
    EXPECT_EQ(Vec3d::ZERO, vec3dStdVector[1]);
    EXPECT_EQ(Vec3d(1,1,1), vec3dStdVector[0]);

    arrayWrapperTestFunction(wrapArrayToEdit(&vec3dCvfArray));
    EXPECT_EQ(Vec3d::ZERO, vec3dCvfArray[1]);
    EXPECT_EQ(Vec3d(1,1,1), vec3dStdVector[0]);
    //arrayWrapperTestFunction3(wrapArray(&cvec3dCvfArray));
    arrayWrapperTestFunction(wrapArrayToEdit(&uintCvfArray));
    //arrayWrapperTestFunction3(wrapArray(&cuintCvfArray));
    arrayWrapperTestFunction(wrapArrayToEdit(siztBarePtrArray, 2));
    //arrayWrapperTestFunction3(wrapArray(csiztBarePtr, 2));
    arrayWrapperTestFunction(wrapArrayToEdit(doubleBarePtr,2));
    //arrayWrapperTestFunction3(wrapArray(cdoubleBarePtr, 2));
    EXPECT_EQ(0.0, doubleBarePtr[1]);
    EXPECT_EQ(1.0, doubleBarePtr[0]);
}

