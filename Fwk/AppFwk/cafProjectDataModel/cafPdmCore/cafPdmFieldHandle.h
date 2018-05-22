#pragma once

#include "cafPdmBase.h"
#include <QString>
#include <vector>

namespace caf
{

class PdmObjectHandle;
class PdmUiFieldHandle;
class PdmXmlFieldHandle;

//==================================================================================================
/// Base class for all fields, making it possible to handle them generically
//==================================================================================================
class PdmFieldCapability;

class PdmFieldHandle
{
public:
    PdmFieldHandle() { m_ownerObject = nullptr; }
    virtual ~PdmFieldHandle();

    QString          keyword() const                                { return m_keyword; }

    PdmObjectHandle* ownerObject()                                  { return m_ownerObject; }

    // Child objects
    bool             hasChildObjects();
    virtual void     childObjects(std::vector<PdmObjectHandle*>*)        {  }
    virtual void     removeChildObject(PdmObjectHandle*)                 {  }

    // Ptr referenced objects
    bool             hasPtrReferencedObjects();
    virtual void     ptrReferencedObjects(std::vector<PdmObjectHandle*>*)        {  }

    // Capabilities
    void             addCapability(PdmFieldCapability* capability, bool takeOwnership) { m_capabilities.push_back(std::make_pair(capability, takeOwnership)); }

    template <typename CapabilityType>
    CapabilityType*  capability();

    PdmUiFieldHandle*  uiCapability();
    PdmXmlFieldHandle* xmlCapability();

protected:
    bool isInitializedByInitFieldMacro() const { return m_ownerObject != nullptr; }

private:
    PDM_DISABLE_COPY_AND_ASSIGN(PdmFieldHandle);
    
    friend class PdmObjectHandle;   // Give access to m_ownerObject and set Keyword
    void             setKeyword(const QString& keyword);
    PdmObjectHandle* m_ownerObject;

    QString          m_keyword;

    std::vector<std::pair<PdmFieldCapability*, bool> > m_capabilities;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename CapabilityType>
CapabilityType* PdmFieldHandle::capability()
{
    for (size_t i = 0; i < m_capabilities.size(); ++i)
    {
        CapabilityType* capability = dynamic_cast<CapabilityType*>(m_capabilities[i].first);
        if (capability) return capability;
    }
    return NULL;
}

} // End of namespace caf
