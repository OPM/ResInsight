
#include "gtest/gtest.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"
#include "cafPdmUiTreeView.h"

#include <QApplication>
#include <QModelIndex>

using namespace caf;

class SimpleObj : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    SimpleObj()
        : PdmObject()
    {
        CAF_PDM_InitObject( "SimpleObj", "", "Tooltip SimpleObj", "WhatsThis SimpleObj" );
    }
    ~SimpleObj() {}
};
CAF_PDM_SOURCE_INIT( SimpleObj, "SimpleObj" );

class DemoPdmObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    DemoPdmObject()
    {
        CAF_PDM_InitObject( "DemoPdmObject", "", "Tooltip DemoPdmObject", "WhatsThis DemoPdmObject" );

        CAF_PDM_InitFieldNoDefault( &m_simpleObjPtrField, "SimpleObjPtrField", "SimpleObjPtrField", "", "Tooltip", "WhatsThis" );
    }

    ~DemoPdmObject() { m_simpleObjPtrField.deleteAllChildObjects(); }

    caf::PdmChildArrayField<caf::PdmObjectHandle*> m_simpleObjPtrField;
};

CAF_PDM_SOURCE_INIT( DemoPdmObject, "DemoPdmObject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeViewModelTest, DeleteOneItemAndVerifyTreeOrdering )
{
    SimpleObj* obj1 = new SimpleObj;
    SimpleObj* obj2 = new SimpleObj;
    SimpleObj* obj3 = new SimpleObj;
    SimpleObj* obj4 = new SimpleObj;

    DemoPdmObject* demoObj = new DemoPdmObject;
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );
    demoObj->m_simpleObjPtrField.push_back( obj3 );
    demoObj->m_simpleObjPtrField.push_back( obj4 );

    PdmUiTreeView treeView;
    treeView.setPdmItem( demoObj );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj1 );
    EXPECT_TRUE( mi.isValid() );

    demoObj->m_simpleObjPtrField.removeChildObject( obj1 );
    demoObj->m_simpleObjPtrField().uiCapability()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj1 );
    EXPECT_FALSE( mi.isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeViewModelTest, AddOneItemAndVerifyTreeOrdering )
{
    SimpleObj* obj1 = new SimpleObj;
    SimpleObj* obj2 = new SimpleObj;
    SimpleObj* obj3 = new SimpleObj;
    SimpleObj* obj4 = new SimpleObj;

    DemoPdmObject* demoObj = new DemoPdmObject;
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );
    demoObj->m_simpleObjPtrField.push_back( obj3 );

    PdmUiTreeView treeView;
    treeView.setPdmItem( demoObj );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj4 );
    EXPECT_FALSE( mi.isValid() );

    demoObj->m_simpleObjPtrField.push_back( obj4 );
    demoObj->m_simpleObjPtrField().uiCapability()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj4 );
    EXPECT_TRUE( mi.isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeViewModelTest, ChangeOrderingAndVerifyTreeOrdering )
{
    SimpleObj* obj1 = new SimpleObj;
    SimpleObj* obj2 = new SimpleObj;
    SimpleObj* obj3 = new SimpleObj;
    SimpleObj* obj4 = new SimpleObj;

    DemoPdmObject* demoObj = new DemoPdmObject;
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );
    demoObj->m_simpleObjPtrField.push_back( obj3 );
    demoObj->m_simpleObjPtrField.push_back( obj4 );

    PdmUiTreeView treeView;
    treeView.setPdmItem( demoObj );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj4 );
    EXPECT_EQ( 3, mi.row() );

    demoObj->m_simpleObjPtrField.clear();
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj4 );
    demoObj->m_simpleObjPtrField.push_back( obj3 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );

    demoObj->m_simpleObjPtrField().uiCapability()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj4 );
    EXPECT_EQ( 1, mi.row() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeViewModelTest, ChangeDeepInTreeNotifyRootAndVerifyTreeOrdering )
{
    DemoPdmObject* root = new DemoPdmObject;

    SimpleObj* rootObj1 = new SimpleObj;
    root->m_simpleObjPtrField.push_back( rootObj1 );

    DemoPdmObject* demoObj = new DemoPdmObject;
    root->m_simpleObjPtrField.push_back( demoObj );

    SimpleObj* obj1 = new SimpleObj;
    SimpleObj* obj2 = new SimpleObj;
    SimpleObj* obj3 = new SimpleObj;
    SimpleObj* obj4 = new SimpleObj;
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );
    demoObj->m_simpleObjPtrField.push_back( obj3 );
    demoObj->m_simpleObjPtrField.push_back( obj4 );

    PdmUiTreeView treeView;
    treeView.setPdmItem( root );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj4 );
    EXPECT_EQ( 3, mi.row() );

    demoObj->m_simpleObjPtrField.removeChildObject( obj4 );

    root->m_simpleObjPtrField().uiCapability()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj4 );
    EXPECT_FALSE( mi.isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeViewModelTest, DISABLED_PerformanceLargeNumberOfItems )
{
    // int objCount = 20000;
    int objCount = 100000;

    DemoPdmObject* demoObj = new DemoPdmObject;
    for ( int i = 0; i < objCount; i++ )
    {
        demoObj->m_simpleObjPtrField.push_back( new SimpleObj );
    }

    PdmUiTreeView treeView;
    treeView.setPdmItem( demoObj );
    demoObj->m_simpleObjPtrField().uiCapability()->updateConnectedEditors();
}
