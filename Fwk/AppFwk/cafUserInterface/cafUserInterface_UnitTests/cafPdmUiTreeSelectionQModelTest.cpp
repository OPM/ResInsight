
#include "gtest/gtest.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiTreeSelectionQModel.h"

#include <cstring>

QList<caf::PdmOptionItemInfo> createOptions()
{
    QList<caf::PdmOptionItemInfo> options;

    {
        QString text;

        text = "First";
        options.push_back( caf::PdmOptionItemInfo( text, text ) );

        text = "Second";
        options.push_back( caf::PdmOptionItemInfo( text, text ) );

        {
            text                            = "Second_a";
            caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo( text, text );
            itemInfo.setLevel( 1 );
            options.push_back( itemInfo );
        }

        {
            text                            = "Second_b";
            caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo( text, text );
            itemInfo.setLevel( 1 );
            options.push_back( itemInfo );
        }

        text = "Third";
        options.push_back( caf::PdmOptionItemInfo( text, text ) );

        text = "Fourth";
        options.push_back( caf::PdmOptionItemInfo( text, text ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeSelectionQModelTest, BasicUsage )
{
    QList<caf::PdmOptionItemInfo> options = createOptions();

    caf::PdmUiTreeSelectionQModel myModel;
    myModel.setOptions( nullptr, options );

    EXPECT_EQ( options.size(), myModel.optionItemCount() );

    EXPECT_EQ( 4, myModel.rowCount( myModel.index( -1, -1 ) ) );

    EXPECT_EQ( 0, myModel.rowCount( myModel.index( 0, 0 ) ) );
    EXPECT_EQ( 2, myModel.rowCount( myModel.index( 1, 0 ) ) );
    EXPECT_EQ( 0, myModel.rowCount( myModel.index( 2, 0 ) ) );
    EXPECT_EQ( 0, myModel.rowCount( myModel.index( 3, 0 ) ) );

    // Test for row out of bounds
    EXPECT_FALSE( myModel.index( 4, 0 ).isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeSelectionQModelTest, ParentBehaviour )
{
    QList<caf::PdmOptionItemInfo> options = createOptions();

    caf::PdmUiTreeSelectionQModel myModel;
    myModel.setOptions( nullptr, options );

    QModelIndex parentIndex = myModel.index( 1, 0 );
    EXPECT_EQ( 2, myModel.rowCount( parentIndex ) );

    {
        QModelIndex firstChildIndex = myModel.index( 0, 0, parentIndex );
        EXPECT_STREQ( "Second_a", myModel.data( firstChildIndex ).toString().toLatin1() );
        EXPECT_TRUE( parentIndex == myModel.parent( firstChildIndex ) );
    }

    {
        QModelIndex secondChildIndex = myModel.index( 1, 0, parentIndex );
        EXPECT_STREQ( "Second_b", myModel.data( secondChildIndex ).toString().toLatin1() );
        EXPECT_TRUE( parentIndex == myModel.parent( secondChildIndex ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeSelectionQModelTest, SetDataAndSignal )
{
    QList<caf::PdmOptionItemInfo> options = createOptions();

    caf::PdmUiTreeSelectionQModel myModel;
    myModel.setOptions( nullptr, options );

    QModelIndex parentIndex = myModel.index( 0, 0 );

    myModel.setData( parentIndex, QVariant( true ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeSelectionQModelTest, SetCheckedStateForItems )
{
    QList<caf::PdmOptionItemInfo> options = createOptions();

    caf::PdmUiTreeSelectionQModel myModel;
    myModel.setOptions( nullptr, options );

    QModelIndex parentIndex     = myModel.index( 1, 0 );
    QModelIndex firstChildIndex = myModel.index( 0, 0, parentIndex );

    QModelIndexList indices;
    indices << firstChildIndex;

    myModel.setCheckedStateForItems( indices, false );

    // No test code for this behaviour, only making sure this runs without any errors
}

//--------------------------------------------------------------------------------------------------
/// Poisons the memory on delete, to make access to a deleted model fail
//--------------------------------------------------------------------------------------------------
class PoisonedSelectionModel : public caf::PdmUiTreeSelectionQModel
{
public:
    static void operator delete( void* ptr, std::size_t size )
    {
        std::memset( ptr, 0xDD, size );
        s_poisonedStorage = ptr;
    }

    // The poisoned storage is kept alive until the test is done, and released here
    static void releasePoisonedStorage()
    {
        ::operator delete( s_poisonedStorage );
        s_poisonedStorage = nullptr;
    }

private:
    static inline void* s_poisonedStorage = nullptr;
};

class SelectionTestEditor : public caf::PdmUiFieldEditorHandle
{
protected:
    void configureAndUpdateUi( const QString& uiConfigName ) override {}
};

class ModelDeletingObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    ModelDeletingObject()
    {
        CAF_PDM_InitObject( "ModelDeletingObject" );
        CAF_PDM_InitField( &m_selection, "Selection", std::vector<int>(), "Selection" );
    }

    caf::PdmField<std::vector<int>> m_selection;
    QObject*                        m_modelToDelete = nullptr;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override
    {
        delete m_modelToDelete;
        m_modelToDelete = nullptr;
    }
};
CAF_PDM_SOURCE_INIT( ModelDeletingObject, "ModelDeletingObject" );

//--------------------------------------------------------------------------------------------------
/// Changing the field can rebuild the property view and delete the editor owning the model
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeSelectionQModelTest, SetDataWhenModelIsDeletedByFieldChange )
{
    ModelDeletingObject object;

    SelectionTestEditor editor;
    editor.setUiField( object.m_selection.uiCapability() );

    auto model = new PoisonedSelectionModel;
    model->setOptions( &editor, createOptions() );
    object.m_modelToDelete = model;

    QPointer<QObject> modelGuard( model );

    EXPECT_TRUE( model->setData( model->index( 0, 0 ), QVariant( true ), Qt::CheckStateRole ) );
    EXPECT_TRUE( modelGuard.isNull() );

    PoisonedSelectionModel::releasePoisonedStorage();
}

//--------------------------------------------------------------------------------------------------
/// Checking and unchecking is done in one field update, as the first update can delete the model
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTreeSelectionQModelTest, SetCheckedAndUncheckedStateInOneUpdate )
{
    ModelDeletingObject object;
    object.m_selection = std::vector<int>{ 0 };

    SelectionTestEditor editor;
    editor.setUiField( object.m_selection.uiCapability() );

    caf::PdmUiTreeSelectionQModel model;
    model.setOptions( &editor, createOptions() );

    // Top level rows are First, Second, Third and Fourth with option indices 0, 1, 4 and 5
    model.setCheckedStateForItems( { model.index( 2, 0 ) }, { model.index( 0, 0 ) } );

    EXPECT_EQ( std::vector<int>{ 4 }, object.m_selection.v() );
}
