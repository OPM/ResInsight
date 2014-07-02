#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

class TestObj: public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;


public:
    TestObj();

    ~TestObj();

    caf::PdmField<double>               m_position;
};
