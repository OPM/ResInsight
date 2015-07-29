#include "cafPdmDefaultObjectFactory.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///  PdmObjectFactory implementations
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle * PdmDefaultObjectFactory::create(const QString& classNameKeyword)
{
    std::map<QString, PdmObjectCreatorBase*>::iterator entryIt;
    entryIt = m_factoryMap.find(classNameKeyword);
    if (entryIt != m_factoryMap.end())
    {
        return entryIt->second->create();
    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmDefaultObjectFactory * PdmDefaultObjectFactory::instance()
{
    static PdmDefaultObjectFactory* fact = new PdmDefaultObjectFactory;
    return fact;
}

} // End of namespace caf
