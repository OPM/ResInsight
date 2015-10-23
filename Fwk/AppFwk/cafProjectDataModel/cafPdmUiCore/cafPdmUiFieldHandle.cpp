#include "cafPdmUiFieldHandle.h"
#include "cafPdmFieldHandle.h"

#include "cafPdmUiObjectHandle.h"

#include <assert.h>

namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiFieldHandle::PdmUiFieldHandle(PdmFieldHandle* owner, bool giveOwnership)
{
    m_owner = owner; 
    owner->addCapability(this, giveOwnership);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::notifyFieldChanged(const QVariant& oldFieldValue, const QVariant& newFieldValue)
{
    if (oldFieldValue != newFieldValue)
    {
        PdmFieldHandle* fieldHandle = this->fieldHandle();
        assert(fieldHandle && fieldHandle->ownerObject());

        PdmUiObjectHandle* uiObjHandle = uiObj(fieldHandle->ownerObject());
        if (uiObjHandle)
        {
            uiObjHandle->fieldChangedByUi(fieldHandle, oldFieldValue, newFieldValue);
            uiObjHandle->updateConnectedEditors();
        }

        // Update field editors
        this->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// Implementation of uiCapability() defined in cafPdmFieldHandle.h
//--------------------------------------------------------------------------------------------------
PdmUiFieldHandle* PdmFieldHandle::uiCapability()
{
    PdmUiFieldHandle* uiField = capability<PdmUiFieldHandle>();
    assert(uiField);

    return uiField;
}

} // End of namespace caf
