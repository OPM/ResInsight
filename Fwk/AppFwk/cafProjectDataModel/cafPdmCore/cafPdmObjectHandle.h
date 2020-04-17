#pragma once

#include "cafAssert.h"
#include "cafPdmBase.h"

#include <QString>

#include <set>
#include <vector>


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
    PdmObjectHandle()           { m_parentField = nullptr;  }
    virtual ~PdmObjectHandle();

    static QString classKeywordStatic();  // For PdmXmlFieldCap to be able to handle fields of PdmObjectHandle directly
    static std::vector<QString> classKeywordAliases();
    
    /// The registered fields contained in this PdmObject. 
    void                    fields(std::vector<PdmFieldHandle*>& fields) const;
    PdmFieldHandle*         findField(const QString& keyword) const;

    /// The field referencing this object as a child
    PdmFieldHandle*         parentField() const;

    /// Returns _this_ if _this_ is of requested type
    /// Traverses parents recursively and returns first parent of the requested type.
    template <typename T>
    void                    firstAncestorOrThisOfType(T*& ancestor) const;

    /// Traverses parents recursively and returns first parent of the requested type.
    /// Does NOT check _this_ object
    template <typename T>
    void                    firstAncestorOfType(T*& ancestor) const;

    /// Calls firstAncestorOrThisOfType, and asserts that a valid object is found 
    template <typename T>
    void                    firstAncestorOrThisOfTypeAsserted(T*& ancestor) const;

    /// Traverses all children recursively to find objects of the requested type. This object is also 
    /// included if it is of the requested type.
    template <typename T>
    void                    descendantsIncludingThisOfType(std::vector<T*>& descendants) const;

     // PtrReferences
    /// The PdmPtrField's containing pointers to this PdmObjecthandle 
    /// Use ownerObject() on the fieldHandle to get the PdmObjectHandle 
    void                    referringPtrFields(std::vector<PdmFieldHandle*>& fieldsReferringToMe) const;
    /// Convenience method to get the objects pointing to this field 
    void                    objectsWithReferringPtrFields(std::vector<PdmObjectHandle*>& objects) const;
    /// Convenience method to get the objects of specified type pointing to this field
    template <typename T>
    void                    objectsWithReferringPtrFieldsOfType(std::vector<T*>& objectsOfType) const;

    // Detach object from all referring fields
    void                    prepareForDelete();

    // Object capabilities
    void                    addCapability(PdmObjectCapability* capability, bool takeOwnership) { m_capabilities.push_back(std::make_pair(capability, takeOwnership)); }

    template <typename CapabilityType>
    CapabilityType* capability() const
    {
        for (size_t i = 0; i < m_capabilities.size(); ++i)
        {
            CapabilityType* capability = dynamic_cast<CapabilityType*>(m_capabilities[i].first);
            if (capability) return capability;
        }
        return nullptr;
    }

    PdmUiObjectHandle*  uiCapability() const;     // Implementation is in cafPdmUiObjectHandle.cpp
    PdmXmlObjectHandle* xmlCapability() const;    // Implementation is in cafPdmXmlObjectHandle.cpp

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
void PdmObjectHandle::firstAncestorOrThisOfType(T*& ancestor) const
{
    ancestor = nullptr;

    // Check if this matches the type

    const T* objectOfTypeConst = dynamic_cast<const T*>(this);
    if (objectOfTypeConst)
    {
        ancestor = const_cast<T*>(objectOfTypeConst);
        return;
    }

    this->firstAncestorOfType<T>(ancestor);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObjectHandle::firstAncestorOfType(T*& ancestor) const
{
    ancestor = nullptr;
    
    // Search parents for first type match
    PdmObjectHandle* parent = nullptr;
    PdmFieldHandle* parentField = this->parentField();
    if (parentField) parent = parentField->ownerObject();

    if (parent != nullptr)
    {
        parent->firstAncestorOrThisOfType<T>(ancestor);
    }
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObjectHandle::firstAncestorOrThisOfTypeAsserted(T*& ancestor) const
{
    firstAncestorOrThisOfType(ancestor);

    CAF_ASSERT(ancestor);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObjectHandle::descendantsIncludingThisOfType(std::vector<T*>& descendants) const
{
    const T* objectOfType = dynamic_cast<const T*>(this);
    if (objectOfType)
    {
        descendants.push_back(const_cast<T*>(objectOfType));
    }

    for (auto f : m_fields)
    {
        std::vector<PdmObjectHandle*> childObjects;
        f->childObjects(&childObjects);

        for (auto childObject : childObjects)
        {
            if (childObject)
            {
                childObject->descendantsIncludingThisOfType(descendants);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObjectHandle::objectsWithReferringPtrFieldsOfType(std::vector<T*>& objectsOfType) const
{
    std::vector<PdmObjectHandle*> objectsReferencingThis;
    this->objectsWithReferringPtrFields(objectsReferencingThis);

    for (auto object : objectsReferencingThis)
    {
        if (dynamic_cast<T*>(object))
        {
            objectsOfType.push_back(dynamic_cast<T*>(object));
        }
    }
}


} // End of namespace caf

