
#pragma once

#include "cvfBase.h"
#include "cvfObject.h"

#include "cvfCollection.h"
#include "cvfMatrix4.h"
#include "cvfVector3.h"

namespace cvf
{
class ModelBasicList;
class Part;
class DrawableGeo;
class Ray;
class HitItem;
} // namespace cvf

namespace caf
{
class BoxManipulatorGeometryGenerator;
};

namespace caf
{
//==================================================================================================
//
//
//==================================================================================================
class BoxManipulatorPartManager : public cvf::Object
{
public:
    enum BoxFace
    {
        BCF_X_POS,
        BCF_X_NEG,
        BCF_Y_POS,
        BCF_Y_NEG,
        BCF_Z_POS,
        BCF_Z_NEG
    };

    enum HandleType
    {
        BCFI_NONE,
        BCFI_CENTER
    };

public:
    BoxManipulatorPartManager();
    ~BoxManipulatorPartManager() override;

    void setOrigin( const cvf::Vec3d& origin );
    void setSize( const cvf::Vec3d& size );
    void originAndSize( cvf::Vec3d* origin, cvf::Vec3d* size );

    bool isManipulatorActive() const;
    void tryToActivateManipulator( const cvf::HitItem* hitItem );
    void updateManipulatorFromRay( const cvf::Ray* ray );
    void endManipulator();

    void appendPartsToModel( cvf::ModelBasicList* model );

private:
    cvf::Vec3d normalFromFace( BoxFace face );
    void       navCubeCornerPoints( cvf::Vec3f points[8] );

    void createBoundingBoxPart();
    void createAllHandleParts();
    void createCubeFaceHandlePart( BoxFace face, cvf::Vec3f p1, cvf::Vec3f p2, cvf::Vec3f p3, cvf::Vec3f p4 );

    void clearAllGeometryAndParts();
    void recreateAllGeometryAndParts();

    static cvf::ref<cvf::DrawableGeo> createHandleGeo( const cvf::Vec3f& v1,
                                                       const cvf::Vec3f& v2,
                                                       const cvf::Vec3f& v3,
                                                       const cvf::Vec3f& v4,
                                                       const cvf::Vec3f& v5 );

private:
    std::vector<std::pair<BoxFace, HandleType>> m_handleIds; // These arrays have the same length
    cvf::Collection<cvf::Part>                  m_handleParts; // These arrays have the same length

    cvf::ref<cvf::Part> m_boundingBoxPart;

    cvf::Vec3d m_origin;
    cvf::Vec3d m_size;

    cvf::Vec3d m_initialPickPoint;
    cvf::Vec3d m_sizeOnStartManipulation;
    cvf::Vec3d m_originOnStartManipulation;

    size_t m_currentHandleIndex;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class BoxManipulatorSourceInfo : public cvf::Object
{
public:
    BoxManipulatorSourceInfo( BoxManipulatorPartManager::BoxFace    cubeFace,
                              BoxManipulatorPartManager::HandleType cubeFaceItem )
        : m_cubeFace( cubeFace )
        , m_cubeHandle( cubeFaceItem )
    {
    }

    BoxManipulatorPartManager::BoxFace    m_cubeFace;
    BoxManipulatorPartManager::HandleType m_cubeHandle;
};

} // namespace caf
