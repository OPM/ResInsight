
#include "LineEditAndPushButtons.h"

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT(LineEditAndPushButtons, "LineEditAndPushButtons");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
LineEditAndPushButtons::LineEditAndPushButtons()
{
    CAF_PDM_InitObject("Line Edit And Push Buttons", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_statusTextField, "StatusTextField", "Status Text", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_textField, "TextField", "Text", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_textListField, "TextListField", "Text List Field", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_pushButton_a, "PushButtonA", "Rotate", "", "", "");
    m_pushButton_a.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_pushButton_a.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_pushButtonReplace, "PushButtonB", "Replace (CTRL + Enter)", "", "", "");
    m_pushButtonReplace.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_pushButtonReplace.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_pushButtonClear, "PushButtonC", "Clear (Alt + Enter)", "", "", "");
    m_pushButtonClear.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_pushButtonClear.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_pushButtonAppend, "PushButtonD", "Append (Shift + Enter)", "", "", "");
    m_pushButtonAppend.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_pushButtonAppend.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    std::vector<QString> items;
    items.push_back("sldkfj");
    items.push_back("annet sldkfj");
    items.push_back("kort");
    items.push_back("veldig langt");
    items.push_back("kort");

    m_textListField = items;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue)
{
    if (changedField == &m_pushButton_a)
    {
        rotateContent();
    }

    if (changedField == &m_textField)
    {
        auto mods = QGuiApplication::keyboardModifiers();

        // Use global keyboard modifiers to trigger different events when content is changed in editor changes

        if (mods & Qt::ShiftModifier)
            appendText();
        else if (mods & Qt::ControlModifier)
            replaceText();
        else if (mods & Qt::AltModifier)
            clearText();
        else
        {
            m_statusTextField = m_textField;
        }
    }

    if (changedField == &m_pushButtonReplace)
    {
        replaceText();
    }
    if (changedField == &m_pushButtonClear)
    {
        clearText();
    }
    if (changedField == &m_pushButtonAppend)
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
void LineEditAndPushButtons::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                   QString                    uiConfigName,
                                                   caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_textField)
    {
        auto myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->notifyWhenTextIsEdited = true;
        }
    }

    {
        auto myAttr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (myAttr)
        {
            if (field == &m_pushButton_a)
            {
                myAttr->m_buttonText = "&Push Me";
            }
            if (field == &m_pushButtonReplace)
            {
                myAttr->m_buttonText = "Replace (Ctrl + Enter)";
            }
            if (field == &m_pushButtonClear)
            {
                myAttr->m_buttonText = "Clear (Alt + Enter)";
            }
            if (field == &m_pushButtonAppend)
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
    newContent.insert(newContent.begin(), original.begin(), original.end());

    auto firstItem = newContent.front();
    newContent.pop_front();
    newContent.push_back(firstItem);

    std::vector<QString> tmp;
    tmp.insert(tmp.begin(), newContent.begin(), newContent.end());

    m_textListField = tmp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LineEditAndPushButtons::appendText()
{
    auto original = m_textListField.value();
    original.push_back(m_textField);

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
