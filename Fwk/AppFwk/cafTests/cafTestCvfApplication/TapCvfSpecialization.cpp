#include "TapCvfSpecialization.h"

#include "cafPdmUiListEditor.h"

CAF_PDM_SOURCE_INIT(TapCvfSpecialization, "TapCvfSpecialization");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TapCvfSpecialization::TapCvfSpecialization()
{
    CAF_PDM_InitObject("Test Object for type specializations", "", "", "");

    CAF_PDM_InitField(&m_vectorField, "m_vectorField", cvf::Vec3d::ZERO, "Start Point", "", "", "");
    CAF_PDM_InitField(&m_matrixField, "m_matrixField", cvf::Mat4d::ZERO, "Zero matrix", "", "", "");
    CAF_PDM_InitField(&m_colorField, "m_colorField", cvf::Color3f(cvf::Color3::GREEN), "Color3f", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_vecArrayField, "Points", "Selected points", "", "", "");

    m_vecArrayField.v().push_back(cvf::Vec3d(1, 2, 3));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TapCvfSpecialization::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue)
{
    if (changedField == &m_colorField)
    {
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TapCvfSpecialization::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                 QString                    uiConfigName,
                                                 caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_vecArrayField)
    {
        caf::PdmUiListEditorAttribute* myAttr = dynamic_cast<caf::PdmUiListEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_baseColor.setRgbF(m_colorField().r(), m_colorField().g(), m_colorField().b());
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TapCvfSpecialization::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    caf::IconProvider iconProvider = this->uiIconProvider();

    cvf::Color3f cvfColor = m_colorField();
    QColor       qcolor(cvfColor.rByte(), cvfColor.gByte(), cvfColor.bByte());

    iconProvider.setBackgroundColorString(qcolor.name());

    this->setUiIcon(iconProvider);
}
