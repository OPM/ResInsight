#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

class SmallGridDemoPdmObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    SmallGridDemoPdmObject();

    // Outside group
    caf::PdmField<int> m_intFieldStandard;
    caf::PdmField<int> m_intFieldUseFullSpace;
    caf::PdmField<int> m_intFieldUseFullSpaceLabel;
    caf::PdmField<int> m_intFieldUseFullSpaceField;
    caf::PdmField<int> m_intFieldWideLabel;
    caf::PdmField<int> m_intFieldWideField;
    caf::PdmField<int> m_intFieldWideBoth;
    caf::PdmField<int> m_intFieldLeft;
    caf::PdmField<int> m_intFieldRight;

    // First group
    caf::PdmField<int>     m_intFieldWideBoth2;
    caf::PdmField<int>     m_intFieldLeft2;
    caf::PdmField<int>     m_intFieldCenter;
    caf::PdmField<int>     m_intFieldRight2;
    caf::PdmField<int>     m_intFieldLabelTop;
    caf::PdmField<QString> m_stringFieldLabelHidden;

    // Auto group
    caf::PdmField<int>     m_intFieldWideBothAuto;
    caf::PdmField<int>     m_intFieldLeftAuto;
    caf::PdmField<int>     m_intFieldCenterAuto;
    caf::PdmField<int>     m_intFieldRightAuto;
    caf::PdmField<int>     m_intFieldLabelTopAuto;
    caf::PdmField<QString> m_stringFieldLabelHiddenAuto;

    // Combination with groups
    caf::PdmField<int> m_intFieldLeftOfGroup;
    caf::PdmField<int> m_intFieldRightOfGroup;
    caf::PdmField<int> m_intFieldInsideGroup1;
    caf::PdmField<int> m_intFieldInsideGroup2;

    // Side-by-side groups
    caf::PdmField<int> m_intFieldInsideGroup3;
    caf::PdmField<int> m_intFieldInsideGroup4;
    caf::PdmField<int> m_intFieldInsideGroup5;
    caf::PdmField<int> m_intFieldInsideGroup6;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
};
