
#include "gtest/gtest.h"

#include "Parent.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmDataValueField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmValueField.h"

#include <vector>

class DemoPdmObject : public caf::PdmObjectHandle
{
public:
    DemoPdmObject()
    {
        this->addField( &m_proxyDoubleField, "m_proxyDoubleField" );
        m_proxyDoubleField.registerSetMethod( this, &DemoPdmObject::setDoubleMember );
        m_proxyDoubleField.registerGetMethod( this, &DemoPdmObject::doubleMember );

        this->addField( &m_proxyIntField, "m_proxyIntField" );
        m_proxyIntField.registerSetMethod( this, &DemoPdmObject::setIntMember );
        m_proxyIntField.registerGetMethod( this, &DemoPdmObject::intMember );

        this->addField( &m_proxyStringField, "m_proxyStringField" );
        m_proxyStringField.registerSetMethod( this, &DemoPdmObject::setStringMember );
        m_proxyStringField.registerGetMethod( this, &DemoPdmObject::stringMember );

        this->addField( &m_memberDoubleField, "m_memberDoubleField" );
        this->addField( &m_memberIntField, "m_memberIntField" );
        this->addField( &m_memberStringField, "m_memberStringField" );

        // Default values
        m_doubleMember = 2.1;
        m_intMember    = 7;
        m_stringMember = "abba";

        m_memberDoubleField = 0.0;
        m_memberIntField    = 0;
        m_memberStringField = "";
    }

    ~DemoPdmObject() {}

    // Fields
    caf::PdmProxyValueField<double>  m_proxyDoubleField;
    caf::PdmProxyValueField<int>     m_proxyIntField;
    caf::PdmProxyValueField<QString> m_proxyStringField;

    caf::PdmDataValueField<double>  m_memberDoubleField;
    caf::PdmDataValueField<int>     m_memberIntField;
    caf::PdmDataValueField<QString> m_memberStringField;

    // Internal class members accessed by proxy fields
    double doubleMember() const
    {
        std::cout << "doubleMember" << std::endl;
        return m_doubleMember;
    }
    void setDoubleMember( const double& d )
    {
        m_doubleMember = d;
        std::cout << "setDoubleMember" << std::endl;
    }

    int  intMember() const { return m_intMember; }
    void setIntMember( const int& val ) { m_intMember = val; }

    QString stringMember() const { return m_stringMember; }
    void    setStringMember( const QString& val ) { m_stringMember = val; }

private:
    double  m_doubleMember;
    int     m_intMember;
    QString m_stringMember;
};

class InheritedDemoObj : public DemoPdmObject
{
public:
    InheritedDemoObj()
    {
        this->addField( &m_texts, "Texts" );
        this->addField( &m_childArrayField, "DemoPdmObjectects" );
        this->addField( &m_ptrField, "m_ptrField" );

        this->addField( &m_singleFilePath, "m_singleFilePath" );
        this->addField( &m_multipleFilePath, "m_multipleFilePath" );
    }

    caf::PdmDataValueField<QString>         m_texts;
    caf::PdmChildArrayField<DemoPdmObject*> m_childArrayField;
    caf::PdmPtrField<InheritedDemoObj*>     m_ptrField;

    caf::PdmDataValueField<caf::FilePath>              m_singleFilePath;
    caf::PdmDataValueField<std::vector<caf::FilePath>> m_multipleFilePath;
};

TEST( BaseTest, Delete )
{
    DemoPdmObject* s2 = new DemoPdmObject;
    delete s2;
}

//--------------------------------------------------------------------------------------------------
/// TestPdmDataValueField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestPdmDataValueField )
{
    DemoPdmObject* a = new DemoPdmObject;

    ASSERT_DOUBLE_EQ( 0.0, a->m_memberDoubleField.value() );
    a->m_memberDoubleField.setValue( 1.2 );
    ASSERT_DOUBLE_EQ( 1.2, a->m_memberDoubleField.value() );

    ASSERT_EQ( 0, a->m_memberIntField.value() );
    a->m_memberIntField.setValue( 11 );
    ASSERT_EQ( 11, a->m_memberIntField.value() );

    ASSERT_TRUE( a->m_memberStringField.value().isEmpty() );
    a->m_memberStringField.setValue( "123" );
    ASSERT_TRUE( a->m_memberStringField.value() == "123" );
}

