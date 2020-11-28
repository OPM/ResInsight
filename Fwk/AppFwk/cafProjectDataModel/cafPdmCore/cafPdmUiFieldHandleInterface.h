#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <QVariant>
#pragma GCC diagnostic pop

namespace caf
{
class PdmUiFieldHandleInterface
{
public:
    PdmUiFieldHandleInterface() {}
    virtual ~PdmUiFieldHandleInterface() {}

    virtual QVariant toUiBasedQVariant() const { return QVariant(); }
    virtual void     notifyFieldChanged( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ){};
};

} // End of namespace caf
