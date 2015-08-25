#include "cafPdmChildArrayField.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmObjectHandle.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmChildArrayFieldHandle::hasSameFieldCountForAllObjects()
{
    std::vector<PdmObjectHandle*> listObjects;
    this->childObjects(&listObjects);

    if (listObjects.size() == 0)
    {
        return true;
    }

    size_t fieldCount = 0;
    for (size_t i = 0; i < listObjects.size(); i++)
    {
        std::vector<PdmFieldHandle*> fields;
        listObjects[i]->fields(fields);

        if (i == 0)
        {
            fieldCount = fields.size();
        }
        else if (fieldCount != fields.size())
        {
            return false;
        }
    }

    return true;
}

} // End of namespace caf
