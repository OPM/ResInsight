#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

class OptionalFields : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    OptionalFields();

private:
    caf::PdmField<std::optional<QString>> m_optionalStringField;
    caf::PdmField<std::optional<double>>  m_optionalDoubleField;
    caf::PdmField<std::optional<int>>     m_optionalIntField;
};
