#include "RivGeoMechPartMgrCache.h"
#include "RivGeoMechPartMgr.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGeoMechPartMgrCache::RivGeoMechPartMgrCache()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGeoMechPartMgrCache::~RivGeoMechPartMgrCache()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RivGeoMechPartMgrCache::needsRegeneration(const Key& key)
{
    return m_partMgrs[key].needsRegen;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgrCache::scheduleRegeneration(const Key& key)
{
     m_partMgrs[key].needsRegen = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgrCache::generationFinished(const Key& key)
{
    m_partMgrs[key].needsRegen = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGeoMechPartMgr* RivGeoMechPartMgrCache::partMgr(const Key& key)
{
    CacheEntry& ce = m_partMgrs[key];
    if (ce.partMgr.isNull())
    {
        ce.partMgr = new RivGeoMechPartMgr;
    }

    return ce.partMgr.p();
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgrCache::Key::set(RivCellSetEnum aGeometryType, int aFrameIndex)
{
    m_frameIndex = aFrameIndex;
    m_geometryType = aGeometryType;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RivGeoMechPartMgrCache::Key::operator<(const Key& other) const
{
    if (m_frameIndex != other.m_frameIndex)
    {
        return (m_frameIndex < other.m_frameIndex);
    }
    return (m_geometryType <  other.m_geometryType);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGeoMechPartMgrCache::Key::Key(RivCellSetEnum aGeometryType, int aFrameIndex) 
: m_geometryType(aGeometryType), m_frameIndex(aFrameIndex)
{

}
