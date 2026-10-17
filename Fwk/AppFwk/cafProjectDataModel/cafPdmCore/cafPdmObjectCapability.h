#pragma once

namespace caf
{
class PdmObjectCapability
{
public:
    PdmObjectCapability() {}
    virtual ~PdmObjectCapability() {}

    // Called for each capability of an object after its fields are read, before PdmXmlObjectHandle::initAfterRead()
    virtual void onAfterRead() {}
};

} // End namespace caf
