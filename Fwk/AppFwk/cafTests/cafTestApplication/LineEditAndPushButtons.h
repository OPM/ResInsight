#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

class LineEditAndPushButtons : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    LineEditAndPushButtons();

    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    void defineEditorAttribute(const caf::PdmFieldHandle* field,
                               QString                    uiConfigName,
                               caf::PdmUiEditorAttribute* attribute) override;

    void rotateContent();
    void appendText();
    void replaceText();
    void clearText();

private:
    caf::PdmField<QString>              m_textField;
    caf::PdmField<QString>              m_statusTextField;
    caf::PdmField<std::vector<QString>> m_textListField;

    caf::PdmField<bool> m_pushButton_a;

    caf::PdmField<bool> m_pushButtonReplace;
    caf::PdmField<bool> m_pushButtonClear;
    caf::PdmField<bool> m_pushButtonAppend;
};
