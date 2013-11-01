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
#include "cvfCollection.h"
#include "cvfMath.h"

#include "gtest/gtest.h"
#include <iostream>

using namespace cvf;




class MyObj : public Object
{
public:
    MyObj() { num_ = 0; }
    MyObj(int num) { num_ = num; }

    int num() const { return num_; }
    void num(int num) { num_ = num; }

    bool operator<(const MyObj& rhs)
    {
        return num_ < rhs.num_;
    }
    
private:
    int num_;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, DefaultConstructor)
{
    Collection<MyObj> col;
    EXPECT_EQ(0, col.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, CopyConstructor)
{
    Collection<MyObj> col1;
    col1.push_back(new MyObj(1));
    col1.push_back(new MyObj(2));
    ASSERT_EQ(2, col1.size());
    ASSERT_EQ(1, col1[0]->refCount());
    ASSERT_EQ(1, col1[1]->refCount());

    Collection<MyObj> col2(col1);
    ASSERT_EQ(2, col2.size());
    ASSERT_EQ(2, col2[0]->refCount());
    ASSERT_EQ(2, col2[1]->refCount());

    col2.push_back(new MyObj(99));
    ASSERT_EQ(2, col1.size());
    ASSERT_EQ(3, col2.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, ConstructFromStdVector)
{
    std::vector< ref<MyObj> >  native;
    native.push_back(new MyObj(10));
    native.push_back(new MyObj(11));

    Collection<MyObj> col2(native);
    ASSERT_EQ(2u, col2.size());
    ASSERT_EQ(10, col2[0]->num());
    ASSERT_EQ(11, col2[1]->num());
    ASSERT_EQ(2, col2[0]->refCount());
    ASSERT_EQ(2, col2[1]->refCount());

    Collection<MyObj> col3(col2);
    ASSERT_EQ(2u, col3.size());
    ASSERT_EQ(10, col3[0]->num());
    ASSERT_EQ(11, col3[1]->num());
    ASSERT_EQ(3, col3[0]->refCount());
    ASSERT_EQ(3, col3[1]->refCount());
    ASSERT_EQ(3, native[0]->refCount());
    ASSERT_EQ(3, native[1]->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, Assignment)
{
    Collection<MyObj> col1;
    col1.push_back(new MyObj(1));
    col1.push_back(new MyObj(2));
    ASSERT_EQ(2, col1.size());
    ASSERT_EQ(1, col1[0]->refCount());
    ASSERT_EQ(1, col1[1]->refCount());

    Collection<MyObj> col2;
    col2 = col1;
    ASSERT_EQ(2, col2.size());
    ASSERT_EQ(2, col2[0]->refCount());
    ASSERT_EQ(2, col2[1]->refCount());

    col2.push_back(new MyObj(99));
    ASSERT_EQ(2, col1.size());
    ASSERT_EQ(3, col2.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, Resize)
{
    Collection<MyObj> col3;
    col3.push_back(new MyObj(1));
    col3.push_back(new MyObj(2));

    col3.resize(3);

    ASSERT_TRUE(col3[2].isNull());

    MyObj* o = new MyObj(99);
    col3[2] = o;

    ASSERT_EQ(3u, col3.size());
    ASSERT_EQ(99, col3[2]->num());
    ASSERT_EQ(1, col3[2]->refCount());

    o->addRef();
    ASSERT_EQ(2, o->refCount());

    // Remove o from the collection (last element, check that it has been released)
    col3.resize(2);
    ASSERT_EQ(2u, col3.size());
    ASSERT_EQ(1, o->refCount());
    ASSERT_EQ(99, o->num());

    col3.clear();
    ASSERT_EQ(0u, col3.size());

    o->release();
    o = 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, ElementAccess)
{
    Collection<MyObj> col;
    Collection<MyObj> col2;

    col.push_back(new MyObj(1));
    col.push_back(new MyObj(2));
    col.push_back(new MyObj(3));

    col2 = col;

    ASSERT_EQ(1, col2[0]->num());
    ASSERT_EQ(2, col2[1]->num());
    ASSERT_EQ(3, col2[2]->num());
    ASSERT_EQ(2, col2[0]->refCount());
    ASSERT_EQ(2, col2[1]->refCount());
    ASSERT_EQ(2, col2[2]->refCount());

    col.clear();

    ASSERT_EQ(1, col2[0]->num());
    ASSERT_EQ(2, col2[1]->num());
    ASSERT_EQ(3, col2[2]->num());
    ASSERT_EQ(1, col2[0]->refCount());
    ASSERT_EQ(1, col2[1]->refCount());
    ASSERT_EQ(1, col2[2]->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, Push_back)
{
    Collection<MyObj> col;

    col.push_back(new MyObj(1));
    col.push_back(new MyObj(2));

    ref<MyObj> o = new MyObj(3);
    col.push_back(o.p());

    ASSERT_EQ(3u, col.size());
    EXPECT_EQ(1, col[0]->num());
    EXPECT_EQ(2, col[1]->num());
    EXPECT_EQ(3, col[2]->num());
    EXPECT_EQ(1, col[0]->refCount());
    EXPECT_EQ(1, col[1]->refCount());
    EXPECT_EQ(2, col[2]->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, Contains)
{
    ref<MyObj> o1 = new MyObj;
    ref<MyObj> o2 = new MyObj;
    ref<MyObj> o3 = new MyObj;

    Collection<MyObj> col;
    col.push_back(o1.p());
    col.push_back(o2.p());

    ASSERT_TRUE(col.contains(o1.p()));
    ASSERT_TRUE(col.contains(o2.p()));
    ASSERT_FALSE(col.contains(o3.p()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, IndexOf)
{
    ref<MyObj> o1 = new MyObj;
    ref<MyObj> o2 = new MyObj;
    ref<MyObj> o3 = new MyObj;

    Collection<MyObj> col;
    col.push_back(o1.p());
    col.push_back(o2.p());

    EXPECT_EQ(0, col.indexOf(o1.p()));
    EXPECT_EQ(1, col.indexOf(o2.p()));
    
    EXPECT_EQ(UNDEFINED_SIZE_T, col.indexOf(NULL));
    EXPECT_EQ(UNDEFINED_SIZE_T, col.indexOf(o3.p()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, Erase)
{
    ref<MyObj> o1 = new MyObj(1);
    ref<MyObj> o2 = new MyObj(2);
    ref<MyObj> o3 = new MyObj(3);
    ref<MyObj> o4 = new MyObj(4);

    MyObj* po1 = o1.p();
    MyObj* po2 = o2.p();
    MyObj* po3 = o3.p();
    MyObj* po4 = o3.p();

    Collection<MyObj> col;
    col.push_back(po1);
    col.push_back(po2);
    col.push_back(po3);
    col.push_back(po4);
    EXPECT_EQ(4, col.size());

    col.erase(NULL);
    EXPECT_EQ(4, col.size());

    col.erase(po2);
    EXPECT_EQ(3, col.size());

    col.erase(NULL);
    EXPECT_EQ(3, col.size());

    col.eraseAt(1);
    ASSERT_EQ(2, col.size());

    EXPECT_EQ(po1, col[0].p());
    EXPECT_EQ(po4, col[1].p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(CollectionDeathTest, IllegalEraseAt)
{
    Collection<MyObj> col;
    EXPECT_DEATH(col.eraseAt(0), "Assertion");
    EXPECT_DEATH(col.eraseAt(1), "Assertion");

    col.push_back(new MyObj(1));
    col.push_back(new MyObj(2));
    EXPECT_EQ(2, col.size());

    EXPECT_DEATH(col.eraseAt(2), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, IteratorTest)
{
    ref<MyObj> o1 = new MyObj(2);
    ref<MyObj> o2 = new MyObj(3);
    ref<MyObj> o3 = new MyObj(4);
    ref<MyObj> o4 = new MyObj(5);

    MyObj* po1 = o1.p();
    MyObj* po2 = o2.p();
    MyObj* po3 = o3.p();
    MyObj* po4 = o4.p();

    Collection<MyObj> col;
    col.push_back(po1);
    col.push_back(po2);
    col.push_back(po3);
    col.push_back(po4);
    EXPECT_EQ(4, col.size());

    Collection<MyObj>::iterator it = col.begin();

    int sum = 0;
    int count = 0;
    while (it != col.end())
    {
        MyObj* obj = it->p();
        ASSERT_TRUE(obj != NULL);
        sum += obj->num();
        count++;
        it++;
    }

    ASSERT_EQ(4, count);
    ASSERT_EQ(14, sum);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, ConstIteratorTest)
{
    ref<MyObj> o1 = new MyObj(2);
    ref<MyObj> o2 = new MyObj(3);
    ref<MyObj> o3 = new MyObj(4);
    ref<MyObj> o4 = new MyObj(5);

    MyObj* po1 = o1.p();
    MyObj* po2 = o2.p();
    MyObj* po3 = o3.p();
    MyObj* po4 = o4.p();

    Collection<MyObj> col;
    col.push_back(po1);
    col.push_back(po2);
    col.push_back(po3);
    col.push_back(po4);
    EXPECT_EQ(4, col.size());

    Collection<MyObj>::const_iterator it = col.begin();

    int sum = 0;
    int count = 0;
    while (it != col.end())
    {
        const MyObj* obj = it->p();
        ASSERT_TRUE(obj != NULL);
        sum += obj->num();
        count++;
        it++;
    }

    ASSERT_EQ(4, count);
    ASSERT_EQ(14, sum);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool compFunc(ref<MyObj> i,ref<MyObj> j) { return (*i<*j); }
TEST(CollectionTest, SortAndBinSearchTest)
{
    ref<MyObj> o1 = new MyObj(5);
    ref<MyObj> o2 = new MyObj(2);
    ref<MyObj> o3 = new MyObj(1);
    ref<MyObj> o4 = new MyObj(4);

    MyObj* po1 = o1.p();
    MyObj* po2 = o2.p();
    MyObj* po3 = o3.p();
    MyObj* po4 = o4.p();

    Collection<MyObj> col;
    col.push_back(po1);
    col.push_back(po2);
    col.push_back(po3);
    col.push_back(po4);
    EXPECT_EQ(4, col.size());

    // Using standard stl sort algorithm
    sort(col.begin(), col.end(), compFunc);

    EXPECT_EQ(1, col[0]->num());
    EXPECT_EQ(2, col[1]->num());
    EXPECT_EQ(4, col[2]->num());
    EXPECT_EQ(5, col[3]->num());

    ref<MyObj> valToFind = new MyObj(2);
    EXPECT_TRUE(binary_search(col.begin(), col.end(), valToFind, compFunc));

    valToFind->num(5);
    EXPECT_TRUE(binary_search(col.begin(), col.end(), valToFind, compFunc));

    valToFind->num(88);
    EXPECT_FALSE(binary_search(col.begin(), col.end(), valToFind, compFunc));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void testFunc(ref<MyObj> obj) { obj->num(obj->num()*2); }
TEST(CollectionTest, ForEach)
{
    Collection<MyObj> col;
    col.push_back(new MyObj(5));
    col.push_back(new MyObj(3));
    col.push_back(new MyObj(8));
    col.push_back(new MyObj(2));

    for_each(col.begin(), col.end(), testFunc);

    EXPECT_EQ(10, col[0]->num());
    EXPECT_EQ(6, col[1]->num());
    EXPECT_EQ(16, col[2]->num());
    EXPECT_EQ(4, col[3]->num());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CollectionTest, Reverse)
{
    Collection<MyObj> col;
    col.push_back(new MyObj(5));
    col.push_back(new MyObj(3));
    col.push_back(new MyObj(8));
    col.push_back(new MyObj(2));

    reverse(col.begin(), col.end());

    EXPECT_EQ(2, col[0]->num());
    EXPECT_EQ(8, col[1]->num());
    EXPECT_EQ(3, col[2]->num());
    EXPECT_EQ(5, col[3]->num());
}



