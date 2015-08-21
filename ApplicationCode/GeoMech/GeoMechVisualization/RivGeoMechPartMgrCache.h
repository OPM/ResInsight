#pragma once
#include <cstddef>
#include "cvfObject.h"
#include <map>
#include "RivCellSetEnum.h"

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

