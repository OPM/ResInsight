#pragma once

#include "cafPdmChildField.h"
#include "cafPdmPointer.h"
#include "cafPdmObjectHandle.h"

class TestObj;

class Child: public caf::PdmObjectHandle
{

public:
    Child();
    ~Child();

    caf::PdmChildField<TestObj*> m_testObj;
};


