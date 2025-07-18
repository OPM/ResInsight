
#include "cafPdmObjectHandle.h"

#include "cafAssert.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObjectCapability.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle::PdmObjectHandle()
{
    m_parentField  = nullptr;
    m_isDeletable  = false;
    m_uiCapability = nullptr;

    m_isInsideInitAfterRead = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle::~PdmObjectHandle()
{
    this->prepareForDelete();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmObjectHandle::classKeywordStatic()
{
    return classKeywordAliases().front();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> PdmObjectHandle::classKeywordAliases()
{
    return { QString( "PdmObjectHandle" ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
[[nodiscard]] std::vector<PdmFieldHandle*> PdmObjectHandle::fields() const
{
    return m_fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::setAsParentField( PdmFieldHandle* parentField )
{
    CAF_ASSERT( m_parentField == nullptr );

    m_parentField = parentField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::removeAsParentField( PdmFieldHandle* parentField )
{
    CAF_ASSERT( m_parentField == parentField );

    if ( parentField ) disconnectObserverFromAllSignals( parentField->ownerObject() );

    m_parentField = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::disconnectObserverFromAllSignals( SignalObserver* observer )
{
    if ( observer )
    {
        for ( auto emittedSignal : emittedSignals() )
        {
            emittedSignal->disconnect( observer );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::addReferencingPtrField( PdmFieldHandle* fieldReferringToMe )
{
    if ( fieldReferringToMe != nullptr ) m_referencingPtrFields.insert( fieldReferringToMe );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::removeReferencingPtrField( PdmFieldHandle* fieldReferringToMe )
{
    if ( fieldReferringToMe != nullptr )
    {
        disconnectObserverFromAllSignals( fieldReferringToMe->ownerObject() );
        m_referencingPtrFields.erase( fieldReferringToMe );
    }
}

//--------------------------------------------------------------------------------------------------
/// Appends pointers to all the PdmPtrFields containing a pointer to this object.
/// As the PdmPtrArrayFields can hold several pointers to the same object, the returned vector can
/// contain multiple pointers to the same field.
//--------------------------------------------------------------------------------------------------
[[nodiscard]] std::vector<PdmFieldHandle*> PdmObjectHandle::referringPtrFields() const
{
    std::vector<PdmFieldHandle*> fieldsReferringToMe;

    std::multiset<PdmFieldHandle*>::const_iterator it;
    for ( it = m_referencingPtrFields.begin(); it != m_referencingPtrFields.end(); ++it )
    {
        fieldsReferringToMe.push_back( *it );
    }

    return fieldsReferringToMe;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
[[nodiscard]] std::vector<PdmObjectHandle*> PdmObjectHandle::objectsWithReferringPtrFields() const
{
    std::vector<PdmObjectHandle*> objects;

    std::vector<caf::PdmFieldHandle*> ptrFields = referringPtrFields();
    for ( size_t i = 0; i < ptrFields.size(); i++ )
    {
        objects.push_back( ptrFields[i]->ownerObject() );
    }

    return objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::prepareForDelete()
{
    m_parentField  = nullptr;
    m_uiCapability = nullptr;

    for ( size_t i = 0; i < m_capabilities.size(); ++i )
    {
        if ( m_capabilities[i].second ) delete m_capabilities[i].first;
    }

    // Set all guarded pointers pointing to this to NULL
    std::set<PdmObjectHandle**>::iterator it;
    for ( it = m_pointersReferencingMe.begin(); it != m_pointersReferencingMe.end(); ++it )
    {
        ( **it ) = nullptr;
    }

    m_capabilities.clear();
    m_referencingPtrFields.clear();
    m_pointersReferencingMe.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::addCapability( PdmObjectCapability* capability, bool takeOwnership )
{
    m_capabilities.push_back( std::make_pair( capability, takeOwnership ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::addField( PdmFieldHandle* field, const QString& keyword )
{
    field->m_ownerObject = this;

    CAF_ASSERT( !keyword.isEmpty() );

    auto it = m_fieldKeywords.find( keyword );
    if ( it != m_fieldKeywords.end() )
    {
        // This should never happen, as the keyword is unique.
        CAF_ASSERT( false );
    }

    field->setKeyword( keyword );
    m_fields.push_back( field );
    m_fieldKeywords[keyword] = field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* PdmObjectHandle::doCopyObject() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
[[nodiscard]] PdmFieldHandle* PdmObjectHandle::findField( const QString& keyword ) const
{
    auto fieldIt = m_fieldKeywords.find( keyword );
    if ( fieldIt != m_fieldKeywords.end() )
    {
        return fieldIt->second;
    }

    std::vector<PdmFieldHandle*> fields = this->fields();
    for ( const auto field : fields )
    {
        if ( field->matchesKeyword( keyword ) )
        {
            return field;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
[[nodiscard]] PdmFieldHandle* PdmObjectHandle::parentField() const
{
    return m_parentField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::setDeletable( bool isDeletable )
{
    m_isDeletable = isDeletable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmObjectHandle::isDeletable() const
{
    return m_isDeletable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::onChildDeleted( PdmChildArrayFieldHandle*           childArray,
                                      std::vector<caf::PdmObjectHandle*>& referringObjects )
{
}

// These two functions can be used when PdmCore is used standalone without PdmUi/PdmXml
/*
PdmUiObjectHandle* PdmObjectHandle::uiCapability()
{
return NULL;
}

PdmXmlObjectHandle* PdmObjectHandle::xmlCapability()
{
return NULL;
}
*/

} // End namespace caf
