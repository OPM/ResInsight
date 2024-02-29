
#include "LineEditAndPushButtons.h"

#include "cafPdmUiLabelEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
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

    CAF_PDM_InitFieldNoDefault( &m_textListField, "TextListField", "Text List Field", "", "", "" );
    m_textListField.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_pushButton_a, "PushButtonA", "Rotate", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_pushButton_a );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonReplace, "PushButtonB", "Replace (CTRL + Enter)", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_pushButtonReplace );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonClear, "PushButtonC", "Clear (Alt + Enter)", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_pushButtonClear );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonAppend, "PushButtonD", "Append (Shift + Enter)", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_pushButtonAppend );

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
    if ( changedField == &m_pushButton_a )
    {
        rotateContent();
    }

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

    if ( changedField == &m_pushButtonReplace )
    {
        replaceText();
    }
    if ( changedField == &m_pushButtonClear )
    {
        clearText();
    }
    if ( changedField == &m_pushButtonAppend )
    {
        appendText();
    }

    m_pushButton_a      = false;
    m_pushButtonReplace = false;
    m_pushButtonClear   = false;
    m_pushButtonAppend  = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                    QString                    uiConfigName,
                                                    caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_textField )
    {
        auto myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->notifyWhenTextIsEdited = true;
        }
    }
    else if ( field == &m_labelLongTextField )
    {
        auto myAttr = dynamic_cast<caf::PdmUiLabelEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_useWordWrap                                  = true;
            myAttr->m_useSingleWidgetInsteadOfLabelAndEditorWidget = true;
        }
    }
    else if ( field == &m_textListField )
    {
        auto myAttr = dynamic_cast<caf::PdmUiListEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->heightHint = 150;
        }
    }

    {
        auto myAttr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( myAttr )
        {
            if ( field == &m_pushButton_a )
            {
                myAttr->m_buttonText = "&Push Me";
            }
            if ( field == &m_pushButtonReplace )
            {
                myAttr->m_buttonText = "Replace (Ctrl + Enter)";
            }
            if ( field == &m_pushButtonClear )
            {
                myAttr->m_buttonText = "Clear (Alt + Enter)";
            }
            if ( field == &m_pushButtonAppend )
            {
                myAttr->m_buttonText = "Append (Shift + Enter)";
            }
        }
    }
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::appendText()
{
    auto original = m_textListField.value();
    original.push_back( m_textField );

    m_textListField = original;
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
}
