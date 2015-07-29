#pragma once

#include "cafPdmDocument.h"



class TapProject : public caf::PdmDocument
{
    CAF_PDM_HEADER_INIT;

public:
    TapProject(void);
    virtual ~TapProject(void);

    caf::PdmChildArrayField< caf::PdmObjectHandle*  > m_objectList;

};