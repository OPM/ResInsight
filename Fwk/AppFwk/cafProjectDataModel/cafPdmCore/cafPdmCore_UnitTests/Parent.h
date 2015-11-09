#pragma once

#include "cafPdmObjectHandle.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"

class Child;

class Parent : public caf::PdmObjectHandle
{
public:
    Parent();
   ~Parent();

    void doSome();

    caf::PdmChildArrayField<Child*> m_simpleObjectsField;
    caf::PdmChildField<Child*>           m_simpleObjectF;
};
