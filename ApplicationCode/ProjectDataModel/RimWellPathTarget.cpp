#include "RimWellPathTarget.h"
#include "RimModeledWellPath.h"

#include <cmath>
#include "RimWellPathGeometryDef.h"

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
    
    CAF_PDM_InitField(&m_isEnabled, "IsEnabled", true, "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_targetType, "TargetType", "Type", "", "", "");
    //m_targetType.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&m_dogleg1, "Dogleg1", 5.0, "DLS 1", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_targetPoint, "TargetPoint", "Point", "", "", "");
    CAF_PDM_InitField(&m_azimuth, "Azimuth", 0.0, "Azi(deg)", "", "", "");
    CAF_PDM_InitField(&m_inclination, "Inclination", 0.0, "Inc(deg)", "", "", "");
    CAF_PDM_InitField(&m_dogleg2, "Dogleg2", 5.0, "DLS 2", "", "", "");
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
bool RimWellPathTarget::isEnabled() const
{
    return m_isEnabled;    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setAsPointTargetXYD(const cvf::Vec3d& point)
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
    m_azimuth = cvf::Math::toDegrees(azimuth); 
    m_inclination = cvf::Math::toDegrees(inclination);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setDerivedTangent(double azimuth, double inclination)
{
    if (m_targetType == POINT)
    {
        m_azimuth = cvf::Math::toDegrees(azimuth); 
        m_inclination = cvf::Math::toDegrees(inclination);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathTarget::TargetTypeEnum RimWellPathTarget::targetType() const
{
    return m_targetType();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathTarget::targetPointXYZ() const
{
    cvf::Vec3d xyzPoint(m_targetPoint());
    xyzPoint.z() = -xyzPoint.z();
    return xyzPoint;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::azimuth() const
{
    if ( m_targetType() == POINT_AND_TANGENT )
    {
        return cvf::Math::toRadians( m_azimuth);
    }
    else
    {
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::inclination() const
{
    if ( m_targetType() == POINT_AND_TANGENT )
    {
        return cvf::Math::toRadians(m_inclination);
    }
    else
    {
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathTarget::tangent() const
{
    double aziRad = cvf::Math::toRadians(m_azimuth);
    double incRad = cvf::Math::toRadians(m_inclination);
    return cvf::Vec3d (sin(aziRad) * sin(incRad),
                       cos(aziRad) * sin(incRad),
                       -cos(incRad));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::radius1() const
{
    // Needs to be aware of unit to select correct DLS conversion
    // Degrees pr 100 ft
    // Degrees pr 10m
    // Degrees pr 30m

    return 30.0/cvf::Math::toRadians(m_dogleg1);
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::radius2() const
{
    // Needs to be aware of unit to select correct DLS conversion
    // Degrees pr 100 ft
    // Degrees pr 10m
    // Degrees pr 30m

    return 30.0/cvf::Math::toRadians(m_dogleg2);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPathTarget::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    if (fieldNeedingOptions == & m_targetType)
    {
        options.push_back(caf::PdmOptionItemInfo("o->",RimWellPathTarget::POINT_AND_TANGENT, false, QIcon(":/WellTargetPointTangent16x16.png") ));
        options.push_back(caf::PdmOptionItemInfo("o", RimWellPathTarget::POINT, false, QIcon(":/WellTargetPoint16x16.png")));
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimModeledWellPath* wellPath;
    firstAncestorOrThisOfTypeAsserted(wellPath);
    wellPath->updateWellPathVisualization();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    if (m_isEnabled())
    {

        m_targetType.uiCapability()->setUiReadOnly(false);
        m_targetPoint.uiCapability()->setUiReadOnly(false);
        m_dogleg2.uiCapability()->setUiReadOnly(false);

        if ( m_targetType == POINT )
        {
            m_azimuth.uiCapability()->setUiReadOnly(true);
            m_inclination.uiCapability()->setUiReadOnly(true);
            m_dogleg1.uiCapability()->setUiReadOnly(true);
        }
        else
        {
            m_azimuth.uiCapability()->setUiReadOnly(false);
            m_inclination.uiCapability()->setUiReadOnly(false);
            m_dogleg1.uiCapability()->setUiReadOnly(false);
        }

        RimWellPathGeometryDef* geomDef = nullptr;
        firstAncestorOrThisOfTypeAsserted(geomDef);

        if ( this == geomDef->firstActiveTarget() )
        {
            m_dogleg1.uiCapability()->setUiReadOnly(true);
        }

        if ( this == geomDef->lastActiveTarget() )
        {
            m_dogleg2.uiCapability()->setUiReadOnly(true);
        }
    }
    else
    {
        m_dogleg1.uiCapability()->setUiReadOnly(true);
        m_targetType.uiCapability()->setUiReadOnly(true);
        m_targetPoint.uiCapability()->setUiReadOnly(true);
        m_azimuth.uiCapability()->setUiReadOnly(true);
        m_inclination.uiCapability()->setUiReadOnly(true);
        m_dogleg2.uiCapability()->setUiReadOnly(true);
    }
}
