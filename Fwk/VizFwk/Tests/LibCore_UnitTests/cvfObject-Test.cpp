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
#include "cvfObject.h"

#include "gtest/gtest.h"
#include <iostream>
#include <set>
#include <algorithm>

using namespace cvf;


class ClassA : public Object
{
public:
    ClassA()        { data = 1; }
    ClassA(int val) { data = val; }
    int data;
};

class ClassB : public Object
{
public:
    ClassB() { data = 2; }
    int data;
};

class DerivedClass : public ClassA
{
public:
    DerivedClass() { data = -1; D = 0; }
    int D;
};

static ref<ClassA> createClassAInstance(int val)
{
    return new ClassA(val);
}


//==================================================================================================
//
// ObjectTest
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ObjectTest, MinimalLifeCycle)
{
    Object* obj = new ClassA;
    EXPECT_EQ(0, obj->refCount());

    delete obj;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ObjectTest, AddRefAndRelease)
{
    Object* obj = new ClassA;
    EXPECT_EQ(0, obj->refCount());

    EXPECT_EQ(1, obj->addRef());
    EXPECT_EQ(1, obj->refCount());

    EXPECT_EQ(2, obj->addRef());
    EXPECT_EQ(2, obj->refCount());
    
    EXPECT_EQ(1, obj->release());
    EXPECT_EQ(1, obj->refCount());

    EXPECT_EQ(0, obj->release());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_TIGHT_ASSERTS == 1
TEST(ObjectDeathTest, AddRefAndReleaseBehaviorOnNullPointer)
{
    Object* obj = NULL;

    EXPECT_DEATH(obj->addRef(), "Assertion");
    EXPECT_DEATH(obj->refCount(), "Assertion");

    // Release is OK on NULL pointer
    EXPECT_EQ(0, obj->release());
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(ObjectDeathTest, DeleteAnObjectWithAReference)
{
    Object* obj = new ClassA;
    obj->addRef();

    EXPECT_DEATH(delete obj, "Assertion");
}
#endif


//==================================================================================================
//
// RefTest - tests for ref class
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, ConstructWithNullPointer)
{
    ref<ClassA> r;
    EXPECT_EQ(NULL, r.p());

    const ref<ClassA> cr;
    EXPECT_EQ(NULL, cr.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, ConstructFromNakedPointer)
{
    ClassA* pa = new ClassA;
    DerivedClass* pd = new DerivedClass;

    {
        ref<ClassA> r(pa);
        EXPECT_EQ(pa, r.p());
        EXPECT_EQ(1, pa->refCount());
    }

    {
        ref<ClassA> r(pd);
        EXPECT_EQ(pd, r.p());
        EXPECT_EQ(1, pd->refCount());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, CopyConstructor)
{
    ClassA* pa = new ClassA;
    ref<ClassA> ra(pa);
    ASSERT_EQ(pa, ra.p());
    ASSERT_EQ(1, pa->refCount());

    {
        ref<ClassA> r(ra);
        ASSERT_EQ(pa, r.p());
        EXPECT_EQ(2, pa->refCount());
    }

    {
        ref<ClassA> r = ra;
        ASSERT_EQ(pa, r.p());
        EXPECT_EQ(2, pa->refCount());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, CopyConstructorFromTemporary)
{
    ref<ClassA> r = createClassAInstance(99);
    EXPECT_EQ(1, r->refCount());
    EXPECT_EQ(99, r->data);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, ConstructFromRelatedRef)
{
    DerivedClass* pd = new DerivedClass;
    ASSERT_EQ(-1, pd->data);

    ref<DerivedClass> rd(pd);
    ASSERT_EQ(1, pd->refCount());
    ASSERT_EQ(-1, rd->data);

    // Trigger construction from related (in this case derived)
    ref<ClassA> r(rd);
    EXPECT_EQ(pd, r.p());
    EXPECT_EQ(2, pd->refCount());
    EXPECT_EQ(-1, r->data);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, BasicLifeCycle)
{
    ref<ClassA> smartPtr = new ClassA;
    ASSERT_EQ(1, smartPtr->refCount());
    {
        ref<ClassA> smartPtr2 = smartPtr;
        ASSERT_EQ(2, smartPtr->refCount());
        ASSERT_EQ(true, smartPtr == smartPtr2);

        smartPtr->data = 3;
        ASSERT_EQ(3, smartPtr2->data);
    }

    ASSERT_EQ(1, smartPtr->refCount());
    ASSERT_EQ(3, smartPtr->data);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, NullPointerTest)
{
    ref<ClassA> smartPtr;
    ASSERT_TRUE(smartPtr.isNull());
    ASSERT_FALSE(smartPtr.notNull());

    smartPtr = new ClassA;
    ASSERT_TRUE(smartPtr.notNull());
    ASSERT_FALSE(smartPtr.isNull());

    smartPtr = NULL;
    ASSERT_TRUE(smartPtr.isNull());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, AssignmentFromNakedPointer)
{
    ClassA* pNaked = new ClassA(99);
    ASSERT_EQ(0, pNaked->refCount());

    ref<ClassA> r;
    r = pNaked;
    EXPECT_EQ(pNaked, r.p());
    EXPECT_EQ(99, r->data);
    EXPECT_EQ(1, pNaked->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, AssignmentFromNakedPointerDerivedClass)
{
    DerivedClass* pNaked = new DerivedClass;
    ASSERT_EQ(0, pNaked->refCount());
    ASSERT_EQ(-1, pNaked->data);

    ref<ClassA> r;
    r = pNaked;
    EXPECT_EQ(pNaked, r.p());
    EXPECT_EQ(-1, r->data);
    EXPECT_EQ(1, pNaked->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, AssignmentFromRef)
{
    ClassA* p1 = new ClassA(10);
    ClassA* p2 = new ClassA(20);
    ASSERT_EQ(10, p1->data);
    ASSERT_EQ(20, p2->data);

    ref<ClassA> r1(p1);
    ref<ClassA> r2(p2);
    ASSERT_EQ(p1, r1.p());
    ASSERT_EQ(p2, r2.p());
    ASSERT_EQ(1, p1->refCount());
    ASSERT_EQ(1, p2->refCount());

    ref<ClassA> r;
    r = r1;
    EXPECT_EQ(p1, r.p());
    EXPECT_EQ(10, r->data);
    EXPECT_EQ(2, p1->refCount());
    EXPECT_EQ(1, p2->refCount());

    r = r2;
    EXPECT_EQ(p2, r.p());
    EXPECT_EQ(20, r->data);
    EXPECT_EQ(1, p1->refCount());
    EXPECT_EQ(2, p2->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, AssignmentFromRefDerivedClasses)
{
    ClassA* pa = new ClassA;
    DerivedClass* pd = new DerivedClass;

    ref<ClassA> ra(pa);
    ref<DerivedClass> rd(pd);

    {
        ref<Object> r;
        
        r = ra;
        EXPECT_EQ(pa, r.p());
        EXPECT_EQ(2, pa->refCount());
        EXPECT_EQ(1, pd->refCount());

        r = rd;
        EXPECT_EQ(pd, r.p());
        EXPECT_EQ(1, pa->refCount());
        EXPECT_EQ(2, pd->refCount());
    }

    {
        ref<ClassA> r;

        r = rd;
        EXPECT_EQ(pd, r.p());
        EXPECT_EQ(-1, r->data);
        EXPECT_EQ(1, pa->refCount());
        EXPECT_EQ(2, pd->refCount());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, CompareLessThan)
{
    ref<ClassA> r1 = new ClassA(22);
    ref<ClassA> r2 = new ClassA(33);

    if (r1.p() > r2.p())
    {
        EXPECT_TRUE(r2 < r1);
        EXPECT_FALSE(r1 < r2);
    }
    else
    {
        EXPECT_TRUE(r1 < r2);
        EXPECT_FALSE(r2 < r1);
    }
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
TEST(RefTest, Swap)
{
    ClassA* p1 = new ClassA;
    ClassA* p2 = new ClassA;

    ref<ClassA> r1 = p1;
    ref<ClassA> r2 = p2;

    p1->data = 1;
    p2->data = 2;

    r1.swap(r2);

    EXPECT_EQ(p1, r2.p());
    EXPECT_EQ(p2, r1.p());
    EXPECT_EQ(2, r1->data);
    EXPECT_EQ(1, r2->data);
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
TEST(RefTest, GlobalSwap)
{
    ClassA* p1 = new ClassA;
    ClassA* p2 = new ClassA;

    ref<ClassA> r1 = p1;
    ref<ClassA> r2 = p2;

    p1->data = 1;
    p2->data = 2;

    swap(r1, r2);

    EXPECT_EQ(p1, r2.p());
    EXPECT_EQ(p2, r1.p());
    EXPECT_EQ(2, r1->data);
    EXPECT_EQ(1, r2->data);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, ComparisonWithNakedPointer)
{
    ref<ClassA> r1 = new ClassA(10);

    ClassA* pNaked1 = r1.p();
    ASSERT_EQ(10, pNaked1->data);

    EXPECT_TRUE((pNaked1 == r1));
    EXPECT_TRUE((r1 == pNaked1));
    EXPECT_FALSE((pNaked1 != r1));
    EXPECT_FALSE((r1 != pNaked1));

    ClassA* pNaked2 = new ClassA(20);
    ASSERT_EQ(20, pNaked2->data);

    EXPECT_FALSE((pNaked2 == r1));
    EXPECT_FALSE((r1 == pNaked2));
    EXPECT_TRUE((pNaked2 != r1));
    EXPECT_TRUE((r1 != pNaked2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RefTest, ComparisonWithNakedPointerDerivedClass)
{
    ref<ClassA> ra = new ClassA;
    ClassA* pNakedD = new DerivedClass;
    ASSERT_EQ(1, ra->data);
    ASSERT_EQ(-1, pNakedD->data);

    EXPECT_FALSE((pNakedD == ra));
    EXPECT_FALSE((ra == pNakedD));
    EXPECT_TRUE((pNakedD != ra));
    EXPECT_TRUE((ra != pNakedD));
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
TEST(RefTest, RefObjectsInStdSet)
{
    // This test relies on the less-than operator in cvf::ref
    // The test ensures that no other overload gets used instead when ordering the elements of the set
    ref<ClassA> r1 = new ClassA;
    ref<ClassA> r2 = new ClassA;
    ref<ClassA> r3 = new ClassA;
    r1->data = 1;
    r2->data = 2;
    r3->data = 3;
    EXPECT_EQ(1, r1->data);
    EXPECT_EQ(2, r2->data);
    EXPECT_EQ(3, r3->data);
    EXPECT_EQ(1, r1->refCount());
    EXPECT_EQ(1, r2->refCount());
    EXPECT_EQ(1, r3->refCount());

    ClassA* p1 = r1.p();
    ClassA* p2 = r2.p();
    ClassA* p3 = r3.p();

    std::set<ref<ClassA> > mySet;

    mySet.insert(p1);
    EXPECT_EQ(2, r1->refCount());
    EXPECT_EQ(1, mySet.size());

    mySet.insert(p1);
    EXPECT_EQ(2, r1->refCount());
    EXPECT_EQ(1, mySet.size());

    mySet.insert(p2);
    EXPECT_EQ(2, r2->refCount());
    EXPECT_EQ(2, mySet.size());

    mySet.insert(p3);
    EXPECT_EQ(2, r3->refCount());
    EXPECT_EQ(3, mySet.size());

    mySet.erase(p2);
    EXPECT_EQ(1, r2->refCount());
    EXPECT_EQ(2, mySet.size());

    EXPECT_TRUE(mySet.find(p1) != mySet.end());
    EXPECT_TRUE(mySet.find(p3) != mySet.end());
    EXPECT_TRUE(mySet.find(p2) == mySet.end());

    mySet.clear();
    EXPECT_EQ(1, r1->refCount());
    EXPECT_EQ(1, r2->refCount());
    EXPECT_EQ(1, r3->refCount());
    EXPECT_EQ(0, mySet.size());
}



//==================================================================================================
//
// ConstRefTest - tests for cref class
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, ConstructWithNullPointer)
{
    cref<ClassA> r;
    EXPECT_EQ(NULL, r.p());

    const cref<ClassA> cr;
    EXPECT_EQ(NULL, cr.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, ConstructFromNakedPointer)
{
    ClassA* pa1 = new ClassA;
    const ClassA* pa2 = new ClassA;
    DerivedClass* pd = new DerivedClass;

    {
        cref<ClassA> r(pa1);
        EXPECT_EQ(pa1, r.p());
        EXPECT_EQ(1, pa1->refCount());
    }

    {
        cref<ClassA> r(pa2);
        EXPECT_EQ(pa2, r.p());
        EXPECT_EQ(1, pa2->refCount());
    }

    {
        cref<ClassA> r(pd);
        EXPECT_EQ(pd, r.p());
        EXPECT_EQ(1, pd->refCount());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, CopyConstructor)
{
    ClassA* pa = new ClassA;
    cref<ClassA> ra(pa);
    ASSERT_EQ(pa, ra.p());
    ASSERT_EQ(1, pa->refCount());

    {
        cref<ClassA> r(ra);
        ASSERT_EQ(pa, r.p());
        EXPECT_EQ(2, pa->refCount());
    }

    {
        cref<ClassA> r = ra;
        ASSERT_EQ(pa, r.p());
        EXPECT_EQ(2, pa->refCount());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, ConstructFromRelatedRef)
{
    DerivedClass* pd = new DerivedClass;
    ASSERT_EQ(-1, pd->data);

    cref<DerivedClass> rd(pd);
    ASSERT_EQ(1, pd->refCount());
    ASSERT_EQ(-1, rd->data);

    // Trigger construction from related (in this case derived)
    cref<ClassA> r(rd);
    EXPECT_EQ(pd, r.p());
    EXPECT_EQ(2, pd->refCount());
    EXPECT_EQ(-1, r->data);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, NullPointerTest)
{
    cref<ClassA> smartPtr;
    ASSERT_TRUE(smartPtr.isNull());
    ASSERT_FALSE(smartPtr.notNull());

    smartPtr = new ClassA;
    ASSERT_TRUE(smartPtr.notNull());
    ASSERT_FALSE(smartPtr.isNull());

    smartPtr = NULL;
    ASSERT_TRUE(smartPtr.isNull());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, AssignmentFromNakedPointer)
{
    const ClassA* pNaked = new ClassA(99);
    ASSERT_EQ(0, pNaked->refCount());

    cref<ClassA> r;
    r = pNaked;
    EXPECT_EQ(pNaked, r.p());
    EXPECT_EQ(99, r->data);
    EXPECT_EQ(1, pNaked->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, AssignmentFromNakedPointerDerivedClass)
{
    const DerivedClass* pNaked = new DerivedClass;
    ASSERT_EQ(0, pNaked->refCount());
    ASSERT_EQ(-1, pNaked->data);

    cref<ClassA> r;
    r = pNaked;
    EXPECT_EQ(pNaked, r.p());
    EXPECT_EQ(-1, r->data);
    EXPECT_EQ(1, pNaked->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, AssignmentFromRef)
{
    const ClassA* p1 = new ClassA(10);
    const ClassA* p2 = new ClassA(20);
    ASSERT_EQ(10, p1->data);
    ASSERT_EQ(20, p2->data);

    cref<ClassA> r1(p1);
    cref<ClassA> r2(p2);
    ASSERT_EQ(p1, r1.p());
    ASSERT_EQ(p2, r2.p());
    ASSERT_EQ(1, p1->refCount());
    ASSERT_EQ(1, p2->refCount());

    cref<ClassA> r;
    r = r1;
    EXPECT_EQ(p1, r.p());
    EXPECT_EQ(10, r->data);
    EXPECT_EQ(2, p1->refCount());
    EXPECT_EQ(1, p2->refCount());

    r = r2;
    EXPECT_EQ(p2, r.p());
    EXPECT_EQ(20, r->data);
    EXPECT_EQ(1, p1->refCount());
    EXPECT_EQ(2, p2->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, AssignmentFromRefDerivedClasses)
{
    const ClassA* pa = new ClassA;
    const DerivedClass* pd = new DerivedClass;

    cref<ClassA> ra(pa);
    cref<DerivedClass> rd(pd);

    {
        cref<Object> r;

        r = ra;
        EXPECT_EQ(pa, r.p());
        EXPECT_EQ(2, pa->refCount());
        EXPECT_EQ(1, pd->refCount());

        r = rd;
        EXPECT_EQ(pd, r.p());
        EXPECT_EQ(1, pa->refCount());
        EXPECT_EQ(2, pd->refCount());
    }

    {
        cref<ClassA> r;

        r = rd;
        EXPECT_EQ(pd, r.p());
        EXPECT_EQ(-1, r->data);
        EXPECT_EQ(1, pa->refCount());
        EXPECT_EQ(2, pd->refCount());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, CompareLessThan)
{
    cref<ClassA> r1 = new ClassA(22);
    cref<ClassA> r2 = new ClassA(33);

    if (r1.p() > r2.p())
    {
        EXPECT_TRUE(r2 < r1);
        EXPECT_FALSE(r1 < r2);
    }
    else
    {
        EXPECT_TRUE(r1 < r2);
        EXPECT_FALSE(r2 < r1);
    }
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, Swap)
{
    const ClassA* p1 = new ClassA(1);
    const ClassA* p2 = new ClassA(2);

    cref<ClassA> r1 = p1;
    cref<ClassA> r2 = p2;

    r1.swap(r2);

    EXPECT_EQ(p1, r2.p());
    EXPECT_EQ(p2, r1.p());
    EXPECT_EQ(2, r1->data);
    EXPECT_EQ(1, r2->data);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, ComparisonWithNakedPointer)
{
    cref<ClassA> r1 = new ClassA(10);

    const ClassA* pNaked1 = r1.p();
    ASSERT_EQ(10, pNaked1->data);

    EXPECT_TRUE((pNaked1 == r1));
    EXPECT_TRUE((r1 == pNaked1));
    EXPECT_FALSE((pNaked1 != r1));
    EXPECT_FALSE((r1 != pNaked1));

    const ClassA* pNaked2 = new ClassA(20);
    ASSERT_EQ(20, pNaked2->data);

    EXPECT_FALSE((pNaked2 == r1));
    EXPECT_FALSE((r1 == pNaked2));
    EXPECT_TRUE((pNaked2 != r1));
    EXPECT_TRUE((r1 != pNaked2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, ComparisonWithNakedPointerDerivedClass)
{
    cref<ClassA> ra = new ClassA;
    const ClassA* pNakedD = new DerivedClass;
    ASSERT_EQ(1, ra->data);
    ASSERT_EQ(-1, pNakedD->data);

    EXPECT_FALSE((pNakedD == ra));
    EXPECT_FALSE((ra == pNakedD));
    EXPECT_TRUE((pNakedD != ra));
    EXPECT_TRUE((ra != pNakedD));
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
TEST(ConstRefTest, RefObjectsInStdSet)
{
    // This test relies on the less-than operator in cvf::cref
    // The test ensures that no other overload gets used instead when ordering the elements of the set
    cref<ClassA> r1 = new ClassA(1);
    cref<ClassA> r2 = new ClassA(2);
    cref<ClassA> r3 = new ClassA(3);
    EXPECT_EQ(1, r1->data);
    EXPECT_EQ(2, r2->data);
    EXPECT_EQ(3, r3->data);
    EXPECT_EQ(1, r1->refCount());
    EXPECT_EQ(1, r2->refCount());
    EXPECT_EQ(1, r3->refCount());

    const ClassA* p1 = r1.p();
    const ClassA* p2 = r2.p();
    const ClassA* p3 = r3.p();

    std::set<cref<ClassA> > mySet;

    mySet.insert(p1);
    EXPECT_EQ(2, r1->refCount());
    EXPECT_EQ(1, mySet.size());

    mySet.insert(p1);
    EXPECT_EQ(2, r1->refCount());
    EXPECT_EQ(1, mySet.size());

    mySet.insert(p2);
    EXPECT_EQ(2, r2->refCount());
    EXPECT_EQ(2, mySet.size());

    mySet.insert(p3);
    EXPECT_EQ(2, r3->refCount());
    EXPECT_EQ(3, mySet.size());

    mySet.erase(p2);
    EXPECT_EQ(1, r2->refCount());
    EXPECT_EQ(2, mySet.size());

    EXPECT_TRUE(mySet.find(p1) != mySet.end());
    EXPECT_TRUE(mySet.find(p3) != mySet.end());
    EXPECT_TRUE(mySet.find(p2) == mySet.end());

    mySet.clear();
    EXPECT_EQ(1, r1->refCount());
    EXPECT_EQ(1, r2->refCount());
    EXPECT_EQ(1, r3->refCount());
    EXPECT_EQ(0, mySet.size());
}

