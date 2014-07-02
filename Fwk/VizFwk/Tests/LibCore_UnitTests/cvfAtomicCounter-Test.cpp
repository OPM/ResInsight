//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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



#include "cvfAtomicCounter.h"

#ifdef CVF_ATOMIC_COUNTER_CLASS_EXISTS

#include "cvfDebugTimer.h"
#include "cvfObject.h"
#include "cvfCollection.h"

#include "gtest/gtest.h"

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
TEST(DISABLED_ObjectConstructionBenchmark, TestBasicObjectConstruction)
{
    int objectCount = 1000000;
    int iterationCount = 5;

    String sNumber(objectCount);
    String refCountTxt = String("TestBasicObjectConstruction : ") + sNumber;
    DebugTimer tim(refCountTxt.toAscii().ptr());

    for (int iteration = 0; iteration < iterationCount; iteration++)
    {
        for (int i = 0; i < objectCount; i++)
        {
            MyObj* r2 = new MyObj();
            
            r2->addRef();
            r2->release();
        }

        tim.reportLapTimeMS();
    }
}



class ObjectReferencingSharedObject : public Object
{
public:
    ObjectReferencingSharedObject() { }

    cvf::ref<MyObj> m_sharedObject;
};


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_ObjectConstructionBenchmark, TestReferenceOtherObject)
{
    int objectCount = 1000000;
    int iterationCount = 5;

    String sNumber(objectCount);
    String refCountTxt = String("TestReferenceOtherObjectClass : ") + sNumber;
    DebugTimer tim(refCountTxt.toAscii().ptr());

    for (int iteration = 0; iteration < iterationCount; iteration++)
    {
        cvf::ref<MyObj> sharedObj = new MyObj();

        std::vector< cvf::ref<ObjectReferencingSharedObject> > col;
        col.resize(objectCount);

        for (int i = 0; i < objectCount; i++)
        {
            cvf::ref<ObjectReferencingSharedObject> newObj = new ObjectReferencingSharedObject();
            newObj->m_sharedObject = sharedObj.p();

            col[i] = newObj;
        }

        String sNumber(sharedObj->refCount());
        String refCountTxt = String("Shared object reference count : ") + sNumber;

        tim.reportLapTimeMS(refCountTxt.toAscii().ptr());
    }
}


#endif //#ifdef CVF_ATOMIC_COUNTER_CLASS_EXISTS
