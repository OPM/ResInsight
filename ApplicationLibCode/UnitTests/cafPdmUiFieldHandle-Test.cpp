/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiEditorHandle.h"

#include <cstddef>
#include <new>

namespace
{
class PdmUiFieldHandleTestObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    PdmUiFieldHandleTestObject()
    {
        CAF_PDM_InitObject( "PdmUiFieldHandleTestObject" );
        CAF_PDM_InitField( &m_value, "Value", 0, "Value" );
    }

    caf::PdmField<int> m_value;
};

CAF_PDM_SOURCE_INIT( PdmUiFieldHandleTestObject, "PdmUiFieldHandleTestObject" );

class TestFieldEditor : public caf::PdmUiEditorHandle
{
public:
    TestFieldEditor() = default;
    explicit TestFieldEditor( caf::PdmUiFieldHandle* field ) { bindToPdmItem( field ); }

    void bind( caf::PdmUiItem* item ) { bindToPdmItem( item ); }

    int  updateCount        = 0;
    bool disconnectOnUpdate = false;

    // Connected to this editor's item when this editor disconnects
    TestFieldEditor* editorToConnectOnDisconnect = nullptr;

protected:
    void configureAndUpdateUi( const QString& ) override
    {
        updateCount++;
        if ( disconnectOnUpdate )
        {
            auto item = pdmItem();
            bindToPdmItem( nullptr );
            if ( editorToConnectOnDisconnect ) editorToConnectOnDisconnect->bind( item );
        }
    }
};

//--------------------------------------------------------------------------------------------------
/// Deletes other editors of the same field when updated
//--------------------------------------------------------------------------------------------------
class DeletingFieldEditor : public TestFieldEditor
{
public:
    using TestFieldEditor::TestFieldEditor;

    std::vector<TestFieldEditor*> editorsToDelete;

protected:
    void configureAndUpdateUi( const QString& uiConfigName ) override
    {
        TestFieldEditor::configureAndUpdateUi( uiConfigName );
        for ( auto editor : editorsToDelete )
        {
            delete editor;
        }
        editorsToDelete.clear();
    }
};

//--------------------------------------------------------------------------------------------------
/// Recreates its field editors on update, like an object editor does when the visible fields change
//--------------------------------------------------------------------------------------------------
class RebuildingContainerEditor : public caf::PdmUiEditorHandle
{
public:
    std::vector<TestFieldEditor*> fieldEditors;
    int                           updateCount = 0;

protected:
    void configureAndUpdateUi( const QString& ) override
    {
        updateCount++;
        for ( auto editor : fieldEditors )
        {
            delete editor;
        }
        fieldEditors.clear();
    }
};

//--------------------------------------------------------------------------------------------------
/// Destroys its field editors on update and creates new ones at the same addresses, to emulate an allocator
/// reusing the memory of deleted editors
//--------------------------------------------------------------------------------------------------
class RecreatingContainerEditor : public caf::PdmUiEditorHandle
{
public:
    static constexpr int editorCount = 2;

    explicit RecreatingContainerEditor( caf::PdmUiFieldHandle* field )
        : m_field( field )
    {
        for ( int i = 0; i < editorCount; i++ )
        {
            createEditor( i );
        }
    }

    ~RecreatingContainerEditor() override
    {
        for ( int i = 0; i < editorCount; i++ )
        {
            editor( i )->~TestFieldEditor();
        }
    }

    TestFieldEditor* editor( int index ) { return std::launder( reinterpret_cast<TestFieldEditor*>( m_storage[index] ) ); }

    int updateCount = 0;

protected:
    void configureAndUpdateUi( const QString& ) override
    {
        updateCount++;
        for ( int i = 0; i < editorCount; i++ )
        {
            editor( i )->~TestFieldEditor();
            createEditor( i );
        }
    }

private:
    void createEditor( int index )
    {
        auto newEditor = new ( m_storage[index] ) TestFieldEditor( m_field );
        newEditor->setContainingEditor( this );
    }

