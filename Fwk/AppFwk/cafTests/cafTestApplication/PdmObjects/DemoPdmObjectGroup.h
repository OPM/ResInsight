#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmDocument.h"
#include "cafPdmObject.h"

class DemoPdmObjectGroup : public caf::PdmDocument
{
    CAF_PDM_HEADER_INIT;

public:
    DemoPdmObjectGroup();

public:
    caf::PdmChildArrayField<caf::PdmObjectHandle*> objects;
};
