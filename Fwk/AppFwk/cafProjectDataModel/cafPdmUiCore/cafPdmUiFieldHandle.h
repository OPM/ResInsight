#pragma once

#include "cafPdmUiItem.h"
#include "cafPdmFieldCapability.h"
#include "cafPdmUiFieldHandleInterface.h"

namespace caf
{

class PdmFieldHandle;

class PdmUiFieldHandle : public PdmUiItem, public PdmFieldCapability, public PdmUiFieldHandleInterface
{
public:
    PdmUiFieldHandle(PdmFieldHandle* owner, bool giveOwnership);
    virtual ~PdmUiFieldHandle() { }

    PdmFieldHandle* fieldHandle() { return m_owner; }

    // Generalized access methods for User interface
    // The QVariant encapsulates the real value, or an index into the valueOptions

    virtual QVariant uiValue() const                                { return QVariant(); }
    virtual QList<PdmOptionItemInfo>
                     valueOptions(bool* useOptionsOnly)             { return  QList<PdmOptionItemInfo>(); }

    virtual void     notifyFieldChanged(const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant);

    bool             isAutoAddingOptionFromValue() const              { return m_isAutoAddingOptionFromValue; }
    void             setAutoAddingOptionFromValue(bool isAddingValue) { m_isAutoAddingOptionFromValue = isAddingValue;} 

private:
    friend class PdmUiCommandSystemProxy;
    friend class CmdFieldChangeExec;
    virtual void     setValueFromUiEditor(const QVariant& uiValue)        {  }

private:
    PdmFieldHandle*  m_owner;
    bool             m_isAutoAddingOptionFromValue;
};


} // End of namespace caf