    caf::PdmUiFieldHandle* m_field;
    alignas( TestFieldEditor ) std::byte m_storage[editorCount][sizeof( TestFieldEditor )];
};

} // namespace

//--------------------------------------------------------------------------------------------------
/// Field editors deleted while the containing editor is updated must not be accessed afterwards
//--------------------------------------------------------------------------------------------------
TEST( PdmUiFieldHandle, NotifyFieldChangedWhenContainingEditorDeletesFieldEditors )
{
    PdmUiFieldHandleTestObject object;
    RebuildingContainerEditor  container;

    for ( int i = 0; i < 3; i++ )
    {
        auto editor = new TestFieldEditor( object.m_value.uiCapability() );
        editor->setContainingEditor( &container );
        container.fieldEditors.push_back( editor );
    }

    object.m_value.uiCapability()->notifyFieldChanged( QVariant( 0 ), QVariant( 1 ) );

    EXPECT_EQ( 1, container.updateCount );
    EXPECT_TRUE( object.m_value.uiCapability()->connectedEditors().empty() );
}

//--------------------------------------------------------------------------------------------------
/// Editors deleted while another editor of the same item is updated must be skipped, and the remaining
/// editors must still be updated
//--------------------------------------------------------------------------------------------------
TEST( PdmUiFieldHandle, UpdateConnectedEditorsWhenEditorDeletesOtherEditors )
{
    PdmUiFieldHandleTestObject object;
    auto*                      field = object.m_value.uiCapability();

    DeletingFieldEditor deletingEditor( field );
    TestFieldEditor     survivingEditor( field );

    for ( int i = 0; i < 3; i++ )
    {
        deletingEditor.editorsToDelete.push_back( new TestFieldEditor( field ) );
    }

    field->updateConnectedEditors();

    EXPECT_EQ( 1, deletingEditor.updateCount );
    EXPECT_EQ( 1, survivingEditor.updateCount );
    EXPECT_EQ( 2u, field->connectedEditors().size() );
}

//--------------------------------------------------------------------------------------------------
/// An editor disconnecting itself while updated must not stop the remaining editors from being updated, and editors
/// connected during the update are not part of it
//--------------------------------------------------------------------------------------------------
TEST( PdmUiFieldHandle, UpdateConnectedEditorsWhenEditorDisconnectsItself )
{
    PdmUiFieldHandleTestObject object;
    auto*                      field = object.m_value.uiCapability();

    // Array elements have increasing addresses, so the editor set is ordered by array index. The first editor
    // disconnects itself and connects the last editor, which typically reuses the erased set node. Iterating the live
    // set would then continue from the last editor and skip the second one.
    TestFieldEditor editors[3];
    editors[0].disconnectOnUpdate          = true;
    editors[0].editorToConnectOnDisconnect = &editors[2];
    editors[0].bind( field );
    editors[1].bind( field );

    field->updateConnectedEditors();

    EXPECT_EQ( 1, editors[0].updateCount );
    EXPECT_EQ( 1, editors[1].updateCount );
    EXPECT_EQ( 0, editors[2].updateCount );
    EXPECT_EQ( 2u, field->connectedEditors().size() );
}

//--------------------------------------------------------------------------------------------------
/// A field editor recreated at the address of a deleted editor must not be treated as the deleted editor
//--------------------------------------------------------------------------------------------------
TEST( PdmUiFieldHandle, NotifyFieldChangedWhenFieldEditorsAreRecreatedAtSameAddress )
{
    PdmUiFieldHandleTestObject object;
    RecreatingContainerEditor  container( object.m_value.uiCapability() );

    object.m_value.uiCapability()->notifyFieldChanged( QVariant( 0 ), QVariant( 1 ) );

    EXPECT_EQ( 1, container.updateCount );
    EXPECT_EQ( size_t( RecreatingContainerEditor::editorCount ), object.m_value.uiCapability()->connectedEditors().size() );
}
