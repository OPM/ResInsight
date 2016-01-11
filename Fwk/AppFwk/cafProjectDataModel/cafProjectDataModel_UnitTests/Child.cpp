#include "Child.h"
#include "TestObj.h"

CAF_PDM_SOURCE_INIT(Child, "Child");

Child::Child()
{
    CAF_PDM_InitFieldNoDefault(&m_testObj, "Numbers", "Important Numbers", "", "", "");
}

Child::~Child()
{
}
