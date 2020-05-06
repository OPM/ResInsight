#include "TapProject.h"
#include "TapCvfSpecialization.h"

CAF_PDM_SOURCE_INIT(TapProject, "RPMProject");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TapProject::TapProject(void)
{
    CAF_PDM_InitFieldNoDefault(&m_objectList, "ObjectList", "Objects list Field", "", "List", "This is a list of PdmObjects");

    CAF_PDM_InitFieldNoDefault(&m_testSpecialization, "TapCvfSpecialization", "TapCvfSpecialization Field", "", "", "");
    m_testSpecialization.push_back(new TapCvfSpecialization);
    m_testSpecialization.push_back(new TapCvfSpecialization);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TapProject::~TapProject(void) {}
