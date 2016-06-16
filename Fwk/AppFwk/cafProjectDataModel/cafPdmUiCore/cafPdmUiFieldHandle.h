#pragma once

#include "cafPdmUiItem.h"
#include "cafPdmFieldCapability.h"

namespace caf
{

class PdmFieldHandle;

class PdmUiFieldHandle : public PdmUiItem, public PdmFieldCapability
{
public:
    PdmUiFieldHandle(PdmFieldHandle* owner, bool giveOwnership);
    virtual ~PdmUiFieldHandle() { }

    PdmFieldHandle* fieldHandle() { return m_owner; }

    // Generalized access methods for User interface
    // The QVariant encapsulates the real value, or an index into the valueOptions

    virtual QVariant uiValue() const                                { return QVariant(); }
    virtual void     setValueFromUi(const QVariant& uiValue)        {  }
    virtual QList<PdmOptionItemInfo>
                     valueOptions(bool* useOptionsOnly)             { return  QList<PdmOptionItemInfo>(); }

    virtual QVariant toUiBasedQVariant() const                      { return QVariant(); }
    void             notifyFieldChanged(const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant);

    bool             isAutoAddingOptionFromValue() const              { return m_isAutoAddingOptionFromValue; }
    void             setAutoAddingOptionFromValue(bool isAddingValue) { m_isAutoAddingOptionFromValue = isAddingValue;} 
private:
    PdmFieldHandle*  m_owner;
    bool             m_isAutoAddingOptionFromValue;
};


} // End of namespace caf
