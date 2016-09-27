
#pragma once

#include "cvfBase.h"
#include "cvfObject.h"

#include "cvfVector3.h"
#include "cvfCollection.h"
#include "cvfMatrix4.h"
/*
#include "cvfArray.h"
#include "cvfBoundingBox.h"

*/
namespace cvf {
    class ModelBasicList;
    class Part;
    class DrawableGeo;
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
    enum NavCubeFace
    {
        NCF_X_POS,
        NCF_X_NEG,
        NCF_Y_POS,
        NCF_Y_NEG,
        NCF_Z_POS,
        NCF_Z_NEG
    };

    enum NavCubeFaceItem
    {
        NCFI_NONE,
        NCFI_CENTER,
        NCFI_BOTTOM_LEFT,
        NCFI_BOTTOM,
        NCFI_BOTTOM_RIGHT,
        NCFI_RIGHT,
        NCFI_TOP_RIGHT,
        NCFI_TOP,
        NCFI_TOP_LEFT,
        NCFI_LEFT
    };

    enum NavCubeItem
    {
        NCI_NONE,

        NCI_CORNER_XN_YN_ZN,
        NCI_CORNER_XP_YN_ZN,
        NCI_CORNER_XP_YP_ZN,
        NCI_CORNER_XN_YP_ZN,
        NCI_CORNER_XN_YN_ZP,
        NCI_CORNER_XP_YN_ZP,
        NCI_CORNER_XP_YP_ZP,
        NCI_CORNER_XN_YP_ZP,

        NCI_EDGE_YN_ZN,
        NCI_EDGE_XP_ZN,
        NCI_EDGE_YP_ZN,
        NCI_EDGE_XN_ZN,
        NCI_EDGE_YN_ZP,
        NCI_EDGE_XP_ZP,
        NCI_EDGE_YP_ZP,
        NCI_EDGE_XN_ZP,
        NCI_EDGE_XN_YN,
        NCI_EDGE_XP_YN,
        NCI_EDGE_XP_YP,
        NCI_EDGE_XN_YP,

        NCI_FACE_X_POS,
        NCI_FACE_X_NEG,
        NCI_FACE_Y_POS,
        NCI_FACE_Y_NEG,
        NCI_FACE_Z_POS,
        NCI_FACE_Z_NEG,

        NCI_ARROW_LEFT,
        NCI_ARROW_RIGHT,
        NCI_ARROW_TOP,
        NCI_ARROW_BOTTOM,
        NCI_HOME,
        NCI_ROTATE_CW,
        NCI_ROTATE_CCW
    };


public:
    BoxManipulatorPartManager();

    void setOrigin(const cvf::Mat4d& origin);
    void setSize(const cvf::Vec3d& size);

    void appendPartsToModel(cvf::ModelBasicList* model);

    size_t partIndexFromSourceInfo(cvf::Part* part);



private:
    void                createCubeGeos();
    void                createCubeFaceGeos(NavCubeFace face, cvf::Vec3f p1, cvf::Vec3f p2, cvf::Vec3f p3, cvf::Vec3f p4);
    cvf::ref<cvf::DrawableGeo>    createQuadGeo(const cvf::Vec3f& v1, const cvf::Vec3f& v2, const cvf::Vec3f& v3, const cvf::Vec3f& v4, const cvf::Vec2f& t1, const cvf::Vec2f& t2, const cvf::Vec2f& t3, const cvf::Vec2f& t4);
    void                navCubeCornerPoints(cvf::Vec3f points[8]);
    
    //NavCubeItem         navCubeItem(NavCubeFace face, NavCubeFaceItem item) const;
    std::pair<NavCubeFace, NavCubeFaceItem> navCubeItem(NavCubeFace face, NavCubeFaceItem item) const;

    void clearAllGeometryAndParts();
    void recreateAllGeometryAndParts();

    void createBoundingBoxPart();
    void createCubeParts();

private:
    cvf::Collection<cvf::DrawableGeo>                       m_cubeGeos;                 // These arrays have the same length
    std::vector<NavCubeFace>                                m_cubeGeoFace;              // These arrays have the same length
    std::vector< std::pair<NavCubeFace, NavCubeFaceItem> >  m_cubeItemType;             // These arrays have the same length

    cvf::Collection<cvf::Part>      m_cubeParts;                // These arrays have the same length
    cvf::ref<cvf::Part>             m_boundingBoxPart;

    cvf::ref<caf::BoxManipulatorGeometryGenerator> m_geometryGenerator;

    cvf::Mat4d          m_origin;
    cvf::Vec3d          m_size;
};






class BoxManipulatorSourceInfo : public cvf::Object
{
public:
    BoxManipulatorSourceInfo(BoxManipulatorPartManager::NavCubeFace cubeFace, BoxManipulatorPartManager::NavCubeFaceItem cubeFaceItem)
        : m_cubeFace(cubeFace),
        m_cubeFaceItem(cubeFaceItem)
    {
    }

    BoxManipulatorPartManager::NavCubeFace m_cubeFace;
    BoxManipulatorPartManager::NavCubeFaceItem m_cubeFaceItem;
};


}
