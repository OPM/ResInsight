/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

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
bool RivGeoMechPartMgrCache::isNeedingRegeneration(const Key& key) const
{
    std::map<Key, CacheEntry >::const_iterator ceIt = m_partMgrs.find(key);
    if (ceIt != m_partMgrs.end())
    {
        return ceIt->second.needsRegen;
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgrCache::scheduleRegeneration(const Key& key)
{
    std::map<Key, CacheEntry >::iterator ceIt = m_partMgrs.find(key);
    if (ceIt != m_partMgrs.end())
    {
        ceIt->second.needsRegen = true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgrCache::setGenerationFinished(const Key& key)
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
