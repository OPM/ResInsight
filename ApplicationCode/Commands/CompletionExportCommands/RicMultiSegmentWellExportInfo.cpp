#include "RicMultiSegmentWellExportInfo.h"

#include "RigWellPath.h"
#include "RimWellPath.h"


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellSegmentLocation::operator<(const RicWellSegmentLocation& rhs) const
{
    return measuredDepth < rhs.measuredDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMultiSegmentWellExportInfo::setTopWellBoreVolume(double topWellBoreVolume)
{
    m_topWellBoreVolume = topWellBoreVolume;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMultiSegmentWellExportInfo::setLinerDiameter(double linerDiameter)
{
    m_linerDiameter = linerDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMultiSegmentWellExportInfo::setRoughnessFactor(double roughnessFactor)
{
    m_roughnessFactor = roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMultiSegmentWellExportInfo::addWellSegmentLocation(const RicWellSegmentLocation& location)
{
    m_wellSegmentLocations.push_back(location);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMultiSegmentWellExportInfo::sortLocations()
{
    std::sort(m_wellSegmentLocations.begin(), m_wellSegmentLocations.end());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPath* RicMultiSegmentWellExportInfo::wellPath() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMultiSegmentWellExportInfo::initialMD() const
{
    return m_initialMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMultiSegmentWellExportInfo::initialTVD() const
{
    return -m_wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(m_initialMD).z();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystem RicMultiSegmentWellExportInfo::unitSystem() const
{
    return m_unitSystem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMultiSegmentWellExportInfo::topWellBoreVolume() const
{
    return m_topWellBoreVolume;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMultiSegmentWellExportInfo::linerDiameter() const
{
    return m_linerDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMultiSegmentWellExportInfo::roughnessFactor() const
{
    return m_roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicMultiSegmentWellExportInfo::lengthAndDepthText() const
{
    return m_lengthAndDepthText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicMultiSegmentWellExportInfo::pressureDropText() const
{
    return m_pressureDropText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RicWellSegmentLocation>& RicMultiSegmentWellExportInfo::wellSegmentLocations() const
{
    return m_wellSegmentLocations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicWellSegmentLocation>& RicMultiSegmentWellExportInfo::wellSegmentLocations()
{
    return m_wellSegmentLocations;
}