//--------------------------------------------------------------------------------------------------
/// TestPdmProxyValueField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestPdmProxyValueField )
{
    DemoPdmObject* a = new DemoPdmObject;

    ASSERT_DOUBLE_EQ( 2.1, a->m_proxyDoubleField.value() );
    a->m_proxyDoubleField.setValue( 1.2 );
    ASSERT_DOUBLE_EQ( 1.2, a->m_proxyDoubleField.value() );

    ASSERT_EQ( 7, a->m_proxyIntField.value() );
    a->m_proxyIntField.setValue( 11 );
    ASSERT_EQ( 11, a->m_proxyIntField.value() );

    ASSERT_TRUE( a->m_proxyStringField.value() == "abba" );
    a->m_proxyStringField.setValue( "123" );
    ASSERT_TRUE( a->m_proxyStringField.value() == "123" );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestPdmValueFieldInterface )
{
    DemoPdmObject* a = new DemoPdmObject;

    {
        caf::PdmValueField* valField = dynamic_cast<caf::PdmValueField*>( a->findField( "m_proxyDoubleField" ) );
        QVariant            newVal   = 3.4;
        valField->setFromQVariant( newVal );
        QVariant var = valField->toQVariant();
        ASSERT_TRUE( newVal == var );
    }

    {
        caf::PdmValueField* valField = dynamic_cast<caf::PdmValueField*>( a->findField( "m_proxyIntField" ) );
        QVariant            newVal   = 3;
        valField->setFromQVariant( newVal );
        QVariant var = valField->toQVariant();
        ASSERT_TRUE( newVal == var );
    }

    {
        caf::PdmValueField* valField = dynamic_cast<caf::PdmValueField*>( a->findField( "m_proxyStringField" ) );
        QVariant            newVal   = "test";
        valField->setFromQVariant( newVal );
        QVariant var = valField->toQVariant();
        ASSERT_TRUE( newVal == var );
    }

    {
        caf::PdmValueField* valField = dynamic_cast<caf::PdmValueField*>( a->findField( "m_memberDoubleField" ) );
        QVariant            newVal   = 3.4;
        valField->setFromQVariant( newVal );
        QVariant var = valField->toQVariant();
        ASSERT_TRUE( newVal == var );
    }

    {
        caf::PdmValueField* valField = dynamic_cast<caf::PdmValueField*>( a->findField( "m_memberIntField" ) );
        QVariant            newVal   = 3;
        valField->setFromQVariant( newVal );
        QVariant var = valField->toQVariant();
        ASSERT_TRUE( newVal == var );
    }

    {
        caf::PdmValueField* valField = dynamic_cast<caf::PdmValueField*>( a->findField( "m_memberStringField" ) );
        QVariant            newVal   = "test";
        valField->setFromQVariant( newVal );
        QVariant var = valField->toQVariant();
        ASSERT_TRUE( newVal == var );
    }
}

//--------------------------------------------------------------------------------------------------
/// Test of PdmDataValueField operations
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, NormalPdmField )
{
    class A : public caf::PdmObjectHandle
    {
    public:
        explicit A( const std::vector<double>& testValue )
            : field2( testValue )
            , field3( field2 )
        {
            this->addField( &field1, "field1" );
            this->addField( &field2, "field2" );
            this->addField( &field3, "field3" );
        }

        caf::PdmDataValueField<std::vector<double>> field1;
        caf::PdmDataValueField<std::vector<double>> field2;
        caf::PdmDataValueField<std::vector<double>> field3;
    };

    std::vector<double> testValue;
    testValue.push_back( 1.1 );
    testValue.push_back( 1.2 );
    testValue.push_back( 1.3 );

    std::vector<double> testValue2;
    testValue2.push_back( 2.1 );
    testValue2.push_back( 2.2 );
    testValue2.push_back( 2.3 );

    // Constructors

    A a( testValue );

    EXPECT_EQ( 1.3, a.field2.v()[2] );
    EXPECT_EQ( 1.3, a.field3.v()[2] );
    EXPECT_EQ( size_t( 0 ), a.field1().size() );

    // Operators
    // ==
    EXPECT_FALSE( a.field1 == a.field3 );
    // = field to field
    a.field1 = a.field2;
    // ()
    EXPECT_EQ( 1.3, a.field1()[2] );
    // = value to field
    a.field1 = testValue2;
    // v()
    EXPECT_EQ( 2.3, a.field1.v()[2] );
    // ==
    a.field3 = a.field1;
    EXPECT_TRUE( a.field1 == a.field3 );
}

