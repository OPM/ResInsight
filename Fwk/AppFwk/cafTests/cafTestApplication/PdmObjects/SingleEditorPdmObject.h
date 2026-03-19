#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

class SingleEditorPdmObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    SingleEditorPdmObject();

    // Outside group
    caf::PdmField<int> m_intFieldStandard;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
};
