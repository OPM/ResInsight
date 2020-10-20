
#include "cafDisplayCoordTransform.h"
#include "cvfMatrix4.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::DisplayCoordTransform::DisplayCoordTransform()
    : m_scale( 1.0, 1.0, 1.0 )
    , m_translation( cvf::Vec3d::ZERO )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::DisplayCoordTransform::setScale( const cvf::Vec3d& scale )
{
    m_scale = scale;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::DisplayCoordTransform::setTranslation( const cvf::Vec3d& translation )
{
    m_translation = translation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d caf::DisplayCoordTransform::translateToDisplayCoord( const cvf::Vec3d& domainCoord ) const
{
    return domainCoord - m_translation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d caf::DisplayCoordTransform::transformToDisplayCoord( const cvf::Vec3d& domainCoord ) const
{
    cvf::Vec3d coord = translateToDisplayCoord( domainCoord );

    coord.x() *= m_scale.x();
    coord.y() *= m_scale.y();
    coord.z() *= m_scale.z();

    return coord;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> caf::DisplayCoordTransform::transformToDisplayCoords( const std::vector<cvf::Vec3d>& domainCoords ) const
{
    std::vector<cvf::Vec3d> displayCoords;

    for ( const auto& coord : domainCoords )
    {
        displayCoords.emplace_back( transformToDisplayCoord( coord ) );
    }

    return displayCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d caf::DisplayCoordTransform::scaleToDisplaySize( const cvf::Vec3d& domainSize ) const
{
    cvf::Vec3d coord = domainSize;
    coord.x() *= m_scale.x();
    coord.y() *= m_scale.y();
    coord.z() *= m_scale.z();

    return coord;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d caf::DisplayCoordTransform::translateToDomainCoord( const cvf::Vec3d& displayCoord ) const
{
    return displayCoord + m_translation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d caf::DisplayCoordTransform::transformToDomainCoord( const cvf::Vec3d& displayCoord ) const
{
    cvf::Vec3d unScaledDisplayCoord = displayCoord;
    unScaledDisplayCoord.x() /= m_scale.x();
    unScaledDisplayCoord.y() /= m_scale.y();
    unScaledDisplayCoord.z() /= m_scale.z();

    return translateToDomainCoord( unScaledDisplayCoord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d caf::DisplayCoordTransform::scaleToDomainSize( const cvf::Vec3d& displaySize ) const
{
    cvf::Vec3d coord = displaySize;
    coord.x() /= m_scale.x();
    coord.y() /= m_scale.y();
    coord.z() /= m_scale.z();

    return coord;
}
