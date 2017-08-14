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

#pragma once

#include "RivCellSetEnum.h"

#include "cvfBase.h"
#include "cvfObject.h"

#include <cstddef>
#include <map>

class RivGeoMechPartMgr;
class RivGeoMechPartMgrGeneratorInterface;

class RivGeoMechPartMgrCache : public cvf::Object
{
public:
    RivGeoMechPartMgrCache();
    ~RivGeoMechPartMgrCache();

    class Key
    {
    public:
        Key() : m_geometryType(-1), m_frameIndex(-1) {}

        Key(RivCellSetEnum aGeometryType, int aFrameIndex);

        void            set(RivCellSetEnum aGeometryType, int aFrameIndex);

        int             frameIndex()   const  { return m_frameIndex;}
        unsigned short  geometryType() const  { return m_geometryType; }

        bool            operator< (const Key& other) const;

    private:
        int             m_frameIndex;
        unsigned short  m_geometryType;
    };

    bool                isNeedingRegeneration(const Key& key) const;
    void                scheduleRegeneration (const Key& key);
    void                setGenerationFinished(const Key& key);
    RivGeoMechPartMgr*  partMgr              (const Key& key);

private:
    class CacheEntry
    {
    public:
        CacheEntry() : needsRegen(true) {}

        bool                        needsRegen;
        cvf::ref<RivGeoMechPartMgr> partMgr;
    };

    std::map<Key, CacheEntry > m_partMgrs;
};

