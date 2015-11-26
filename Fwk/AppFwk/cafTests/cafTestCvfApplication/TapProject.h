#pragma once


#include "cafPdmDocument.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"

class TapCvfSpecialization;


class TapProject : public caf::PdmDocument
{
    CAF_PDM_HEADER_INIT;

public:
    TapProject(void);
    virtual ~TapProject(void);

    caf::PdmChildArrayField< caf::PdmObjectHandle*  > m_objectList;

    caf::PdmChildField< TapCvfSpecialization* > m_testSpecialization;
};