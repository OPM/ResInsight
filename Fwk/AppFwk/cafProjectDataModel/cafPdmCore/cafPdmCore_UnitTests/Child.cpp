#include "Child.h"
#include "TestObj.h"

Child::Child()
{
    this->addField( &m_testObj, "Numbers" );
}

Child::~Child()
{
    delete m_testObj();
}
