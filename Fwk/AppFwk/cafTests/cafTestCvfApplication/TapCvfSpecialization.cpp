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

/*
    CAF_PDM_InitField(&m_toggleField, "Toggle", false, "Toggle Field", "", "Toggle Field tooltip", " Toggle Field whatsthis");
    CAF_PDM_InitField(&m_doubleField, "BigNumber", 0.0, "Big Number", "", "Enter a big number here", "This is a place you can enter a big real value if you want");
    CAF_PDM_InitField(&m_intField, "IntNumber", 0, "Small Number", "", "Enter some small number here", "This is a place you can enter a small integer value if you want");
    CAF_PDM_InitField(&m_textField, "TextField", QString(""), "Text", "", "Text tooltip", "This is a place you can enter a small integer value if you want");

    m_proxyDoubleField.registerSetMethod(this, &SmallDemoPdmObject::setDoubleMember);
    m_proxyDoubleField.registerGetMethod(this, &SmallDemoPdmObject::doubleMember);
    CAF_PDM_InitFieldNoDefault(&m_proxyDoubleField, "ProxyDouble", "Proxy Double", "", "", "");

    m_proxyDoubleField = 0;
    if (!(m_proxyDoubleField == 3)) { std::cout << "Double is not 3 " << std::endl; }
*/
}
