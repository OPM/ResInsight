#include "TestObj.h"

CAF_PDM_SOURCE_INIT( TestObj, "TestObj" );

TestObj::TestObj()
{
    CAF_PDM_InitObject( "TestObj", "", "", "" );
    CAF_PDM_InitField( &m_position, "Position", 8765.2, "Position", "", "", "" );
}

TestObj::~TestObj()
{
}
