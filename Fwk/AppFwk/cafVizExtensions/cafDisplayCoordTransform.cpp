
#include "cafDisplayCoordTransform.h"
#include "cvfMatrix4.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::DisplayCoordTransform::DisplayCoordTransform()
    : m_scale(1.0, 1.0, 1.0),
    m_translation(cvf::Vec3d::ZERO)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::DisplayCoordTransform::setScale(const cvf::Vec3d& scale)
{
    m_scale = scale;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::DisplayCoordTransform::setTranslation(const cvf::Vec3d& translation)
{
    m_translation = translation;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d caf::DisplayCoordTransform::transformToDisplayCoord(const cvf::Vec3d& domainCoord) const
{
    cvf::Vec3d coord = domainCoord - m_translation;
    coord.x() *= m_scale.x();
    coord.y() *= m_scale.y();
    coord.z() *= m_scale.z();
    
    return coord;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d caf::DisplayCoordTransform::scaleToDisplaySize(const cvf::Vec3d& domainSize) const
{
    cvf::Vec3d coord = domainSize;
    coord.x() *= m_scale.x();
    coord.y() *= m_scale.y();
    coord.z() *= m_scale.z();

    return coord;
}
