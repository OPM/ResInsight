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
    explicit TestFieldEditor( caf::PdmUiFieldHandle* field ) { bindToPdmItem( field ); }

protected:
    void configureAndUpdateUi( const QString& ) override {}
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
