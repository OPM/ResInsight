#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

class ColorTriplet : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    ColorTriplet();

private:
    caf::PdmField<QString> m_colorField;
    caf::PdmField<int>     m_categoryValue;
    caf::PdmField<QString> m_categoryText;
};
