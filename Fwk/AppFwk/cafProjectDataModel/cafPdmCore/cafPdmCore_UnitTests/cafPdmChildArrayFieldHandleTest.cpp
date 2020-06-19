
#include "gtest/gtest.h"

#include "cafAppEnum.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmDataValueField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cafPdmReferenceHelper.h"

#include <QDebug>

class MsjSimpleObj : public caf::PdmObjectHandle
{
public:
    MsjSimpleObj()
        : PdmObjectHandle()
    {
        this->addField( &name, "Name" );
        this->addField( &id, "ID" );

        static int a = 0;

        id   = a++;
        name = QString( "Name %1" ).arg( id );
    }

    caf::PdmDataValueField<QString> name;
    caf::PdmDataValueField<int>     id;
};

class SimpleObjDerived : public MsjSimpleObj
{
public:
    SimpleObjDerived()
        : MsjSimpleObj()
    {
        this->addField( &valueA, "valueA" );
    }

    caf::PdmDataValueField<int> valueA;
};

class SimpleObjDerivedOther : public MsjSimpleObj
{
public:
    SimpleObjDerivedOther()
        : MsjSimpleObj()
    {
        this->addField( &valueDouble, "valueDouble" );
    }

    caf::PdmDataValueField<double> valueDouble;
};

class ContainerObj : public caf::PdmObjectHandle
{
public:
    ContainerObj()
        : PdmObjectHandle()
    {
        this->addField( &derivedObjs, "derivedObjs" );
        this->addField( &derivedOtherObjs, "derivedOtherObjs" );
    }

    ~ContainerObj()
    {
        derivedObjs.deleteAllChildObjects();
        derivedOtherObjs.deleteAllChildObjects();
    }

    caf::PdmChildArrayField<SimpleObjDerived*>      derivedObjs;
    caf::PdmChildArrayField<SimpleObjDerivedOther*> derivedOtherObjs;
};

template <class U, typename T>
U findObjectById( T start, T end, int id )
{
    for ( T it = start; it != end; it++ )
    {
        if ( id == it->p()->id() )
        {
            return it->p();
        }
    }

    return NULL;
}

TEST( ChildArrayFieldHandle, DerivedObjects )
{
    ContainerObj* containerObj = new ContainerObj;

    SimpleObjDerived* s0 = new SimpleObjDerived;
    SimpleObjDerived* s1 = new SimpleObjDerived;
    SimpleObjDerived* s2 = new SimpleObjDerived;
    containerObj->derivedObjs.push_back( s0 );
    containerObj->derivedObjs.push_back( s1 );
    containerObj->derivedObjs.push_back( s2 );

    SimpleObjDerived* myObj =
        findObjectById<SimpleObjDerived*>( containerObj->derivedObjs.begin(), containerObj->derivedObjs.end(), 2 );
    EXPECT_EQ( s2, myObj );

    myObj = findObjectById<SimpleObjDerived*>( containerObj->derivedObjs.begin(), containerObj->derivedObjs.end(), -1 );
    EXPECT_EQ( NULL, myObj );

    delete containerObj;
}

TEST( ChildArrayFieldHandle, DerivedOtherObjects )
{
    ContainerObj* containerObj = new ContainerObj;

    SimpleObjDerivedOther* s0 = new SimpleObjDerivedOther;
    SimpleObjDerivedOther* s1 = new SimpleObjDerivedOther;
    SimpleObjDerivedOther* s2 = new SimpleObjDerivedOther;

    int s2Id = s2->id;

    containerObj->derivedOtherObjs.push_back( s0 );
    containerObj->derivedOtherObjs.push_back( s1 );
    containerObj->derivedOtherObjs.push_back( s2 );

    SimpleObjDerivedOther* myObj = findObjectById<SimpleObjDerivedOther*>( containerObj->derivedOtherObjs.begin(),
                                                                           containerObj->derivedOtherObjs.end(),
                                                                           s2Id );
    EXPECT_EQ( s2, myObj );

    myObj = findObjectById<SimpleObjDerivedOther*>( containerObj->derivedOtherObjs.begin(),
                                                    containerObj->derivedOtherObjs.end(),
                                                    -1 );
    EXPECT_EQ( NULL, myObj );

    delete containerObj;
}
