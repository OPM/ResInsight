#pragma once

#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

class TestObj;

class Child: public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    Child();

    ~Child();


    caf::PdmChildField<TestObj*> m_testObj;
};


