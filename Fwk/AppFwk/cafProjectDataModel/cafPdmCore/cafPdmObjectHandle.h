#pragma once

#include "cafAssert.h"
#include "cafPdmBase.h"
#include "cafSignal.h"

#include <QString>

#include <map>
#include <set>
#include <vector>

class QMimeData;

namespace caf
{
class PdmObjectCapability;
class PdmFieldHandle;
class PdmUiObjectHandle;
class PdmXmlObjectHandle;
class PdmChildArrayFieldHandle;

//==================================================================================================
/// The base class of all objects
//==================================================================================================
class PdmObjectHandle : public SignalObserver, public SignalEmitter
{
public:
    PdmObjectHandle();
    virtual ~PdmObjectHandle();

    static QString classKeywordStatic(); // For PdmXmlFieldCap to be able to handle fields of PdmObjectHandle directly
    static std::vector<QString> classKeywordAliases();

    // Function template to call the virtual copy function
    template <typename T>
    [[nodiscard]] T* copyObject() const;

    /// The registered fields contained in this PdmObject.
    [[nodiscard]] std::vector<PdmFieldHandle*> fields() const;
    [[nodiscard]] PdmFieldHandle*              findField( const QString& keyword ) const;

    /// The field referencing this object as a child
    [[nodiscard]] PdmFieldHandle* parentField() const;

    /// Returns _this_ if _this_ is of requested type
    /// Traverses parents recursively and returns first parent of the requested type.
    template <typename T>
    [[nodiscard]] T* firstAncestorOrThisOfType() const;

    /// Traverses parents recursively and returns first parent of the requested type.
    /// Does NOT check _this_ object
    template <typename T>
    [[nodiscard]] T* firstAncestorOfType() const;

    /// Calls firstAncestorOrThisOfType, and asserts that a valid object is found
    template <typename T>
    [[nodiscard]] T* firstAncestorOrThisOfTypeAsserted() const;

    template <typename T>
    [[nodiscard]] std::vector<T*> allAncestorsOfType() const;

    template <typename T>
    [[nodiscard]] std::vector<T*> allAncestorsOrThisOfType() const;

    /// Traverses all children recursively to find objects of the requested type. This object is also
    /// included if it is of the requested type.
    template <typename T>
    [[nodiscard]] std::vector<T*> descendantsIncludingThisOfType() const;

    /// Traverses all children recursively to find objects of the requested type. This object is NOT
    /// included if it is of the requested type.
    template <typename T>
    [[nodiscard]] std::vector<T*> descendantsOfType() const;

    // PtrReferences
    /// The PdmPtrField's containing pointers to this PdmObjecthandle
    /// Use ownerObject() on the fieldHandle to get the PdmObjectHandle
    [[nodiscard]] std::vector<PdmFieldHandle*> referringPtrFields() const;

    /// Convenience method to get the objects pointing to this field
    [[nodiscard]] std::vector<PdmObjectHandle*> objectsWithReferringPtrFields() const;

    /// Convenience method to get the objects of specified type pointing to this field
    template <typename T>
    [[nodiscard]] std::vector<T*> objectsWithReferringPtrFieldsOfType() const;

    // Detach object from all referring fields
    void prepareForDelete();

    // Object capabilities
    void addCapability( PdmObjectCapability* capability, bool takeOwnership );

    template <typename CapabilityType>
    CapabilityType* capability() const;

    PdmUiObjectHandle*  uiCapability() const; // Implementation is in cafPdmUiObjectHandle.cpp
    PdmXmlObjectHandle* xmlCapability() const; // Implementation is in cafPdmXmlObjectHandle.cpp

    // Validation
    [[nodiscard]] virtual std::map<QString, QString> validate( const QString& configName = "" ) const;
    [[nodiscard]] bool                               isValid( const QString& configName = "" ) const;

    virtual void setDeletable( bool isDeletable );
    virtual bool isDeletable() const;
    virtual void onChildDeleted( PdmChildArrayFieldHandle*           childArray,
                                 std::vector<caf::PdmObjectHandle*>& referringObjects );

    virtual void onChildAdded( caf::PdmFieldHandle* containerForNewObject ) {};
    virtual void onChildrenUpdated( PdmChildArrayFieldHandle*           childArray,
                                    std::vector<caf::PdmObjectHandle*>& updatedObjects ) {};

    virtual void
        handleDroppedMimeData( const QMimeData* data, Qt::DropAction action, caf::PdmFieldHandle* destinationField ) {};

    virtual void migrateFieldContent( QString& fieldContent, PdmFieldHandle* fieldHandle ) {};

protected:
    void addField( PdmFieldHandle* field, const QString& keyword );

    // Virtual method used to copy objects. The implementation is in PdmObject
    [[nodiscard]] virtual PdmObjectHandle* doCopyObject() const;

private:
    PDM_DISABLE_COPY_AND_ASSIGN( PdmObjectHandle );

    // Fields
    std::vector<PdmFieldHandle*>       m_fields;
    std::map<QString, PdmFieldHandle*> m_fieldKeywords;

    // Capabilities
    std::vector<std::pair<PdmObjectCapability*, bool>> m_capabilities;

    // Child/Parent Relationships
    void setAsParentField( PdmFieldHandle* parentField );
    void removeAsParentField( PdmFieldHandle* parentField );
    void disconnectObserverFromAllSignals( SignalObserver* observer );

    PdmFieldHandle* m_parentField;

