#include "RimWellPathTarget.h"

CAF_PDM_SOURCE_INIT(RimWellPathTarget, "WellPathTarget");

namespace caf
{
template<>
void caf::AppEnum< RimWellPathTarget::TargetTypeEnum >::setUp()
{
    addItem(RimWellPathTarget::POINT_AND_TANGENT, "POINT_AND_TANGENT",   "Point and Tangent");
    addItem(RimWellPathTarget::POINT, "POINT",   "Point");
    setDefault(RimWellPathTarget::POINT_AND_TANGENT);
}
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathTarget::RimWellPathTarget()
    : m_targetType(POINT_AND_TANGENT)
    , m_targetPoint(cvf::Vec3d::ZERO)
    , m_azimuth(0.0)
    , m_inclination(0.0)
{
    
    CAF_PDM_InitFieldNoDefault(&m_targetType, "TargetType", "Type", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_targetPoint, "TargetPoint", "Point", "", "", "");
    CAF_PDM_InitField(&m_azimuth, "Azimuth", 0.0, "Azimuth", "", "", "");
    CAF_PDM_InitField(&m_inclination, "Inclination", 0.0, "Inclination", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathTarget::~RimWellPathTarget()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setAsPointTarget(const cvf::Vec3d& point)
{
    m_targetType =  POINT; 
    m_targetPoint = point; 
    m_azimuth = 0.0; 
    m_inclination = 0.0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setAsPointAndTangentTarget(const cvf::Vec3d& point, 
                                                   double azimuth, 
                                                   double inclination)
{
    m_targetType =  POINT_AND_TANGENT; 
    m_targetPoint = point; 
    m_azimuth = azimuth; 
    m_inclination = inclination;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathTarget::TargetTypeEnum RimWellPathTarget::targetType()
{
    return m_targetType();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathTarget::targetPoint()
{
    return m_targetPoint();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::azimuth()
{
    if ( m_targetType() == POINT_AND_TANGENT )
    {
        return m_azimuth;
    }
    else
    {
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::inclination()
{
    if ( m_targetType() == POINT_AND_TANGENT )
    {
        return m_inclination;
    }
    else
    {
        return std::numeric_limits<double>::infinity();
    }
}
