#pragma once

#include <QVariant>

namespace caf
{

class PdmUiFieldHandleInterface
{
public:
    PdmUiFieldHandleInterface() {}
    virtual ~PdmUiFieldHandleInterface() {}

    virtual QVariant toUiBasedQVariant() const { return QVariant(); }
    virtual void     notifyFieldChanged(const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant) { };
};

} // End of namespace caf
