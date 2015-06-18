#pragma once
#include <cstddef>
#include "cvfObject.h"
#include <map>
#include "RivCellSetEnum.h"

class RivGeoMechPartMgr;

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

    bool                needsRegeneration    (const Key& key);
    void                scheduleRegeneration (const Key& key);
    void                generationFinished   (const Key& key);
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

