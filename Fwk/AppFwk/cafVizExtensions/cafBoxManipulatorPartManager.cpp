
#include "cafBoxManipulatorPartManager.h"

#include "cvfBoxGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cafBoxManipulatorGeometryGenerator.h"
#include "cafEffectGenerator.h"
#include "cvfModelBasicList.h"


using namespace cvf;

namespace caf {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoxManipulatorPartManager::BoxManipulatorPartManager()
{
    m_geometryGenerator = new caf::BoxManipulatorGeometryGenerator;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::setOrigin(const cvf::Mat4d& origin)
{
    m_origin = origin;

    m_geometryGenerator->setOrigin(origin);

    clearAllGeometryAndParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::setSize(const cvf::Vec3d& size)
{
    m_size = size;

    m_geometryGenerator->setSize(size);

    clearAllGeometryAndParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::appendPartsToModel(cvf::ModelBasicList* model)
{
    if (m_boundingBoxPart.isNull())
    {
        recreateAllGeometryAndParts();
    }

    CVF_ASSERT(m_boundingBoxPart.notNull());
    model->addPart(m_boundingBoxPart.p());

    for (size_t i = 0; i < m_cubeParts.size(); i++)
    {
        model->addPart(m_cubeParts.at(i));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t BoxManipulatorPartManager::partIndexFromSourceInfo(cvf::Part* candidatePart)
{
    if (!candidatePart) return cvf::UNDEFINED_SIZE_T;

    BoxManipulatorSourceInfo* candidateSourceInfo = dynamic_cast<BoxManipulatorSourceInfo*>(candidatePart->sourceInfo());
    if (!candidateSourceInfo) return cvf::UNDEFINED_SIZE_T;

    for (size_t i = 0; i < m_cubeParts.size(); i++)
    {
        cvf::Part* part = m_cubeParts.at(i);
        BoxManipulatorSourceInfo* si = static_cast<BoxManipulatorSourceInfo*>(part->sourceInfo());

        if (si->m_cubeFace == candidateSourceInfo->m_cubeFace &&
            si->m_cubeFaceItem == candidateSourceInfo->m_cubeFaceItem)
        {
            return i;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::createCubeGeos()
{
    Vec3f cp[8];
    navCubeCornerPoints(cp);

    createCubeFaceGeos(NCF_Y_NEG, cp[0], cp[1], cp[5], cp[4]);
    createCubeFaceGeos(NCF_Y_POS, cp[2], cp[3], cp[7], cp[6]);

    createCubeFaceGeos(NCF_Z_POS, cp[4], cp[5], cp[6], cp[7]);
    createCubeFaceGeos(NCF_Z_NEG, cp[3], cp[2], cp[1], cp[0]);

    createCubeFaceGeos(NCF_X_NEG, cp[3], cp[0], cp[4], cp[7]);
    createCubeFaceGeos(NCF_X_POS, cp[1], cp[2], cp[6], cp[5]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::createCubeFaceGeos(NavCubeFace face, cvf::Vec3f p1, cvf::Vec3f p2, cvf::Vec3f p3, cvf::Vec3f p4)
{
    Vec2f t1(0, 0);
    Vec2f t2(1, 0);
    Vec2f t3(1, 1);
    Vec2f t4(0, 1);

    float fCornerFactor = 0.175f;
    float fOneMinusCF = 1.0f - fCornerFactor;
    Vec3f p12 = p1 + (p2 - p1)*fCornerFactor;   Vec2f t12(fCornerFactor, 0);
    Vec3f p14 = p1 + (p4 - p1)*fCornerFactor;   Vec2f t14(0, fCornerFactor);
    Vec3f pi1 = p1 + (p12 - p1) + (p14 - p1);   Vec2f ti1(fCornerFactor, fCornerFactor);

    Vec3f p21 = p2 + (p1 - p2)*fCornerFactor;   Vec2f t21(fOneMinusCF, 0);
    Vec3f p23 = p2 + (p3 - p2)*fCornerFactor;   Vec2f t23(1.0, fCornerFactor);
    Vec3f pi2 = p2 + (p21 - p2) + (p23 - p2);   Vec2f ti2(fOneMinusCF, fCornerFactor);

    Vec3f p32 = p3 + (p2 - p3)*fCornerFactor;   Vec2f t32(1.0, fOneMinusCF);
    Vec3f p34 = p3 + (p4 - p3)*fCornerFactor;   Vec2f t34(fOneMinusCF, 1.0);
    Vec3f pi3 = p3 + (p32 - p3) + (p34 - p3);   Vec2f ti3(fOneMinusCF, fOneMinusCF);

    Vec3f p41 = p4 + (p1 - p4)*fCornerFactor;   Vec2f t41(0, fOneMinusCF);
    Vec3f p43 = p4 + (p3 - p4)*fCornerFactor;   Vec2f t43(fCornerFactor, 1.0);
    Vec3f pi4 = p4 + (p41 - p4) + (p43 - p4);   Vec2f ti4(fCornerFactor, fOneMinusCF);

/*
    // Bottom left
    m_cubeItemType.push_back(navCubeItem(face, NCFI_BOTTOM_LEFT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p1, p12, pi1, p14, t1, t12, ti1, t14).p());

    // Bottom right
    m_cubeItemType.push_back(navCubeItem(face, NCFI_BOTTOM_RIGHT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p2, p23, pi2, p21, t2, t23, ti2, t21).p());

    // Top right
    m_cubeItemType.push_back(navCubeItem(face, NCFI_TOP_RIGHT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p3, p34, pi3, p32, t3, t34, ti3, t32).p());

    // Top left
    m_cubeItemType.push_back(navCubeItem(face, NCFI_TOP_LEFT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p4, p41, pi4, p43, t4, t41, ti4, t43).p());

    // Bottom
    m_cubeItemType.push_back(navCubeItem(face, NCFI_BOTTOM));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p12, p21, pi2, pi1, t12, t21, ti2, ti1).p());

    // Top
    m_cubeItemType.push_back(navCubeItem(face, NCFI_TOP));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p34, p43, pi4, pi3, t34, t43, ti4, ti3).p());

    // Right
    m_cubeItemType.push_back(navCubeItem(face, NCFI_RIGHT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p23, p32, pi3, pi2, t23, t32, ti3, ti2).p());

    // Left
    m_cubeItemType.push_back(navCubeItem(face, NCFI_LEFT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p41, p14, pi1, pi4, t41, t14, ti1, ti4).p());
*/

    // Inner part
    m_cubeItemType.push_back(navCubeItem(face, NCFI_CENTER));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(pi1, pi2, pi3, pi4, ti1, ti2, ti3, ti4).p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> BoxManipulatorPartManager::createQuadGeo(const cvf::Vec3f& v1, const cvf::Vec3f& v2, const cvf::Vec3f& v3, const cvf::Vec3f& v4, const cvf::Vec2f& t1, const cvf::Vec2f& t2, const cvf::Vec2f& t3, const cvf::Vec2f& t4)
{
    ref<DrawableGeo> geo = new DrawableGeo;

    ref<Vec3fArray> vertexArray = new Vec3fArray(4);
    vertexArray->set(0, v1);
    vertexArray->set(1, v2);
    vertexArray->set(2, v3);
    vertexArray->set(3, v4);

    ref<Vec2fArray> textureCoordArray = new Vec2fArray(4);
    textureCoordArray->set(0, t1);
    textureCoordArray->set(1, t2);
    textureCoordArray->set(2, t3);
    textureCoordArray->set(3, t4);

    geo->setVertexArray(vertexArray.p());
    geo->setTextureCoordArray(textureCoordArray.p());

    ref<cvf::UShortArray> indices = new cvf::UShortArray(6);
    indices->set(0, 0);
    indices->set(1, 1);
    indices->set(2, 2);
    indices->set(3, 0);
    indices->set(4, 2);
    indices->set(5, 3);

    ref<cvf::PrimitiveSetIndexedUShort> primSet = new cvf::PrimitiveSetIndexedUShort(cvf::PT_TRIANGLES);
    primSet->setIndices(indices.p());
    geo->addPrimitiveSet(primSet.p());
    geo->computeNormals();

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::navCubeCornerPoints(cvf::Vec3f points[8])
{
    Vec3f min(m_origin.translation());
    Vec3f max(m_origin.translation() + m_size);

    points[0].set(min.x(), min.y(), min.z());
    points[1].set(max.x(), min.y(), min.z());
    points[2].set(max.x(), max.y(), min.z());
    points[3].set(min.x(), max.y(), min.z());
    points[4].set(min.x(), min.y(), max.z());
    points[5].set(max.x(), min.y(), max.z());
    points[6].set(max.x(), max.y(), max.z());
    points[7].set(min.x(), max.y(), max.z());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<BoxManipulatorPartManager::NavCubeFace, BoxManipulatorPartManager::NavCubeFaceItem> BoxManipulatorPartManager::navCubeItem(NavCubeFace face, NavCubeFaceItem faceItem) const
{
    return std::make_pair(face, faceItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::clearAllGeometryAndParts()
{
    m_boundingBoxPart = nullptr;
    m_cubeGeos.clear();
    m_cubeGeoFace.clear();
    m_cubeItemType.clear();
    m_cubeParts.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::recreateAllGeometryAndParts()
{
    createBoundingBoxPart();
    createCubeGeos();
    createCubeParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::createBoundingBoxPart()
{
    m_boundingBoxPart = nullptr;

    cvf::ref<cvf::DrawableGeo> geoMesh = m_geometryGenerator->createBoundingBoxMeshDrawable();
    if (geoMesh.notNull())
    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("Box Manipulator Mesh");
        part->setDrawable(geoMesh.p());

        part->updateBoundingBox();
        //                     part->setEnableMask(meshFaultBit);
        //                     part->setPriority(priMesh);

        cvf::ref<cvf::Effect> eff;
        caf::MeshEffectGenerator effectGenerator(cvf::Color3::WHITE);
        eff = effectGenerator.generateCachedEffect();
        part->setEffect(eff.p());

        m_boundingBoxPart = part;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::createCubeParts()
{
    for (size_t i = 0; i < m_cubeGeos.size(); i++)
    {
        cvf::DrawableGeo* geoMesh = m_cubeGeos.at(i);
        if (geoMesh)
        {
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Box Manipulator Mesh");
            part->setDrawable(geoMesh);

            part->updateBoundingBox();
            //                     part->setEnableMask(meshFaultBit);
            //                     part->setPriority(priMesh);


            caf::SurfaceEffectGenerator surfaceGen(cvf::Color3::GREEN, caf::PO_1);
            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();
            part->setEffect(eff.p());

            auto pair = m_cubeItemType[i];
            BoxManipulatorSourceInfo* sourceInfo = new BoxManipulatorSourceInfo(pair.first, pair.second);
            part->setSourceInfo(sourceInfo);

            m_cubeParts.push_back(part.p());
        }
    }
}

} // namespace cvf

