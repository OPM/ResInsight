#pragma once

#include "cvfBase.h"
#include "cvfColor3.h"
#include "cvfMatrix4.h"
#include "cvfVector3.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfVec3d.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <vector>

class TapCvfSpecialization : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    TapCvfSpecialization();

    caf::PdmField<QString> m_testField;

    caf::PdmField<cvf::Color3f> m_colorField;
    caf::PdmField<cvf::Vec3d>   m_vectorField;
    caf::PdmField<cvf::Mat4d>   m_matrixField;

    caf::PdmField<std::vector<cvf::Vec3d>> m_vecArrayField;

protected:
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);
    void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
};
