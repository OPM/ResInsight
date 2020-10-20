#include "Parent.h"
#include "Child.h"

Parent::Parent()
{
    this->addField( &m_simpleObjectsField, "SimpleObjects" );
    this->addField( &m_simpleObjectF, "SimpleObject" );
}

Parent::~Parent()
{
    delete m_simpleObjectF();
    m_simpleObjectsField.deleteAllChildObjects();
}

void Parent::doSome()
{
    size_t i = m_simpleObjectsField.size();
    if ( i )
    {
        // Child* c = m_simpleObjectsField[0];
        // TestObj* to = c->m_testObj();
    }
}

#include <gtest/gtest.h>

TEST( IncludeTest, Basic )
{
    Parent* p = new Parent;
    delete ( p );
}
