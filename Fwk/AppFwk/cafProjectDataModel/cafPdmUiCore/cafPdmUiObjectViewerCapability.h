
#pragma once

#include "cafPdmObjectCapability.h"

namespace caf
{
class PdmUiObjectViewerCapability : public PdmObjectCapability
{
public:
    virtual int  id() const = 0;
    virtual void createViewWidget() = 0;
    virtual void zoomAll() = 0;
};
}
