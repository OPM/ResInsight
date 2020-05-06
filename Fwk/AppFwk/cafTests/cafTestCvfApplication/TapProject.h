#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmDocument.h"

class TapCvfSpecialization;

class TapProject : public caf::PdmDocument
{
    CAF_PDM_HEADER_INIT;

public:
    TapProject(void);
    virtual ~TapProject(void);

    caf::PdmChildArrayField<caf::PdmObjectHandle*> m_objectList;

    caf::PdmChildArrayField<TapCvfSpecialization*> m_testSpecialization;
};
