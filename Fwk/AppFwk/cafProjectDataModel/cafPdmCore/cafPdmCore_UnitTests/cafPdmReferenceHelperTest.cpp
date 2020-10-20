
#include "gtest/gtest.h"

#include "cafAppEnum.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmDataValueField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cafPdmReferenceHelper.h"

class SimpleObj : public caf::PdmObjectHandle
{
public:
    SimpleObj()
        : PdmObjectHandle()
        , m_doubleMember( 0.0 )
    {
        this->addField( &m_position, "m_position" );
        this->addField( &m_dir, "m_dir" );
        this->addField( &m_up, "m_up" );
        this->addField( &m_proxyDouble, "m_proxyDouble" );
    }

    caf::PdmDataValueField<double>  m_position;
    caf::PdmDataValueField<double>  m_dir;
    caf::PdmDataValueField<double>  m_up;
    caf::PdmProxyValueField<double> m_proxyDouble;

    void setDoubleMember( const double& d )
    {
        m_doubleMember = d;
        std::cout << "setDoubleMember" << std::endl;
    }
    double doubleMember() const
    {
        std::cout << "doubleMember" << std::endl;
        return m_doubleMember;
    }

    double m_doubleMember;
};

class ReferenceSimpleObj : public caf::PdmObjectHandle
{
public:
    ReferenceSimpleObj()
        : PdmObjectHandle()
    {
        this->addField( &m_pointersField, "m_pointersField" );
        this->addField( &m_simpleObjPtrField, "m_simpleObjPtrField" );
    }

    ~ReferenceSimpleObj()
    {
        delete m_pointersField();
        m_simpleObjPtrField.deleteAllChildObjects();
    }

