#include "cafPdmUiFieldHandle.h"
#include "cafPdmFieldHandle.h"

#include "cafPdmUiObjectHandle.h"

#include <assert.h>

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiFieldHandle* uiField(PdmFieldHandle* field)
{
    if (!field) return NULL;
    PdmUiFieldHandle* uiField = field->capability<PdmUiFieldHandle>();
    return uiField;
}


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
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::updateConnectedUiEditors(PdmFieldHandle* field)
{
    if (!field) return;

    PdmUiFieldHandle* uiFieldHandle = field->capability<PdmUiFieldHandle>();
    if (uiFieldHandle)
    {
        uiFieldHandle->updateConnectedEditors();
    }
}


} // End of namespace caf
