#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class LabelsAndHyperlinks : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    LabelsAndHyperlinks();

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<QString> m_labelTextField;
    caf::PdmField<QString> m_hyperlinkTextField;
};