    // PtrReferences
    void                           addReferencingPtrField( PdmFieldHandle* fieldReferringToMe );
    void                           removeReferencingPtrField( PdmFieldHandle* fieldReferringToMe );
    std::multiset<PdmFieldHandle*> m_referencingPtrFields;

    // Give access to set/removeAsParentField
    template <class T>
    friend class PdmChildArrayField;
    template <class T>
    friend class PdmChildField;
    template <class T>
    friend class PdmPtrArrayField;
    template <class T>
    friend class PdmPtrField;
    template <class T>
    friend class PdmField; // For backwards compatibility layer

    template <class T>
    friend class PdmFieldXmlCap;

    // Support system for PdmPointer
    friend class PdmPointerImpl;
    std::set<PdmObjectHandle**> m_pointersReferencingMe;

    bool m_isDeletable;

    // m_isInsideInitAfterRead is used to detect if resolveReferencesRecursively() is called from initAfterRead()
    // Use friend class to avoid public noise
    friend class PdmXmlObjectHandle;
    bool m_isInsideInitAfterRead;

    mutable PdmUiObjectHandle* m_uiCapability;
};
} // namespace caf

#include "cafPdmFieldHandle.h"

namespace caf
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] T* PdmObjectHandle::copyObject() const
{
    return dynamic_cast<T*>( doCopyObject() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] T* PdmObjectHandle::firstAncestorOrThisOfType() const
{
    static_assert( !std::is_pointer<T>::value );

    // If compilation error occurs, include of header file for type T might be missing in calling
    // code resulting in invalid dynamic_cast

    auto objectOfTypeConst = dynamic_cast<const T*>( this );
    if ( objectOfTypeConst )
    {
        return const_cast<T*>( objectOfTypeConst );
    }

    return firstAncestorOfType<T>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] T* PdmObjectHandle::firstAncestorOfType() const
{
    static_assert( !std::is_pointer<T>::value );

    // Search parents for first type match
    PdmObjectHandle* parent      = nullptr;
    PdmFieldHandle*  parentField = this->parentField();
    if ( parentField ) parent = parentField->ownerObject();

    if ( parent != nullptr )
    {
        return parent->firstAncestorOrThisOfType<T>();
    }

    return nullptr;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] T* PdmObjectHandle::firstAncestorOrThisOfTypeAsserted() const
{
    static_assert( !std::is_pointer<T>::value );

    auto ancestor = firstAncestorOrThisOfType<T>();
    CAF_ASSERT( ancestor );

    return ancestor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] std::vector<T*> PdmObjectHandle::allAncestorsOfType() const
{
    static_assert( !std::is_pointer<T>::value );

    std::vector<T*> ancestors;
    T*              firstAncestor = firstAncestorOfType<T>();
    if ( firstAncestor )
    {
        ancestors.push_back( firstAncestor );
        auto other = firstAncestor->template allAncestorsOfType<T>();
        ancestors.insert( ancestors.end(), other.begin(), other.end() );
    }

    return ancestors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] std::vector<T*> PdmObjectHandle::allAncestorsOrThisOfType() const
{
    static_assert( !std::is_pointer<T>::value );

    std::vector<T*> ancestors;

    T* firstAncestorOrThis = firstAncestorOrThisOfType<T>();
    if ( firstAncestorOrThis )
    {
        ancestors.push_back( firstAncestorOrThis );
        auto other = firstAncestorOrThis->template allAncestorsOfType<T>();
        ancestors.insert( ancestors.end(), other.begin(), other.end() );
    }

    return ancestors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] std::vector<T*> PdmObjectHandle::descendantsIncludingThisOfType() const
{
    static_assert( !std::is_pointer<T>::value );

    std::vector<T*> descendants;
    const T*        objectOfType = dynamic_cast<const T*>( this );
    if ( objectOfType )
    {
        descendants.push_back( const_cast<T*>( objectOfType ) );
    }

    auto other = descendantsOfType<T>();
    descendants.insert( descendants.end(), other.begin(), other.end() );

    return descendants;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] std::vector<T*> PdmObjectHandle::descendantsOfType() const
{
    static_assert( !std::is_pointer<T>::value );

    std::vector<T*> descendants;

    for ( auto f : m_fields )
    {
        std::vector<PdmObjectHandle*> childObjects = f->children();

        for ( auto childObject : childObjects )
        {
            if ( childObject )
            {
                auto other = childObject->descendantsIncludingThisOfType<T>();
                descendants.insert( descendants.end(), other.begin(), other.end() );
            }
        }
    }

    return descendants;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] std::vector<T*> PdmObjectHandle::objectsWithReferringPtrFieldsOfType() const
{
    static_assert( !std::is_pointer<T>::value );

    std::vector<T*> objectsOfType;

    std::vector<PdmObjectHandle*> objectsReferencingThis = objectsWithReferringPtrFields();
    for ( auto object : objectsReferencingThis )
    {
        if ( dynamic_cast<T*>( object ) )
        {
            objectsOfType.push_back( dynamic_cast<T*>( object ) );
        }
    }

    return objectsOfType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename CapabilityType>
CapabilityType* PdmObjectHandle::capability() const
{
    for ( auto capability : m_capabilities )
    {
        if ( auto capabilityType = dynamic_cast<CapabilityType*>( capability.first ) ) return capabilityType;
    }
    return nullptr;
}

} // End of namespace caf
