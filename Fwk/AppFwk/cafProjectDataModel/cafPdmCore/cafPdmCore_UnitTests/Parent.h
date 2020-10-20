#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmObjectHandle.h"

class Child;

class Parent : public caf::PdmObjectHandle
{
public:
    Parent();
    ~Parent();

    void doSome();

    caf::PdmChildArrayField<Child*> m_simpleObjectsField;
    caf::PdmChildField<Child*>      m_simpleObjectF;
};
