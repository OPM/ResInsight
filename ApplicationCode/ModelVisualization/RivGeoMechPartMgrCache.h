#pragma once

#include "cvfObject.h"
#include <map>

class RivGeoMechPartMgr;

class RivGeoMechPartMgrCache : public cvf::Object
{
public:
    RivGeoMechPartMgrCache();
    ~RivGeoMechPartMgrCache();

    class Key
    {
    public:
        Key()
            : geometryType(-1), frameIndex(-1) 
        {}

        Key(unsigned short aGeometryType, int aFrameIndex) 
            : geometryType(aGeometryType), frameIndex(aFrameIndex) 
        {}

        void set(unsigned short aGeometryType, int aFrameIndex)
        {
            frameIndex = aFrameIndex;
            geometryType = aGeometryType;
        }

        int             frameIndex;
        unsigned short  geometryType;

        bool operator< (const Key& other) const
        {
            if (frameIndex != other.frameIndex)
            {
                return (frameIndex < other.frameIndex);
            }
            return (geometryType <  other.geometryType);
        }
    };

    bool                needsRegeneration   (const Key& key);
    void                scheduleRegeneration(const Key& key);
    void                generationFinished      (const Key& key);
    RivGeoMechPartMgr*  partMgr             (const Key& key);

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

