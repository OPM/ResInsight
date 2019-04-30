#include "RimPolylineTarget.h"
#include "RimModeledWellPath.h"

#include <cmath>
#include "RimUserDefinedPolylinesAnnotation.h"
#include "cafPdmUiCheckBoxEditor.h"

CAF_PDM_SOURCE_INIT(RimPolylineTarget, "PolylineTarget");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPolylineTarget::RimPolylineTarget()
    : m_isFullUpdateEnabled(true)
{
    
    CAF_PDM_InitField(&m_isEnabled, "IsEnabled", true, "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_targetPointXyd, "TargetPointXyd", "Point", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPolylineTarget::~RimPolylineTarget()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimPolylineTarget::isEnabled() const
{
    return m_isEnabled;    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPolylineTarget::setAsPointTargetXYD(const cvf::Vec3d& point)
{
    m_targetPointXyd = point; 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPolylineTarget::setAsPointXYZ(const cvf::Vec3d& point)
{
    m_targetPointXyd = cvf::Vec3d(point.x(), point.y(), -point.z());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimPolylineTarget::targetPointXYZ() const
{
    cvf::Vec3d xyzPoint(m_targetPointXyd());
    xyzPoint.z() = -xyzPoint.z();
    return xyzPoint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiFieldHandle* RimPolylineTarget::targetPointUiCapability()
{
    return m_targetPointXyd.uiCapability();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPolylineTarget::enableFullUpdate(bool enable)
{
    m_isFullUpdateEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPolylineTarget::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPolylineTarget::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimUserDefinedPolylinesAnnotation* polyline;
    firstAncestorOrThisOfTypeAsserted(polyline);
    polyline->updateVisualization();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPolylineTarget::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    m_targetPointXyd.uiCapability()->setUiReadOnly(m_isEnabled());
}
