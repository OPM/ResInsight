#pragma once

#include "cafPdmPointer.h"
#include "cafPdmDataValueField.h"
#include "cafPdmObjectHandle.h"

class TestObj : public caf::PdmObjectHandle
{
public:
    TestObj();
    ~TestObj();

    caf::PdmDataValueField<double>  m_position;
};
