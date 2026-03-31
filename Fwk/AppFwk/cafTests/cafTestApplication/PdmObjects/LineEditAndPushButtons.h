#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

class LineEditAndPushButtons : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    LineEditAndPushButtons();

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void rotateContent();
    void appendText();
    void replaceText();
    void clearText();

private:
    caf::PdmField<QString> m_textField;
    caf::PdmField<QString> m_statusTextField;

    caf::PdmField<QString> m_labelField;
    caf::PdmField<QString> m_labelLongTextField;

    caf::PdmField<std::vector<QString>> m_textListField;
};
