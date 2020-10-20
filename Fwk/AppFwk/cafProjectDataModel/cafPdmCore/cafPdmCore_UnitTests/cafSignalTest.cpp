#include "gtest/gtest.h"

#include "cafSignal.h"

#include <memory>
#include <string>
#include <tuple>

struct SimpleStruct
{
    int         test1;
    double      test2;
    std::string test3;

    SimpleStruct( int test1 = 0, double test2 = 0.0, std::string test3 = "" )
        : test1( test1 )
        , test2( test2 )
        , test3( test3 )
    {
    }

    bool operator==( const SimpleStruct& rhs ) const
    {
        return test1 == rhs.test1 && test2 == rhs.test2 && test3 == rhs.test3;
    }
    bool operator!=( const SimpleStruct& rhs ) const { return !( *this == rhs ); }
};

class TestEmitter : public caf::SignalEmitter
{
public:
    caf::Signal<>                                   basicSignal;
    caf::Signal<bool>                               boolSignal;
    caf::Signal<std::string>                        stringSignal;
    caf::Signal<std::tuple<bool, std::string, int>> tupleSignal;
    caf::Signal<SimpleStruct>                       structSignal;

public:
    TestEmitter()
        : basicSignal( this )
        , boolSignal( this )
        , stringSignal( this )
        , tupleSignal( this )
        , structSignal( this )
    {
    }
    void triggerBasicSignal() { basicSignal.send(); }

    void setBoolValue( bool test ) { boolSignal.send( test ); }
    void setStringValue( std::string test ) { stringSignal.send( test ); }
    void setTupleValue( std::tuple<bool, std::string, int> test ) { tupleSignal.send( test ); }
    void setSimpleStruct( const SimpleStruct& test ) { structSignal.send( test ); }
};

class TestObserver : public caf::SignalObserver
{
public:
    TestObserver()
        : m_receivedBasicSignal( false )
        , m_boolValue( false )
        , m_stringValue( "" )
        , m_tupleValue( false, "", 0 )
    {
    }

    void connectAllSignals( TestEmitter* emitter )
    {
        emitter->basicSignal.connect( this, &TestObserver::setBasicSignalReceived );
        emitter->boolSignal.connect( this, &TestObserver::setBoolValue );
        emitter->stringSignal.connect( this, &TestObserver::setStringValue );
        emitter->tupleSignal.connect( this, &TestObserver::setTupleValue );
        emitter->structSignal.connect( this, &TestObserver::setSimpleStruct );
    }

    void setBasicSignalReceived( const caf::SignalEmitter* emitter ) { m_receivedBasicSignal = true; }
    void setBoolValue( const caf::SignalEmitter*, bool test ) { m_boolValue = test; }
    void setStringValue( const caf::SignalEmitter*, std::string test ) { m_stringValue = test; }
    void setTupleValue( const caf::SignalEmitter*, std::tuple<bool, std::string, int> test ) { m_tupleValue = test; }
    void setSimpleStruct( const caf::SignalEmitter*, SimpleStruct test ) { m_structValue = test; }

    bool                               receivedBasicSignal() const { return m_receivedBasicSignal; }
    bool                               boolValue() const { return m_boolValue; }
    std::string                        stringValue() const { return m_stringValue; }
    std::tuple<bool, std::string, int> tupleValue() const { return m_tupleValue; }
    SimpleStruct                       structValue() const { return m_structValue; }

protected:
    bool                               m_receivedBasicSignal;
    bool                               m_boolValue;
    std::string                        m_stringValue;
    std::tuple<bool, std::string, int> m_tupleValue;
    SimpleStruct                       m_structValue;
};

TEST( SignalTest, NullTest )
{
    TestObserver observer;
    TestEmitter  emitter;
    observer.connectAllSignals( &emitter );

    ASSERT_EQ( false, observer.receivedBasicSignal() );
    ASSERT_EQ( false, observer.boolValue() );
    ASSERT_EQ( "", observer.stringValue() );
    ASSERT_EQ( false, std::get<0>( observer.tupleValue() ) );
    ASSERT_EQ( "", std::get<1>( observer.tupleValue() ) );
    ASSERT_EQ( 0, std::get<2>( observer.tupleValue() ) );

    SimpleStruct testStruct( 1, 2.3, "a simple struct" );
    ASSERT_NE( testStruct, observer.structValue() );
}

TEST( SignalTest, MessageTest )
{
    TestObserver observer;
    TestEmitter  emitter;
    observer.connectAllSignals( &emitter );

    emitter.triggerBasicSignal();
    ASSERT_EQ( true, observer.receivedBasicSignal() );

    emitter.setBoolValue( true );
    ASSERT_EQ( true, observer.boolValue() );

    emitter.setStringValue( "signals are cool" );
    ASSERT_EQ( "signals are cool", observer.stringValue() );

    std::tuple<bool, std::string, int> tuple( true, "this is a tuple", 42 );
    ASSERT_NE( tuple, observer.tupleValue() );

    emitter.setTupleValue( tuple );
    ASSERT_EQ( tuple, observer.tupleValue() );

    SimpleStruct testStruct( 1, 2.3, "a simple struct" );
    emitter.setSimpleStruct( testStruct );
    ASSERT_EQ( testStruct, observer.structValue() );
}

TEST( SignalTest, ObserverDeletion )
{
    TestEmitter emitter;
    ASSERT_EQ( (size_t)0, emitter.basicSignal.observerCount() );
    {
        TestObserver observer;
        observer.connectAllSignals( &emitter );
        ASSERT_EQ( (size_t)1, emitter.basicSignal.observerCount() );
        emitter.triggerBasicSignal();
        ASSERT_EQ( true, observer.receivedBasicSignal() );
    }
    ASSERT_EQ( (size_t)0, emitter.basicSignal.observerCount() );
    emitter.triggerBasicSignal();
}
