#include "TapCvfSpecialization.h"



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

}
