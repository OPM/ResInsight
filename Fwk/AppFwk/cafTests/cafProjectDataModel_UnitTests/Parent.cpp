#include "Child.h"
#include "Parent.h"

#include "gtest/gtest.h"

CAF_PDM_SOURCE_INIT(Parent, "Parent");


Parent::Parent()
{
    CAF_PDM_InitObject("Parent", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_simpleObjectsField, "SimpleObjects",  "A child object", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_simpleObjectF, "SimpleObject",  "A child object", "", "", "");
}

Parent::~Parent()
{
}

 void Parent::doSome()
{
   size_t i =  m_simpleObjectsField.size();
   if (i){
      //Child* c = m_simpleObjectsField[0];
      //TestObj* to = c->m_testObj();
   }
}

 TEST(IncludeTest, Basic)
 {
    Parent* p = new Parent;
    delete(p);
 }
