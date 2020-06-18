#pragma once

#include "cafPdmDataValueField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmPointer.h"

class TestObj : public caf::PdmObjectHandle
{
public:
    TestObj();
    ~TestObj();

    caf::PdmDataValueField<double> m_position;
};
