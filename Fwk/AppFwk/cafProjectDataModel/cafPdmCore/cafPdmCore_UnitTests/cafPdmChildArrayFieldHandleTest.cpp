
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
        name = QString( "Name %1" ).arg( id() );
    }

    caf::PdmDataValueField<QString> name;
    caf::PdmDataValueField<int>     id;
};

class SimpleObjDerived : public MsjSimpleObj
{
public:
    caf::Signal<> basicSignal;

public:
    SimpleObjDerived()
        : MsjSimpleObj()
        , basicSignal( this )
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

    ~ContainerObj() {
        /*
        *
        *  Test code used when debugging crash related to delete of objects and disconnect of signals
        *
                QString timestamp = QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss.zzz" );
                std::cout << timestamp.toStdString() << std::endl;

                auto count = observedSignals().size();

                std::cout << "Destructor ~ContainerObj: " << count << std::endl;
        */
    };

    void setBasicSignalReceived( const caf::SignalEmitter* emitter ) {}

    void createAndAppendObject()
    {
        SimpleObjDerived* s = new SimpleObjDerived;
        s->basicSignal.connect( this, &ContainerObj::setBasicSignalReceived );

        derivedObjs.push_back( s );
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

TEST( ChildArrayFieldHandle, AsyncDeleteOfMultipleChildren )
{
    // https://github.com/OPM/ResInsight/issues/12262
    //
    // To trigger crash, remove the line
    // clearWithoutDelete();
    // from PdmChildArrayField<DataType*>::deleteChildrenAsync()
    //
    // Crash was reproduced in Debug build in VS2022 17.13.2

    ContainerObj containerObj;
    const int    numObjects = 1000;
    for ( int i = 0; i < numObjects; i++ )
    {
        containerObj.createAndAppendObject();
    }

    containerObj.derivedObjs.deleteChildrenAsync();

    // Wait for async delete to complete
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
}
