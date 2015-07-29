

#include "cafPdmObjectGroup.h"
#include "cafInternalPdmXmlFieldCapability.h"

#include <QFile>

namespace caf
{

CAF_PDM_SOURCE_INIT(PdmObjectGroup, "PdmObjectGroup");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectGroup::PdmObjectGroup() 
{
    CAF_PDM_InitObject("Object Group", "", "", "");
    CAF_PDM_InitFieldNoDefault(&objects, "PdmObjects", "Children","", "", "" );
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectGroup::~PdmObjectGroup()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectGroup::deleteObjects()
{
    size_t it;
    for (it = 0; it != objects.size(); ++it)
    {
        delete objects[it];
    }
    removeNullPtrs();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectGroup::removeNullPtrs()
{
    objects.removeChildObject(NULL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectGroup::addObject(PdmObjectHandle * obj)
{
    objects.push_back(obj);
}




} //End of namespace caf

