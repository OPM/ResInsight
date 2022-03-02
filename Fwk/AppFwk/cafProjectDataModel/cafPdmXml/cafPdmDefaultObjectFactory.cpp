#include "cafPdmDefaultObjectFactory.h"

#include <set>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///  PdmObjectFactory implementations
//--------------------------------------------------------------------------------------------------

PdmDefaultObjectFactory* PdmDefaultObjectFactory::sm_singleton = nullptr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmDefaultObjectFactory::~PdmDefaultObjectFactory()
{
    // Each keyword alias is connected to the one creator object, so create a set of unique creator objects
    std::set<PdmObjectCreatorBase*> uniqueCreators;

    for ( const auto& f : m_factoryMap )
    {
        uniqueCreators.insert( f.second );
    }

    for ( auto f : uniqueCreators )
    {
        if ( f ) delete f;
    }

    m_factoryMap.clear();
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
    sm_singleton = nullptr;
}

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

    return nullptr;
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

} // End of namespace caf
