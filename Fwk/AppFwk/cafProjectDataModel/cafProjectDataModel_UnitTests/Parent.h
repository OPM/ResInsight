#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#if 0
class PdmPointerTarget
{
public:
   PdmPointerTarget() {}
   PdmPointerTarget(const PdmPointerTarget& ) {}
   PdmPointerTarget& operator=(const PdmPointerTarget& ) {}

   virtual ~PdmPointerTarget()
   {
      // Set all guarded pointers pointing to this to NULL

      std::set<PdmObject**>::iterator it;
      for (it = m_pointersReferencingMe.begin(); it != m_pointersReferencingMe.end() ; ++it)
      {
          (**it) = NULL;
      }
   }

private:

   // Support system for PdmPointer

   friend class PdmPointerImpl;
   std::set<PdmPointerTarget**>         m_pointersReferencingMe;
};

#endif

class Child;

class Parent : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    Parent();
    ~Parent();

    void doSome();

    caf::PdmChildArrayField<Child*> m_simpleObjectsField;
    caf::PdmChildField<Child*>      m_simpleObjectF;
};
