#include "cafPdmDefaultObjectFactory.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///  PdmObjectFactory implementations
//--------------------------------------------------------------------------------------------------

PdmDefaultObjectFactory* PdmDefaultObjectFactory::sm_singleton = nullptr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmDefaultObjectFactory::create( const QString& classNameKeyword )
{
    std::map<QString, PdmObjectCreatorBase*>::iterator entryIt;
    entryIt = m_factoryMap.find( classNameKeyword );
    if ( entryIt != m_factoryMap.end() )
    {
        return entryIt->second->create();
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> PdmDefaultObjectFactory::classKeywords() const
{
    std::vector<QString> names;

    for ( const auto& entry : m_factoryMap )
    {
        names.push_back( entry.first );
    }

    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmDefaultObjectFactory* PdmDefaultObjectFactory::instance()
{
    createSingleton();
    return sm_singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmDefaultObjectFactory::createSingleton()
{
    if ( !sm_singleton ) sm_singleton = new PdmDefaultObjectFactory;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmDefaultObjectFactory::deleteSingleton()
{
    if ( sm_singleton ) delete sm_singleton;
}

} // End of namespace caf
