
#include "cafBoxManipulatorPartManager.h"

#include "cafBoxManipulatorGeometryGenerator.h"
#include "cafEffectGenerator.h"

#include "cvfBoxGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cvfRay.h"


using namespace cvf;

namespace caf {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoxManipulatorPartManager::BoxManipulatorPartManager()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::setOrigin(const cvf::Vec3d& origin)
{
    m_origin = origin;

    clearAllGeometryAndParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::setSize(const cvf::Vec3d& size)
{
    m_size = size;

    clearAllGeometryAndParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::originAndSize(cvf::Vec3d* origin, cvf::Vec3d* size)
{
    *origin = m_origin;
    *size = m_size;
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
size_t BoxManipulatorPartManager::partIndexFromSourceInfo(const cvf::Part* candidatePart, const cvf::Vec3d& intersectionPoint)
{
    if (!candidatePart) return cvf::UNDEFINED_SIZE_T;

    const cvf::Object* siConstObj = candidatePart->sourceInfo();
    cvf::Object* siObj = const_cast<cvf::Object*>(siConstObj);

    BoxManipulatorSourceInfo* candidateSourceInfo = dynamic_cast<BoxManipulatorSourceInfo*>(siObj);
    if (!candidateSourceInfo) return cvf::UNDEFINED_SIZE_T;

    for (size_t i = 0; i < m_cubeParts.size(); i++)
    {
        cvf::Part* part = m_cubeParts.at(i);
        BoxManipulatorSourceInfo* si = static_cast<BoxManipulatorSourceInfo*>(part->sourceInfo());

        if (si->m_cubeFace == candidateSourceInfo->m_cubeFace &&
            si->m_cubeFaceItem == candidateSourceInfo->m_cubeFaceItem)
        {
            m_initialPickPoint = intersectionPoint;
            
            return i;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::updateFromPartIndexAndRay(size_t partIndex, const cvf::Ray* ray)
{
    BoxCubeFace face = m_cubeItemType[partIndex].first;
    cvf::Vec3d faceDir = normalFromFace(face);

    cvf::Vec3d closestPoint1;
    cvf::Vec3d closestPoint2;
    BoxManipulatorPartManager::closestPointOfTwoLines(ray->origin(), ray->origin() + ray->direction(), m_initialPickPoint, m_initialPickPoint + faceDir, &closestPoint1, &closestPoint2);

    cvf::Vec3d newOrigin = m_origin;
    cvf::Vec3d newSize = m_size;

    switch (face)
    {
    case caf::BoxManipulatorPartManager::BCF_X_POS:
        newSize.x() = CVF_MAX(0.0, closestPoint2.x() - m_origin.x());
        break;
    case caf::BoxManipulatorPartManager::BCF_X_NEG:
        if (m_size.x() - (closestPoint2.x() - m_origin.x()) > 0.0)
        {
            newOrigin.x() = closestPoint2.x();
            newSize.x() = m_size.x() - (closestPoint2.x() - m_origin.x());
        }
        else
        {
            newOrigin.x() = m_origin.x() + m_size.x();
            newSize.x() = 0.0;
        }
        break;
    case caf::BoxManipulatorPartManager::BCF_Y_POS:
        newSize.y() = CVF_MAX(0.0, closestPoint2.y() - m_origin.y());
        break;
    case caf::BoxManipulatorPartManager::BCF_Y_NEG:
        if (m_size.y() - (closestPoint2.y() - m_origin.y()) > 0.0)
        {
            newOrigin.y() = closestPoint2.y();
            newSize.y() = m_size.y() - (closestPoint2.y() - m_origin.y());
        }
        else
        {
            newOrigin.y() = m_origin.y() + m_size.y();
            newSize.y() = 0.0;
        }
        break;
    case caf::BoxManipulatorPartManager::BCF_Z_POS:
        newSize.z() = CVF_MAX(0.0, closestPoint2.z() - m_origin.z());
        break;
    case caf::BoxManipulatorPartManager::BCF_Z_NEG:
        if (m_size.z() - (closestPoint2.z() - m_origin.z()) > 0.0)
        {
            newOrigin.z() = closestPoint2.z();
            newSize.z() = m_size.z() - (closestPoint2.z() - m_origin.z());
        }
        else
        {
            newOrigin.z() = m_origin.z() + m_size.z();
            newSize.z() = 0.0;
        }
        break;
    default:
        break;
    }

    setOrigin(newOrigin);
    setSize(newSize);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d BoxManipulatorPartManager::normalFromFace(BoxCubeFace face)
{
    switch (face)
    {
    case caf::BoxManipulatorPartManager::BCF_X_POS:
        return cvf::Vec3d::X_AXIS;
        break;
    case caf::BoxManipulatorPartManager::BCF_X_NEG:
        return -cvf::Vec3d::X_AXIS;
        break;
    case caf::BoxManipulatorPartManager::BCF_Y_POS:
        return cvf::Vec3d::Y_AXIS;
        break;
    case caf::BoxManipulatorPartManager::BCF_Y_NEG:
        return -cvf::Vec3d::Y_AXIS;
        break;
    case caf::BoxManipulatorPartManager::BCF_Z_POS:
        return cvf::Vec3d::Z_AXIS;
        break;
    case caf::BoxManipulatorPartManager::BCF_Z_NEG:
        return -cvf::Vec3d::Z_AXIS;
        break;
    default:
        CVF_ASSERT(false);
        break;
    }

    return cvf::Vec3d::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::createCubeGeos()
{
    Vec3f cp[8];
    navCubeCornerPoints(cp);

    createCubeFaceGeos(BCF_Y_NEG, cp[0], cp[1], cp[5], cp[4]);
    createCubeFaceGeos(BCF_Y_POS, cp[2], cp[3], cp[7], cp[6]);

    createCubeFaceGeos(BCF_Z_POS, cp[4], cp[5], cp[6], cp[7]);
    createCubeFaceGeos(BCF_Z_NEG, cp[3], cp[2], cp[1], cp[0]);

    createCubeFaceGeos(BCF_X_NEG, cp[3], cp[0], cp[4], cp[7]);
    createCubeFaceGeos(BCF_X_POS, cp[1], cp[2], cp[6], cp[5]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoxManipulatorPartManager::createCubeFaceGeos(BoxCubeFace face, cvf::Vec3f p1, cvf::Vec3f p2, cvf::Vec3f p3, cvf::Vec3f p4)
{
    float centerItemHeight = (p1 - p2).length();
    if ((p2 - p3).length() < centerItemHeight)
    {
        centerItemHeight = (p2 - p3).length();
    }
    
    float centerItemFactor = 0.1f;
    centerItemHeight *= centerItemFactor;

    Vec3f center = (p1 + p3) / 2.0f;
    Vec3f u = (p2 - p1).getNormalized() * centerItemHeight;
    Vec3f v = (p4 - p1).getNormalized() * centerItemHeight;

    Vec3f pi1 = center - u / 2.0 - v / 2.0;

    Vec3f pi2 = pi1 + u;
    Vec3f pi3 = pi2 + v;
    Vec3f pi4 = pi1 + v;

    // Inner part
    m_cubeItemType.push_back(navCubeItem(face, BCFI_CENTER));
    m_cubeGeos.push_back(createQuadGeo(pi1, pi2, pi3, pi4).p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> BoxManipulatorPartManager::createQuadGeo(const cvf::Vec3f& v1, const cvf::Vec3f& v2, const cvf::Vec3f& v3, const cvf::Vec3f& v4)
{
    ref<DrawableGeo> geo = new DrawableGeo;

    ref<Vec3fArray> vertexArray = new Vec3fArray(4);
    vertexArray->set(0, v1);
    vertexArray->set(1, v2);
    vertexArray->set(2, v3);
    vertexArray->set(3, v4);

    geo->setVertexArray(vertexArray.p());

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
    Vec3f offset(1.0, 1.0, 1.0);

    Vec3f min(m_origin);
    min -= offset;

    Vec3f max(m_origin + m_size);
    max += offset;

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
std::pair<BoxManipulatorPartManager::BoxCubeFace, BoxManipulatorPartManager::NavCubeFaceItem> BoxManipulatorPartManager::navCubeItem(BoxCubeFace face, NavCubeFaceItem faceItem) const
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

    cvf::ref<caf::BoxManipulatorGeometryGenerator> geometryGenerator = new caf::BoxManipulatorGeometryGenerator;
    geometryGenerator->setOrigin(m_origin);
    geometryGenerator->setSize(m_size);

    cvf::ref<cvf::DrawableGeo> geoMesh = geometryGenerator->createBoundingBoxMeshDrawable();
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool BoxManipulatorPartManager::closestPointOfTwoLines(const cvf::Vec3d& p1, const cvf::Vec3d& q1, const cvf::Vec3d& p2, const cvf::Vec3d& q2, cvf::Vec3d* closestPoint1, cvf::Vec3d* closestPoint2)
{
    //    qDebug() << p1 << " " << q1 << " " << p2 << " " << q2;

        // Taken from Real-Time Collistion Detection, Christer Ericson, 2005, p146-147

        // L1(s) = P1 + sd1
        // L2(t) = P2 + td2

        // d1 = Q1-P1
        // d2 = Q2-P2

        // r = P1-P2

        // a = d1*d1
        // b = d1*d2
        // c = d1*r
        // e = d2*d2;
        // d = ae-b^2
        // f = d2*r

        // s = (bf-ce)/d
        // t = (af-bc)/d


    cvf::Vec3d d1 = q1 - p1;
    cvf::Vec3d d2 = q2 - p2;

    double a = d1.dot(d1);
    double b = d1.dot(d2);
    double e = d2.dot(d2);

    double d = a*e - b*b;

    if (d < std::numeric_limits<double>::epsilon())
    {
        // Parallel lines
        if (closestPoint1) *closestPoint1 = p1;
        if (closestPoint2) *closestPoint2 = p2;
        return false;
    }

    cvf::Vec3d r = p1 - p2;
    double c = d1.dot(r);
    double f = d2.dot(r);

    double s = (b*f - c*e) / d;
    double t = (a*f - b*c) / d;

    if (closestPoint1) *closestPoint1 = p1 + s*d1;
    if (closestPoint2) *closestPoint2 = p2 + t*d2;

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace cvf

