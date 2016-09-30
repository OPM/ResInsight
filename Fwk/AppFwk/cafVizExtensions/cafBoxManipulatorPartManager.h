
#pragma once

#include "cvfBase.h"
#include "cvfObject.h"

#include "cvfVector3.h"
#include "cvfCollection.h"
#include "cvfMatrix4.h"

namespace cvf {
    class ModelBasicList;
    class Part;
    class DrawableGeo;
    class Ray;
}

namespace caf {
    class BoxManipulatorGeometryGenerator;
};


namespace caf {


//==================================================================================================
//
//
//==================================================================================================
class BoxManipulatorPartManager : public cvf::Object
{
public:
    enum BoxCubeFace
    {
        BCF_X_POS,
        BCF_X_NEG,
        BCF_Y_POS,
        BCF_Y_NEG,
        BCF_Z_POS,
        BCF_Z_NEG
    };

    enum NavCubeFaceItem
    {
        BCFI_NONE,
        BCFI_CENTER
    };

public:
    BoxManipulatorPartManager();

    void    setOrigin(const cvf::Vec3d& origin);
    void    setSize(const cvf::Vec3d& size);
    void    originAndSize(cvf::Vec3d* origin, cvf::Vec3d* size);

    size_t  partIndexFromSourceInfo(const cvf::Part* part, const cvf::Vec3d& intersectionPoint);
    void    updateFromPartIndexAndRay(size_t partIndex, const cvf::Ray* ray);

    void    appendPartsToModel(cvf::ModelBasicList* model);

private:
    cvf::Vec3d  normalFromFace(BoxCubeFace face);

    void        createCubeGeos();
    void        createCubeFaceGeos(BoxCubeFace face, cvf::Vec3f p1, cvf::Vec3f p2, cvf::Vec3f p3, cvf::Vec3f p4);
    void        navCubeCornerPoints(cvf::Vec3f points[8]);
    

    void        clearAllGeometryAndParts();
    void        recreateAllGeometryAndParts();

    void        createBoundingBoxPart();
    void        createCubeParts();

    cvf::ref<cvf::DrawableGeo>              createQuadGeo(const cvf::Vec3f& v1, const cvf::Vec3f& v2, const cvf::Vec3f& v3, const cvf::Vec3f& v4);
    std::pair<BoxCubeFace, NavCubeFaceItem> navCubeItem(BoxCubeFace face, NavCubeFaceItem item) const;

    static bool closestPointOfTwoLines(const cvf::Vec3d& p1, const cvf::Vec3d& q1, const cvf::Vec3d& p2, const cvf::Vec3d& q2, cvf::Vec3d* closestPoint1, cvf::Vec3d* closestPoint2);

private:
    cvf::Collection<cvf::DrawableGeo>                       m_cubeGeos;                 // These arrays have the same length
    std::vector< std::pair<BoxCubeFace, NavCubeFaceItem> >  m_cubeItemType;             // These arrays have the same length
    cvf::Collection<cvf::Part>                              m_cubeParts;                // These arrays have the same length
    
    cvf::ref<cvf::Part>             m_boundingBoxPart;

    cvf::Vec3d          m_origin;
    cvf::Vec3d          m_size;

    cvf::Vec3d          m_initialPickPoint;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class BoxManipulatorSourceInfo : public cvf::Object
{
public:
    BoxManipulatorSourceInfo(BoxManipulatorPartManager::BoxCubeFace cubeFace, BoxManipulatorPartManager::NavCubeFaceItem cubeFaceItem)
        : m_cubeFace(cubeFace),
        m_cubeFaceItem(cubeFaceItem)
    {
    }

    BoxManipulatorPartManager::BoxCubeFace m_cubeFace;
    BoxManipulatorPartManager::NavCubeFaceItem m_cubeFaceItem;
};


}
