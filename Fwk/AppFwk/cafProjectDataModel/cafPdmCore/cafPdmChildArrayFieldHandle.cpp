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
    std::vector<PdmObjectHandle*> childObjects = children();

    if ( childObjects.empty() )
    {
        return true;
    }

    size_t fieldCount = 0;
    for ( size_t i = 0; i < childObjects.size(); i++ )
    {
        std::vector<PdmFieldHandle*> fields = childObjects[i]->fields();

        if ( i == 0 )
        {
            fieldCount = fields.size();
        }
        else if ( fieldCount != fields.size() )
        {
            return false;
        }
    }

    return true;
}

} // End of namespace caf
