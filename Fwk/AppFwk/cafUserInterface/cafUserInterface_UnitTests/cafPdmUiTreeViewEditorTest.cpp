
#include "gtest/gtest.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"
#include "cafPdmUiTreeViewEditor.h"

#include <QModelIndex>
#include <QTreeView>
#include <QWidget>

using namespace caf;

namespace PdmUiTreeViewEditorTestNs
{
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
// Registered under a unique keyword: the object factory keys on this string regardless of C++
// namespace, and cafPdmUiTreeViewModelTest.cpp already registers "SimpleObj"/"DemoPdmObject".
CAF_PDM_SOURCE_INIT( SimpleObj, "TreeViewEditorTest_SimpleObj" );

class DemoPdmObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    DemoPdmObject()
    {
        CAF_PDM_InitObject( "DemoPdmObject", "", "Tooltip DemoPdmObject", "WhatsThis DemoPdmObject" );

        CAF_PDM_InitFieldNoDefault( &m_simpleObjPtrField, "SimpleObjPtrField", "SimpleObjPtrField", "", "Tooltip", "WhatsThis" );
        m_simpleObjPtrField.uiCapability()->setUiTreeHidden( false );
    }

    caf::PdmChildArrayField<caf::PdmObjectHandle*> m_simpleObjPtrField;
};

CAF_PDM_SOURCE_INIT( DemoPdmObject, "TreeViewEditorTest_DemoPdmObject" );
} // namespace PdmUiTreeViewEditorTestNs

using namespace PdmUiTreeViewEditorTestNs;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeViewEditorTest, CreateWidgetAndVerifyEmptyState )
{
    PdmUiTreeViewEditor editor;

    QWidget* widget = editor.getOrCreateWidget( nullptr );
    ASSERT_TRUE( widget != nullptr );
    ASSERT_TRUE( editor.treeView() != nullptr );

    // No item is bound yet, so no valid model index should be found
    SimpleObj   obj;
    QModelIndex mi = editor.findModelIndex( &obj );
    EXPECT_FALSE( mi.isValid() );

    delete widget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeViewEditorTest, SetPdmItemRootAndFindModelIndex )
{
    SimpleObj* obj1 = new SimpleObj;
    SimpleObj* obj2 = new SimpleObj;

    DemoPdmObject* demoObj = new DemoPdmObject;
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );

    PdmUiTreeViewEditor editor;
    editor.getOrCreateWidget( nullptr );

    editor.setPdmItemRoot( demoObj );
    editor.updateUi( "" );

    QModelIndex mi1 = editor.findModelIndex( obj1 );
    EXPECT_TRUE( mi1.isValid() );

    QModelIndex mi2 = editor.findModelIndex( obj2 );
    EXPECT_TRUE( mi2.isValid() );

    EXPECT_EQ( obj1, dynamic_cast<PdmObjectHandle*>( editor.uiItemFromModelIndex( mi1 ) ) );
    EXPECT_EQ( obj2, dynamic_cast<PdmObjectHandle*>( editor.uiItemFromModelIndex( mi2 ) ) );

    delete demoObj;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeViewEditorTest, DeleteItemAndVerifyModelIndexInvalidated )
{
    SimpleObj* obj1 = new SimpleObj;
    SimpleObj* obj2 = new SimpleObj;

    DemoPdmObject* demoObj = new DemoPdmObject;
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );

    PdmUiTreeViewEditor editor;
    editor.getOrCreateWidget( nullptr );
    editor.setPdmItemRoot( demoObj );
    editor.updateUi( "" );

    QModelIndex mi = editor.findModelIndex( obj1 );
    EXPECT_TRUE( mi.isValid() );

    demoObj->m_simpleObjPtrField.removeChild( obj1 );
    demoObj->m_simpleObjPtrField().uiCapability()->updateConnectedEditors();

    mi = editor.findModelIndex( obj1 );
    EXPECT_FALSE( mi.isValid() );

    delete obj1;
    delete demoObj;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeViewEditorTest, SelectAsCurrentItemAndVerifySelection )
{
    SimpleObj* obj1 = new SimpleObj;
    SimpleObj* obj2 = new SimpleObj;

    DemoPdmObject* demoObj = new DemoPdmObject;
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );

    PdmUiTreeViewEditor editor;
    editor.getOrCreateWidget( nullptr );
    editor.setPdmItemRoot( demoObj );
    editor.updateUi( "" );

    editor.selectAsCurrentItem( obj2 );

    std::vector<PdmUiItem*> selectedItems;
    editor.selectedUiItems( selectedItems );

    ASSERT_EQ( size_t( 1 ), selectedItems.size() );
    EXPECT_EQ( obj2, dynamic_cast<PdmObjectHandle*>( selectedItems[0] ) );

    delete demoObj;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeViewEditorTest, SetExpandedDoesNotCrashForValidAndInvalidItems )
{
    SimpleObj* obj1 = new SimpleObj;

    DemoPdmObject* demoObj = new DemoPdmObject;
    demoObj->m_simpleObjPtrField.push_back( obj1 );

    PdmUiTreeViewEditor editor;
    editor.getOrCreateWidget( nullptr );
    editor.setPdmItemRoot( demoObj );
    editor.updateUi( "" );

    // Should not crash for a valid item
    editor.setExpanded( demoObj, true );
    editor.setExpanded( demoObj, false );

    // Should not crash for an item not present in the tree
    SimpleObj detachedObj;
    editor.setExpanded( &detachedObj, true );

    delete demoObj;
}

//--------------------------------------------------------------------------------------------------
/// The tree view widget can be destroyed by Qt while the editor is still reachable, for instance
/// during teardown of the parent widget. All accessors must then be no-ops instead of crashing.
/// https://github.com/OPM/ResInsight/issues/14699
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeViewEditorTest, AccessorsAreNoOpsAfterWidgetIsDestroyed )
{
    SimpleObj* obj1 = new SimpleObj;

    DemoPdmObject* demoObj = new DemoPdmObject;
    demoObj->m_simpleObjPtrField.push_back( obj1 );

    PdmUiTreeViewEditor editor;
    QWidget*            widget = editor.getOrCreateWidget( nullptr );
    editor.setPdmItemRoot( demoObj );
    editor.updateUi( "" );

    EXPECT_TRUE( editor.findModelIndex( obj1 ).isValid() );

    delete widget;
    EXPECT_TRUE( editor.treeView() == nullptr );

    // The models outlive the widget, so these must be no-ops rather than crashes
    editor.selectAsCurrentItem( obj1 );
    editor.selectItems( { obj1 } );
    editor.setExpanded( obj1, true );

    delete demoObj;
}
