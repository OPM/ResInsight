#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

class ApplicationEnum : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class MyEnumType
    {
        T1,
        T2,
        T3,
        T4,
        T5,
        T6,
        T7
    };

public:
    ApplicationEnum();

private:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<caf::AppEnum<MyEnumType>> m_enumField;
    caf::PdmField<caf::AppEnum<MyEnumType>> m_enum2Field;
    caf::PdmField<caf::AppEnum<MyEnumType>> m_enum3Field;
    caf::PdmField<caf::AppEnum<MyEnumType>> m_radioButtonEnumField;
    caf::PdmField<int>                      m_integerField;
};
