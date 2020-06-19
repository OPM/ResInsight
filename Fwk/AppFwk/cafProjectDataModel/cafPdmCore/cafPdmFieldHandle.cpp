#include "cafPdmFieldHandle.h"

#include "cafPdmFieldCapability.h"

namespace caf
{
#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmFieldHandle::assertValid() const
{
    if (m_keyword == "UNDEFINED")
    {
        std::cout << "PdmField: Detected use of non-initialized field. Did you forget to do CAF_PDM_InitField() on this field ?\n";
        return false;
    }

    if (!PdmXmlSerializable::isValidXmlElementName(m_keyword))
    {
        std::cout << "PdmField: The supplied keyword: \"" << m_keyword.toStdString() << "\" is an invalid XML element name, and will break your file format!\n";
        return false;
    }

    return true;
}
#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldHandle::setKeyword( const QString& keyword )
{
    m_keyword = keyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldHandle::hasChildObjects()
{
    std::vector<PdmObjectHandle*> children;
    this->childObjects( &children );
    return ( children.size() > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmFieldHandle::~PdmFieldHandle()
{
    for ( size_t i = 0; i < m_capabilities.size(); ++i )
    {
        if ( m_capabilities[i].second ) delete m_capabilities[i].first;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldHandle::matchesKeyword( const QString& keyword ) const
{
    if ( m_keyword == keyword ) return true;

    return matchesKeywordAlias( keyword );
}

//--------------------------------------------------------------------------------------------------
/// The class of the ownerObject() can be different to ownerClass().
/// This is because the ownerClass() may be a super-class to the instantiated owner object.
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* PdmFieldHandle::ownerObject()
{
    return m_ownerObject;
}

//--------------------------------------------------------------------------------------------------
/// Get the class in the class hierarchy the field actually belongs to.
/// This can be different to ownerObject's class, which may be a sub-class.
//--------------------------------------------------------------------------------------------------
QString PdmFieldHandle::ownerClass() const
{
    return m_ownerClass;
}

//--------------------------------------------------------------------------------------------------
/// Set the class in the class hierarchy the field actually belongs to.
/// This can be different to ownerObject's class, which may be a sub-class.
//--------------------------------------------------------------------------------------------------
void PdmFieldHandle::setOwnerClass( const QString& ownerClass )
{
    m_ownerClass = ownerClass;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldHandle::hasPtrReferencedObjects()
{
    std::vector<PdmObjectHandle*> ptrReffedObjs;
    this->ptrReferencedObjects( &ptrReffedObjs );
    return ( ptrReffedObjs.size() > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldHandle::registerKeywordAlias( const QString& alias )
{
    m_keywordAliases.push_back( alias );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldHandle::matchesKeywordAlias( const QString& keyword ) const
{
    for ( const QString& alias : m_keywordAliases )
    {
        if ( alias == keyword ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> PdmFieldHandle::keywordAliases() const
{
    return m_keywordAliases;
}

// These two functions can be used when PdmCore is used standalone without PdmUi/PdmXml
/*
PdmUiFieldHandle* PdmFieldHandle::uiCapability()
{
    return NULL;
}

PdmXmlFieldHandle* PdmFieldHandle::xmlCapability()
{
    return NULL;
}
*/

} // End of namespace caf
