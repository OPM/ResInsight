#pragma once

#include "cafPdmFieldCapability.h"
#include "cafPdmUiFieldHandleInterface.h"
#include "cafPdmUiItem.h"

namespace caf
{
class PdmFieldHandle;

class PdmUiFieldHandle : public PdmUiItem, public PdmFieldCapability, public PdmUiFieldHandleInterface
{
public:
    PdmUiFieldHandle(PdmFieldHandle* owner, bool giveOwnership);
    virtual ~PdmUiFieldHandle();

    PdmFieldHandle* fieldHandle();

    // Generalized access methods for User interface
    // The QVariant encapsulates the real value, or an index into the valueOptions

    virtual QVariant                 uiValue() const;
    virtual QList<PdmOptionItemInfo> valueOptions(bool* useOptionsOnly);

    void notifyFieldChanged(const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant) override;

    bool isAutoAddingOptionFromValue() const;
    void setAutoAddingOptionFromValue(bool isAddingValue);

private:
    friend class PdmUiCommandSystemProxy;
    friend class CmdFieldChangeExec;
    virtual void setValueFromUiEditor(const QVariant& uiValue);

private:
    PdmFieldHandle* m_owner;
    bool            m_isAutoAddingOptionFromValue;
};

} // End of namespace caf
