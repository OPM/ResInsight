#pragma once

#include "cafPdmBase.h"

class QString;

#include <vector>
#include <set>


namespace caf
{

class PdmObjectCapability;
class PdmFieldHandle;
class PdmUiObjectHandle;
class PdmXmlObjectHandle;

//==================================================================================================
/// The base class of all objects
//==================================================================================================
class PdmObjectHandle
{
public:
    PdmObjectHandle()           { m_parentField = NULL;  }
    virtual ~PdmObjectHandle();

    /// The registered fields contained in this PdmObject. 
    void                    fields(std::vector<PdmFieldHandle*>& fields) const;
    PdmFieldHandle*         findField(const QString& keyword);

    /// The field referencing this object as a child
    PdmFieldHandle*         parentField() const;
    /// Convenience function. Traverses from parent to parents parent to find first object of the requested type.
    template <typename T>
    void                    firstAnchestorOrThisOfType(T*& ancestor) const;

    // PtrReferences
    /// The PdmPtrField's containing pointers to this PdmObjecthandle 
    /// Use ownerObject() on the fieldHandle to get the PdmObjectHandle 
    void                    referringPtrFields(std::vector<PdmFieldHandle*>& fieldsReferringToMe) const;
    /// Convenience method to get the objects pointing to this field 
    void                    objectsWithReferringPtrFields(std::vector<PdmObjectHandle*>& objects) const;

    // Object capabilities
    void                    addCapability(PdmObjectCapability* capability, bool takeOwnership) { m_capabilities.push_back(std::make_pair(capability, takeOwnership)); }

    template <typename CapabilityType>
    CapabilityType* capability() 
    {
        for (size_t i = 0; i < m_capabilities.size(); ++i)
        {
            CapabilityType* capability = dynamic_cast<CapabilityType*>(m_capabilities[i].first);
            if (capability) return capability;
        }
        return NULL;
    }

    PdmUiObjectHandle*  uiCapability();     // Implementation is in cafPdmUiObjectHandle.cpp
    PdmXmlObjectHandle* xmlCapability();    // Implementation is in cafPdmXmlObjectHandle.cpp

protected: 
    void addField(PdmFieldHandle* field, const QString& keyword);

private:
    PDM_DISABLE_COPY_AND_ASSIGN(PdmObjectHandle);

    // Fields
    std::vector<PdmFieldHandle*>    m_fields;

    // Capabilities
    std::vector<std::pair<PdmObjectCapability*, bool> > m_capabilities;

    // Child/Parent Relationships
    void                            setAsParentField(PdmFieldHandle* parentField);
    void                            removeAsParentField(PdmFieldHandle* parentField);
    PdmFieldHandle*                 m_parentField;

    // PtrReferences
    void                            addReferencingPtrField(PdmFieldHandle* fieldReferringToMe);
    void                            removeReferencingPtrField(PdmFieldHandle* fieldReferringToMe);
    std::multiset<PdmFieldHandle*>  m_referencingPtrFields;

    // Give access to set/removeAsParentField
    template < class T > friend class PdmChildArrayField;
    template < class T > friend class PdmChildField;
    template < class T > friend class PdmPtrArrayField;
    template < class T > friend class PdmPtrField;
    template < class T > friend class PdmField; // For backwards compatibility layer

    template < class T > friend class PdmFieldXmlCap; 
    
    static const char* classKeywordStatic() { return "PdmObjectHandle";} // For PdmXmlFieldCap to be able to handle fields of PdmObjectHandle directly

    // Support system for PdmPointer
    friend class PdmPointerImpl;
    std::set<PdmObjectHandle**>         m_pointersReferencingMe;
};
}

#include "cafPdmFieldHandle.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObjectHandle::firstAnchestorOrThisOfType(T*& ancestor) const
{
    ancestor = NULL;

    // Check if this matches the type

    const T* objectOfType = dynamic_cast<const T*>(this);
    if (objectOfType)
    {
        ancestor = const_cast<T*>(objectOfType);
        return;
    }

    // Search anchestors

    PdmObjectHandle* parent = NULL;
    PdmFieldHandle* parentField = this->parentField();
    if (parentField) parent = parentField->ownerObject();

    while (parent != NULL)
    {
        T* objectOfType = dynamic_cast<T*>(parent);
        if (objectOfType)
        {
            ancestor = objectOfType;
            return;
        }

        // Get next level parent

        PdmFieldHandle*  nextParentField = parent->parentField();
        
        if (nextParentField)
        {
            parent = nextParentField->ownerObject();
        }
        else
        {
            parent = NULL;
        }
    }
}

} // End of namespace caf

