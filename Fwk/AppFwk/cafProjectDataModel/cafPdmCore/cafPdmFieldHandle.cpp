#include "cafPdmFieldHandle.h"

#include "cafPdmFieldCapability.h"

namespace caf
{

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
bool PdmFieldHandle::hasChildren() const
{
    return !children().empty();
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
/// This function is called to configure capabilities for the field handle. This can be used to avoid duplicated code
/// when setting options for *.uiCapability() and *.xmlCapability().
//--------------------------------------------------------------------------------------------------
void PdmFieldHandle::configureCapabilities()
{
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
bool PdmFieldHandle::hasPtrReferencedObjects() const
{
    std::vector<PdmObjectHandle*> ptrReffedObjs = ptrReferencedObjects();
    return !ptrReffedObjs.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldCapability*> PdmFieldHandle::capabilities() const
{
    std::vector<caf::PdmFieldCapability*> allCapabilities;

    for ( const auto& cap : m_capabilities )
    {
        allCapabilities.push_back( cap.first );
    }

    return allCapabilities;
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
