#pragma once


#include "..\LibCore\cvfBase.h"
#include "..\LibCore\cvfColor3.h"
#include "..\LibCore\cvfVector3.h"
#include "..\LibCore\cvfMatrix4.h"

#include "cafPdmObject.h"
#include "cafPdmField.h"



class TapCvfSpecialization : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    TapCvfSpecialization();

    caf::PdmField<cvf::Color3> m_colorField;
    caf::PdmField<cvf::Vec3d>  m_vectorField;
    caf::PdmField<cvf::Mat4d>  m_matrixField;

/*
    virtual caf::PdmFieldHandle* objectToggleField()
    {
        return &m_toggleField;
    }

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
    {
        if (changedField == &m_toggleField)
        {
            std::cout << "Toggle Field changed" << std::endl;
        }
    }
*/

};