    // Fields
    caf::PdmChildField<PdmObjectHandle*> m_pointersField;
    caf::PdmChildArrayField<SimpleObj*>  m_simpleObjPtrField;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmReferenceHelperTest, FindRootFromObject )
{
    {
        caf::PdmObjectHandle* obj = NULL;
        EXPECT_EQ( NULL, caf::PdmReferenceHelper::findRoot( obj ) );
    }

    {
        caf::PdmObjectHandle* obj = new SimpleObj;
        EXPECT_EQ( obj, caf::PdmReferenceHelper::findRoot( obj ) );
        delete obj;
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );

        EXPECT_EQ( ihd1, caf::PdmReferenceHelper::findRoot( s1 ) );
        delete ihd1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmReferenceHelperTest, FindRootFromField )
{
    {
        caf::PdmFieldHandle* fieldHandle = NULL;
        EXPECT_EQ( NULL, caf::PdmReferenceHelper::findRoot( fieldHandle ) );
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );

        EXPECT_EQ( ihd1, caf::PdmReferenceHelper::findRoot( &s1->m_dir ) );
        delete ihd1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmReferenceHelperTest, ReferenceFrommRootToField )
{
    {
        caf::PdmObjectHandle* obj         = NULL;
        caf::PdmFieldHandle*  fieldHandle = NULL;
        EXPECT_TRUE( caf::PdmReferenceHelper::referenceFromRootToField( obj, fieldHandle ).isEmpty() );
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        SimpleObj*          s2   = new SimpleObj;
        SimpleObj*          s3   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );
        ihd1->m_simpleObjPtrField.push_back( s2 );
        ihd1->m_simpleObjPtrField.push_back( s3 );

        EXPECT_TRUE( caf::PdmReferenceHelper::referenceFromRootToField( NULL, &s3->m_dir ).isEmpty() );
        EXPECT_TRUE( caf::PdmReferenceHelper::referenceFromRootToField( ihd1, NULL ).isEmpty() );

        QString refString      = caf::PdmReferenceHelper::referenceFromRootToField( ihd1, &s3->m_dir );
        QString expectedString = "m_dir m_simpleObjPtrField 2";
        EXPECT_STREQ( expectedString.toLatin1(), refString.toLatin1() );

        delete ihd1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmReferenceHelperTest, ReferenceFrommRootToObject )
{
    {
        caf::PdmObjectHandle* root = NULL;
        caf::PdmObjectHandle* obj  = NULL;
        EXPECT_TRUE( caf::PdmReferenceHelper::referenceFromRootToObject( root, obj ).isEmpty() );
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        SimpleObj*          s2   = new SimpleObj;
        SimpleObj*          s3   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );
        ihd1->m_simpleObjPtrField.push_back( s2 );
        ihd1->m_simpleObjPtrField.push_back( s3 );

        EXPECT_TRUE( caf::PdmReferenceHelper::referenceFromRootToObject( NULL, s3 ).isEmpty() );
        EXPECT_TRUE( caf::PdmReferenceHelper::referenceFromRootToObject( ihd1, NULL ).isEmpty() );

        QString refString      = caf::PdmReferenceHelper::referenceFromRootToObject( ihd1, s3 );
        QString expectedString = "m_simpleObjPtrField 2";
        EXPECT_STREQ( expectedString.toLatin1(), refString.toLatin1() );

        ReferenceSimpleObj* ihd2 = new ReferenceSimpleObj;
        SimpleObj*          s4   = new SimpleObj;
        ihd2->m_simpleObjPtrField.push_back( s4 );

        EXPECT_TRUE( caf::PdmReferenceHelper::referenceFromRootToObject( ihd1, s4 ).isEmpty() );

        delete ihd1;
        delete ihd2;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmReferenceHelperTest, ObjectFromReference )
{
    {
        caf::PdmObjectHandle* root = NULL;
        EXPECT_EQ( NULL, caf::PdmReferenceHelper::objectFromReference( root, "" ) );
        EXPECT_EQ( NULL, caf::PdmReferenceHelper::objectFromReference( root, "a 2 b 4" ) );
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        SimpleObj*          s2   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );
        ihd1->m_simpleObjPtrField.push_back( s2 );

        EXPECT_EQ( NULL, caf::PdmReferenceHelper::objectFromReference( ihd1, "" ) );
        EXPECT_EQ( NULL, caf::PdmReferenceHelper::objectFromReference( ihd1, "a 2 b 4" ) );

        QString refString = caf::PdmReferenceHelper::referenceFromRootToObject( ihd1, s2 );
        EXPECT_EQ( s2, caf::PdmReferenceHelper::objectFromReference( ihd1, refString ) );

        ihd1->m_simpleObjPtrField.removeChildObject( s2 );
        EXPECT_EQ( NULL, caf::PdmReferenceHelper::objectFromReference( ihd1, refString ) );

        ihd1->m_simpleObjPtrField.deleteAllChildObjects();

        EXPECT_EQ( NULL, caf::PdmReferenceHelper::objectFromReference( ihd1, refString ) );

        delete s2;
        delete ihd1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmReferenceHelperTest, FieldFromReference )
{
    {
        caf::PdmObjectHandle* root = NULL;
        EXPECT_EQ( NULL, caf::PdmReferenceHelper::fieldFromReference( root, "" ) );
        EXPECT_EQ( NULL, caf::PdmReferenceHelper::fieldFromReference( root, "a 2 b 4" ) );
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        SimpleObj*          s2   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );
        ihd1->m_simpleObjPtrField.push_back( s2 );

        EXPECT_EQ( NULL, caf::PdmReferenceHelper::fieldFromReference( ihd1, "" ) );
        EXPECT_EQ( NULL, caf::PdmReferenceHelper::fieldFromReference( ihd1, "a 2 b 4" ) );

        caf::PdmFieldHandle* fHandle = &s2->m_position;

        QString refString = caf::PdmReferenceHelper::referenceFromRootToField( ihd1, fHandle );
        EXPECT_EQ( fHandle, caf::PdmReferenceHelper::fieldFromReference( ihd1, refString ) );

        ihd1->m_simpleObjPtrField.removeChildObject( s2 );
        EXPECT_EQ( NULL, caf::PdmReferenceHelper::fieldFromReference( ihd1, refString ) );

        ihd1->m_simpleObjPtrField.deleteAllChildObjects();

        EXPECT_EQ( NULL, caf::PdmReferenceHelper::fieldFromReference( ihd1, refString ) );

        delete s2;
        delete ihd1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmReferenceHelperTest, ReferenceFromFieldToObject )
{
    {
        caf::PdmObjectHandle* root        = NULL;
        caf::PdmFieldHandle*  fieldHandle = NULL;

        EXPECT_TRUE( caf::PdmReferenceHelper::referenceFromFieldToObject( fieldHandle, root ).isEmpty() );
    }

    {
        SimpleObj* s1 = new SimpleObj;
        SimpleObj* s2 = new SimpleObj;

        caf::PdmFieldHandle* s2FieldHandle = &s2->m_dir;

        // Unrelated objects
        EXPECT_TRUE( caf::PdmReferenceHelper::referenceFromFieldToObject( s2FieldHandle, s1 ).isEmpty() );

        ReferenceSimpleObj* root    = new ReferenceSimpleObj;
        SimpleObj*          root_s1 = new SimpleObj;
        root->m_simpleObjPtrField.push_back( root_s1 );

        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        root->m_pointersField    = ihd1;

        ihd1->m_simpleObjPtrField.push_back( s1 );
        ihd1->m_simpleObjPtrField.push_back( s2 );

        QString               refString = caf::PdmReferenceHelper::referenceFromFieldToObject( s2FieldHandle, root_s1 );
        caf::PdmObjectHandle* obj       = caf::PdmReferenceHelper::objectFromFieldReference( s2FieldHandle, refString );
        EXPECT_EQ( root_s1, obj );

        delete root;
    }
}