//--------------------------------------------------------------------------------------------------
/// Test of PdmChildArrayField operations
//--------------------------------------------------------------------------------------------------

TEST( BaseTest, PdmChildArrayField )
{
    InheritedDemoObj* ihd1 = new InheritedDemoObj;

    caf::PdmPointer<DemoPdmObject> s1 = new DemoPdmObject;
    caf::PdmPointer<DemoPdmObject> s2 = new DemoPdmObject;
    caf::PdmPointer<DemoPdmObject> s3 = new DemoPdmObject;

    // empty() number 1
    EXPECT_TRUE( ihd1->m_childArrayField.empty() );
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );

    // push_back()
    ihd1->m_childArrayField.push_back( s1 );
    ihd1->m_childArrayField.push_back( s2 );
    ihd1->m_childArrayField.push_back( s3 );

    // Parent field
    EXPECT_EQ( s1->parentField(), &( ihd1->m_childArrayField ) );

    // size()
    EXPECT_EQ( size_t( 3 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( size_t( 3 ), ihd1->m_childArrayField.size() );

    // operator[]
    EXPECT_EQ( s2, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s3, ihd1->m_childArrayField[2] );

    // childObjects
    std::vector<caf::PdmObjectHandle*> objects;
    ihd1->m_childArrayField.childObjects( &objects );
    EXPECT_EQ( size_t( 3 ), objects.size() );

    std::vector<DemoPdmObject*> typedObjects = ihd1->m_childArrayField.childObjects();
    EXPECT_EQ( size_t( 3 ), typedObjects.size() );

    // set()
    ihd1->m_childArrayField.set( 1, NULL );
    EXPECT_TRUE( NULL == ihd1->m_childArrayField[1] );
    EXPECT_TRUE( s2->parentField() == NULL );

    ihd1->m_childArrayField.removeChildObject( NULL );
    EXPECT_EQ( size_t( 2 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( s3, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s1, ihd1->m_childArrayField[0] );

    // insert()
    ihd1->m_childArrayField.insert( 1, s2 );
    EXPECT_EQ( s1, ihd1->m_childArrayField[0] );
    EXPECT_EQ( s2, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s3, ihd1->m_childArrayField[2] );

    EXPECT_TRUE( s2->parentField() == &( ihd1->m_childArrayField ) );

    // erase (index)
    ihd1->m_childArrayField.erase( 1 );
    EXPECT_EQ( size_t( 2 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( s3, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s1, ihd1->m_childArrayField[0] );

    EXPECT_TRUE( s2->parentField() == NULL );

    // clear()
    ihd1->m_childArrayField.clear();
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );

    EXPECT_TRUE( s1->parentField() == NULL );

    ihd1->m_childArrayField.push_back( s1 );
    ihd1->m_childArrayField.push_back( s2 );
    ihd1->m_childArrayField.push_back( s3 );

    ihd1->m_childArrayField.deleteAllChildObjects();
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );
    EXPECT_TRUE( s1 == NULL );
    EXPECT_TRUE( s2 == NULL );
    EXPECT_TRUE( s3 == NULL );
}

TEST( BaseTest, PdmChildArrayParentField )
{
    // Test of instanciating a class with forward declare of object used in PdmChildArrayField and PdmChildField
    Parent* parentObj = new Parent;

    delete parentObj;
}

#include "Child.h"

TEST( BaseTest, PdmPointersFieldInsertVector )
{
    Parent* ihd1 = new Parent;

    Child* s1 = new Child;
    Child* s2 = new Child;
    Child* s3 = new Child;

    std::vector<caf::PdmPointer<Child>> typedObjects;
    typedObjects.push_back( s1 );
    typedObjects.push_back( s2 );
    typedObjects.push_back( s3 );

    ihd1->m_simpleObjectsField.push_back( new Child );
    ihd1->m_simpleObjectsField.insert( ihd1->m_simpleObjectsField.size(), typedObjects );
    EXPECT_EQ( size_t( 4 ), ihd1->m_simpleObjectsField.size() );
    EXPECT_EQ( ihd1->m_simpleObjectsField[3], s3 );

    delete ihd1;
}

//--------------------------------------------------------------------------------------------------
/// PdmChildArrayFieldHandle
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, PdmChildArrayFieldHandle )
{
    //     virtual size_t      size() const = 0;
    //     virtual bool        empty() const = 0;
    //     virtual void        clear() = 0;
    //     virtual PdmObject*  createAppendObject(int indexAfter) = 0;
    //     virtual void        erase(size_t index) = 0;
    //     virtual void        deleteAllChildObjects() = 0;
    //
    //     virtual PdmObject*  at(size_t index) = 0;
    //
    //     bool                hasSameFieldCountForAllObjects();
    DemoPdmObject* s0       = new DemoPdmObject;
    s0->m_memberDoubleField = 1000;

    DemoPdmObject* s1       = new DemoPdmObject;
    s1->m_memberDoubleField = 1000;

    DemoPdmObject* s2       = new DemoPdmObject;
    s2->m_memberDoubleField = 2000;

    DemoPdmObject* s3       = new DemoPdmObject;
    s3->m_memberDoubleField = 3000;

    InheritedDemoObj*              ihd1      = new InheritedDemoObj;
    caf::PdmChildArrayFieldHandle* listField = &( ihd1->m_childArrayField );

    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_TRUE( listField->empty() );

    listField->insertAt( 0, s0 );
    EXPECT_EQ( 1u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    ihd1->m_childArrayField.push_back( s1 );
    ihd1->m_childArrayField.push_back( s2 );
    ihd1->m_childArrayField.push_back( s3 );

    EXPECT_EQ( 4u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    listField->erase( 0 );
    EXPECT_EQ( 3u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    listField->deleteAllChildObjects();
    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_TRUE( listField->empty() );
}
//--------------------------------------------------------------------------------------------------
/// Test of PdmChildField
//--------------------------------------------------------------------------------------------------

TEST( BaseTest, PdmChildField )
{
    class A : public caf::PdmObjectHandle
    {
    public:
        explicit A( Child* a )
            : field2( a )
            , b( 0 )
        {
            this->addField( &field2, "field2" );
        }

        ~A() { delete field2(); }

        caf::PdmChildField<Child*> field2;
        int                        b;
    };

    {
        Child* testValue = new Child;

        // Constructor assignment
        A a( testValue );
        EXPECT_EQ( testValue, a.field2.v() );

        // Guarded
        delete testValue;
        EXPECT_EQ( static_cast<Child*>( nullptr ), a.field2 );
    }
    {
        A      a( NULL );
        Child* c2 = new Child;
        // Assign
        a.field2 = c2;
        // Access
        EXPECT_EQ( c2, a.field2.v() );
        EXPECT_EQ( c2, a.field2 );
        EXPECT_EQ( c2, a.field2.value() );
        EXPECT_TRUE( c2 == a.field2 );

        std::vector<caf::PdmObjectHandle*> objects;
        a.field2.childObjects( &objects );
        EXPECT_EQ( (size_t)1, objects.size() );
        EXPECT_EQ( c2, objects[0] );
    }
}

TEST( BaseTest, PdmPtrField )
{
    InheritedDemoObj* ihd1 = new InheritedDemoObj;
    InheritedDemoObj* ihd2 = new InheritedDemoObj;

    // Direct access
    EXPECT_EQ( static_cast<InheritedDemoObj*>( nullptr ), ihd1->m_ptrField );

    // Assignment
    ihd1->m_ptrField              = ihd1;
    InheritedDemoObj* accessedIhd = ihd1->m_ptrField;
    EXPECT_EQ( ihd1, accessedIhd );

    ihd1->m_ptrField = caf::PdmPointer<InheritedDemoObj>( ihd2 );
    accessedIhd      = ihd1->m_ptrField;
    EXPECT_EQ( ihd2, accessedIhd );

    // Access
    accessedIhd = ihd1->m_ptrField; // Conversion
    EXPECT_EQ( ihd2, accessedIhd );
    accessedIhd = ihd1->m_ptrField.value();
    EXPECT_EQ( ihd2, accessedIhd );

    caf::PdmPointer<InheritedDemoObj> accessedPdmPtr;
    EXPECT_EQ( ihd2, accessedIhd );
    accessedPdmPtr = ihd1->m_ptrField();
    EXPECT_EQ( ihd2, accessedPdmPtr.p() );
    accessedPdmPtr = ihd1->m_ptrField();
    EXPECT_EQ( ihd2, accessedPdmPtr.p() );

    // Operator ==
    EXPECT_TRUE( ihd1->m_ptrField == ihd2 );
    EXPECT_FALSE( ihd1->m_ptrField == ihd1 );

    EXPECT_TRUE( ihd1->m_ptrField == caf::PdmPointer<InheritedDemoObj>( ihd2 ) );

    // Generic access
    {
        std::vector<caf::PdmObjectHandle*> objects;
        ihd1->m_ptrField.ptrReferencedObjects( &objects );
        EXPECT_EQ( 1u, objects.size() );
        EXPECT_EQ( ihd2, objects[0] );
    }

    // Operator ->
    ihd1->m_ptrField->m_texts = "Hei PtrField";
    EXPECT_TRUE( ihd1->m_ptrField->m_texts == "Hei PtrField" );

    // Referencing system
    {
        std::vector<caf::PdmFieldHandle*> ptrFields;
        ihd2->referringPtrFields( ptrFields );
        EXPECT_EQ( 1u, ptrFields.size() );
        EXPECT_EQ( &( ihd1->m_ptrField ), ptrFields[0] );
    }

    {
        std::vector<caf::PdmObjectHandle*> objects;
        ihd2->objectsWithReferringPtrFields( objects );
        EXPECT_EQ( 1u, objects.size() );
        EXPECT_EQ( ihd1, objects[0] );
    }

    {
        std::vector<InheritedDemoObj*> reffingDemoObjects;
        ihd2->objectsWithReferringPtrFieldsOfType( reffingDemoObjects );
        EXPECT_EQ( 1u, reffingDemoObjects.size() );
    }

    delete ihd1;
    delete ihd2;
}

//--------------------------------------------------------------------------------------------------
/// Tests the features of PdmPointer
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, PdmPointer )
{
    InheritedDemoObj* d = new InheritedDemoObj;

    {
        caf::PdmPointer<InheritedDemoObj> p;
        EXPECT_TRUE( p == NULL );
    }

    {
        caf::PdmPointer<InheritedDemoObj> p( d );
        caf::PdmPointer<InheritedDemoObj> p2( p );

        EXPECT_TRUE( p == d && p2 == d );
        EXPECT_TRUE( p.p() == d );
        p = 0;
        EXPECT_TRUE( p == NULL );
        EXPECT_TRUE( p.isNull() );
        EXPECT_TRUE( p2 == d );
        p = p2;
        EXPECT_TRUE( p == d );
        delete d;
        EXPECT_TRUE( p.isNull() && p2.isNull() );
    }

    caf::PdmPointer<DemoPdmObject> p3( new DemoPdmObject() );

    delete p3;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, PdmFilePath )
{
    InheritedDemoObj* d = new InheritedDemoObj;

    QVariant newVal = "path with space";
    d->m_singleFilePath.setFromQVariant( newVal );

    QVariant var = d->m_singleFilePath.toQVariant();
    ASSERT_TRUE( newVal == var );

    delete d;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, MultiplePdmFilePath )
{
    InheritedDemoObj* d = new InheritedDemoObj;

    QString newVal = "path with space";
    d->m_multipleFilePath.v().push_back( newVal );
    d->m_multipleFilePath.v().push_back( newVal );

    QVariant    var = d->m_multipleFilePath.toQVariant();
    QStringList str = var.toStringList();

    EXPECT_EQ( 2, str.size() );

    delete d;
}
