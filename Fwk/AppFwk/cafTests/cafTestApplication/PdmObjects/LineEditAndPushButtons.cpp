#include "LineEditAndPushButtons.h"

#include "cafPdmUiButton.h"
#include "cafPdmUiLabelEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QGuiApplication>

CAF_PDM_SOURCE_INIT( LineEditAndPushButtons, "LineEditAndPushButtons" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
LineEditAndPushButtons::LineEditAndPushButtons()
{
    CAF_PDM_InitObject( "Line Edit And Push Buttons", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_statusTextField, "StatusTextField", "Status Text", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_textField, "TextField", "Text", "", "", "" );
    m_textField.uiCapability()->setAttribute( caf::PdmUiLineEditor::Keys::NOTIFY_WHEN_TEXT_IS_EDITED, true );

    CAF_PDM_InitFieldNoDefault( &m_labelField, "LabelField", "Medium length text in label", "", "", "" );
    m_labelField.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_labelLongTextField,
                                "LongTextField",
                                "Long length text in label length text in label length text in label length text in "
                                "label length text in label",
                                "",
                                "",
                                "" );
    m_labelLongTextField.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_labelLongTextField.uiCapability()->setAttribute( caf::PdmUiLabelEditor::Keys::USE_WORD_WRAP, true );
    m_labelLongTextField.uiCapability()
        ->setAttribute( caf::PdmUiLabelEditor::Keys::USE_SINGLE_WIDGET_INSTEAD_OF_LABEL_AND_EDITOR_WIDGET, true );

    CAF_PDM_InitFieldNoDefault( &m_textListField, "TextListField", "Text List Field", "", "", "" );
    m_textListField.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    m_textListField.uiCapability()->setAttribute( caf::PdmUiListEditor::Keys::HEIGHT_HINT, 150 );

    std::vector<QString> items;
    items.push_back( "sldkfj" );
    items.push_back( "annet sldkfj" );
    items.push_back( "kort" );
    items.push_back( "veldig langt" );
    items.push_back( "kort" );

    m_textListField = items;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue )
{
    if ( changedField == &m_textField )
    {
        auto mods = QGuiApplication::keyboardModifiers();

        // Use global keyboard modifiers to trigger different events when content is changed in editor changes

        if ( mods & Qt::ShiftModifier )
            appendText();
        else if ( mods & Qt::ControlModifier )
            replaceText();
        else if ( mods & Qt::AltModifier )
            clearText();
        else
        {
            m_statusTextField = m_textField;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_statusTextField );
    uiOrdering.add( &m_textField );
    uiOrdering.add( &m_labelField );
    uiOrdering.add( &m_labelLongTextField );
    uiOrdering.add( &m_textListField );
    uiOrdering.addNewButton( "&Push Me", [this]() { rotateContent(); } );
    uiOrdering.addNewButton( "Replace (Ctrl + Enter)", [this]() { replaceText(); } );
    uiOrdering.addNewButton( "Clear (Alt + Enter)", [this]() { clearText(); } );
    uiOrdering.addNewButton( "Append (Shift + Enter)", [this]() { appendText(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::rotateContent()
{
    auto original = m_textListField.value();

    std::list<QString> newContent;
    if ( original.empty() )
    {
        newContent.push_back( "Item A" );
        newContent.push_back( "Item B" );
    }
    else
    {
        newContent.insert( newContent.begin(), original.begin(), original.end() );
    }

    auto firstItem = newContent.front();
    newContent.pop_front();
    newContent.push_back( firstItem );

    std::vector<QString> tmp;
    tmp.insert( tmp.begin(), newContent.begin(), newContent.end() );

    m_textListField = tmp;

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::appendText()
{
    auto original = m_textListField.value();
    original.push_back( m_textField );

    m_textListField = original;

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::replaceText()
{
    clearText();
    appendText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::clearText()
{
    m_textListField = std::vector<QString>();

    updateConnectedEditors();
}
