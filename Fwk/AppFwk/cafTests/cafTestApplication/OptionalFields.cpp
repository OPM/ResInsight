
#include "OptionalFields.h"

#include "cafPdmUiLineEditor.h"

#include <QGuiApplication>

CAF_PDM_SOURCE_INIT( OptionalFields, "OptionalFields" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
OptionalFields::OptionalFields()
{
    CAF_PDM_InitObject( "Optional Fields", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_optionalStringField, "OptionalStringField", "Optional String Value" );
    m_optionalStringField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_optionalDoubleField, "OptionalDoubleField", "Optional Double Value" );
    m_optionalDoubleField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_optionalIntField, "OptionalIntField", "Optional Integer Value" );
    m_optionalIntField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );
}
