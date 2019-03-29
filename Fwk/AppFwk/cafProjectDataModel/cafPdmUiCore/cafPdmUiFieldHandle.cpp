#include "cafPdmUiFieldHandle.h"

#include "cafAssert.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmUiModelChangeDetector.h"
#include "cafPdmUiObjectHandle.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiFieldHandle::PdmUiFieldHandle(PdmFieldHandle* owner, bool giveOwnership)
    : m_isAutoAddingOptionFromValue(true)
{
    m_owner = owner;
    owner->addCapability(this, giveOwnership);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiFieldHandle::~PdmUiFieldHandle() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* PdmUiFieldHandle::fieldHandle()
{
    return m_owner;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant PdmUiFieldHandle::uiValue() const
{
    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> PdmUiFieldHandle::valueOptions(bool* useOptionsOnly) const
{
    return QList<PdmOptionItemInfo>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::notifyFieldChanged(const QVariant& oldFieldValue, const QVariant& newFieldValue)
{
    // Todo : Should use a virtual version of isElementEqual. The variant != operation will not work on user types

    if (oldFieldValue != newFieldValue)
    {
        PdmFieldHandle* fieldHandle = this->fieldHandle();
        CAF_ASSERT(fieldHandle && fieldHandle->ownerObject());

        PdmObjectHandle* ownerObjectHandle = fieldHandle->ownerObject();

        {
            PdmUiObjectHandle* uiObjHandle = uiObj(ownerObjectHandle);
            if (uiObjHandle)
            {
                uiObjHandle->fieldChangedByUi(fieldHandle, oldFieldValue, newFieldValue);
                uiObjHandle->updateConnectedEditors();

            }
        }

        if (ownerObjectHandle->parentField() && ownerObjectHandle->parentField()->ownerObject())
        {
            PdmUiObjectHandle* uiObjHandle = uiObj(ownerObjectHandle->parentField()->ownerObject());
            if (uiObjHandle)
            {
                uiObjHandle->childFieldChangedByUi(ownerObjectHandle->parentField());

                // If updateConnectedEditors() is required, this has to be called in childFieldChangedByUi()
            }
        }

        // Update connected field editors or their parent editors, to make  the ui reflect the change
        this->updateConnectedEditors();

        PdmUiModelChangeDetector::instance()->setModelChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiFieldHandle::isAutoAddingOptionFromValue() const
{
    return m_isAutoAddingOptionFromValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::setAutoAddingOptionFromValue(bool isAddingValue)
{
    m_isAutoAddingOptionFromValue = isAddingValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::setValueFromUiEditor(const QVariant& uiValue) {}

//--------------------------------------------------------------------------------------------------
/// Implementation of uiCapability() defined in cafPdmFieldHandle.h
//--------------------------------------------------------------------------------------------------
PdmUiFieldHandle* PdmFieldHandle::uiCapability()
{
    PdmUiFieldHandle* uiField = capability<PdmUiFieldHandle>();
    CAF_ASSERT(uiField);

    return uiField;
}

} // End of namespace caf
