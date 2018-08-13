#include "RicMultiSegmentWellExportInfo.h"

#include "RigWellPath.h"
#include "RimWellPath.h"

#include <algorithm>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellSegmentLateralIntersection::RicWellSegmentLateralIntersection(const QString& gridName, 
                                                                     size_t gridLocalCellIndex,
                                                                     const cvf::Vec3st& gridLocalCellIJK,
                                                                     double startMD,
                                                                     double deltaMD,
                                                                     double startTVD,
                                                                     double deltaTVD,
                                                                     const cvf::Vec3d& lengthsInCell)
    : m_gridName(gridName)
    , m_gridLocalCellIndex(gridLocalCellIndex)
    , m_gridLocalCellIJK(gridLocalCellIJK)
    , m_startMD(startMD)
    , m_deltaMD(deltaMD)
    , m_startTVD(startTVD)
    , m_deltaTVD(deltaTVD)
    , m_lengthsInCell(lengthsInCell)
    , m_segmentNumber(std::numeric_limits<int>::infinity())
    , m_attachedSegmentNumber(std::numeric_limits<int>::infinity())
    , m_mainBoreCell(false)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RicWellSegmentLateralIntersection::gridName() const
{
    return m_gridName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicWellSegmentLateralIntersection::gridLocalCellIndex() const
{
    return m_gridLocalCellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RicWellSegmentLateralIntersection::gridLocalCellIJK() const
{
    return m_gridLocalCellIJK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLateralIntersection::startMD() const
{
    return m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLateralIntersection::deltaMD() const
{
    return m_deltaMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLateralIntersection::startTVD() const
{
    return m_startTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLateralIntersection::deltaTVD() const
{
    return m_deltaTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d& RicWellSegmentLateralIntersection::lengthsInCell() const
{
    return m_lengthsInCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicWellSegmentLateralIntersection::segmentNumber() const
{
    return m_segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicWellSegmentLateralIntersection::attachedSegmentNumber() const
{
    return m_attachedSegmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellSegmentLateralIntersection::isMainBoreCell() const
{
    return m_mainBoreCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLateralIntersection::setSegmentNumber(int segmentNumber)
{
    m_segmentNumber = segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLateralIntersection::setAttachedSegmentNumber(int attachedSegmentNumber)
{
    m_attachedSegmentNumber = attachedSegmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLateralIntersection::setIsMainBoreCell(bool isMainBoreCell)
{
    m_mainBoreCell = isMainBoreCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellSegmentLateral::RicWellSegmentLateral(size_t lateralIndex, int branchNumber /*= 0*/)
    : m_lateralIndex(lateralIndex)
    , m_branchNumber(branchNumber)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicWellSegmentLateral::lateralIndex() const
{
    return m_lateralIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicWellSegmentLateral::branchNumber() const
{
    return m_branchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLateral::setBranchNumber(int branchNumber)
{
    m_branchNumber = branchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLateral::addIntersection(const RicWellSegmentLateralIntersection& intersection)
{
    m_intersections.push_back(intersection);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicWellSegmentLateralIntersection>& RicWellSegmentLateral::intersections()
{
    return m_intersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RicWellSegmentLateralIntersection>& RicWellSegmentLateral::intersections() const
{
    return m_intersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellSegmentLocation::RicWellSegmentLocation(const QString& label,
                                               double         measuredDepth,
                                               double         trueVerticalDepth,
                                               size_t         subIndex,
                                               int            segmentNumber /*= -1*/)
    : m_label(label)
    , m_measuredDepth(measuredDepth)
    , m_trueVerticalDepth(trueVerticalDepth)
    , m_effectiveDiameter(0.15)
    , m_holeDiameter(RicMultiSegmentWellExportInfo::defaultDoubleValue())
    , m_openHoleRoughnessFactor(5.0e-5)
    , m_skinFactor(RicMultiSegmentWellExportInfo::defaultDoubleValue())
    , m_icdFlowCoefficient(RicMultiSegmentWellExportInfo::defaultDoubleValue())
    , m_icdArea(RicMultiSegmentWellExportInfo::defaultDoubleValue())
    , m_subIndex(subIndex)
    , m_segmentNumber(segmentNumber)
    , m_icdBranchNumber(-1)
    , m_icdSegmentNumber(-1)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicWellSegmentLocation::label() const
{
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLocation::measuredDepth() const
{
    return m_measuredDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLocation::trueVerticalDepth() const
{
    return m_trueVerticalDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLocation::effectiveDiameter() const
{
    return m_effectiveDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLocation::holeDiameter() const
{
    return m_holeDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLocation::openHoleRoughnessFactor() const
{
    return m_openHoleRoughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLocation::skinFactor() const
{
    return m_skinFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLocation::icdFlowCoefficient() const
{
    return m_icdFlowCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSegmentLocation::icdArea() const
{
    return m_icdArea;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicWellSegmentLocation::subIndex() const
{
    return m_subIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicWellSegmentLocation::segmentNumber() const
{
    return m_segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicWellSegmentLocation::icdBranchNumber() const
{
    return m_icdBranchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicWellSegmentLocation::icdSegmentNumber() const
{
    return m_icdSegmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RicWellSegmentLateral>& RicWellSegmentLocation::laterals() const
{
    return m_laterals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicWellSegmentLateral>& RicWellSegmentLocation::laterals()
{
    return m_laterals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLocation::setEffectiveDiameter(double effectiveDiameter)
{
    m_effectiveDiameter = effectiveDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLocation::setHoleDiameter(double holeDiameter)
{
    m_holeDiameter = holeDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLocation::setOpenHoleRoughnessFactor(double roughnessFactor)
{
    m_openHoleRoughnessFactor = roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLocation::setSkinFactor(double skinFactor)
{
    m_skinFactor = skinFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLocation::setIcdFlowCoefficient(double icdFlowCoefficient)
{
    m_icdFlowCoefficient = icdFlowCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLocation::setIcdArea(double icdArea)
{
    m_icdArea = icdArea;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLocation::setSegmentNumber(int segmentNumber)
{
    m_segmentNumber = segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLocation::setIcdBranchNumber(int icdBranchNumber)
{
    m_icdBranchNumber = icdBranchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLocation::setIcdSegmentNumber(int icdSegmentNumber)
{
    m_icdSegmentNumber = icdSegmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentLocation::addLateral(const RicWellSegmentLateral& lateral)
{
    m_laterals.push_back(lateral);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellSegmentLocation::operator<(const RicWellSegmentLocation& rhs) const
{
    return measuredDepth() < rhs.measuredDepth();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMultiSegmentWellExportInfo::RicMultiSegmentWellExportInfo(const RimWellPath*              wellPath,
                                                             RiaEclipseUnitTools::UnitSystem unitSystem,
                                                             double                          initialMD,
                                                             const QString&                  lengthAndDepthText,
                                                             const QString&                  pressureDropText)
    : m_wellPath(wellPath)
    , m_initialMD(initialMD)
    , m_unitSystem(unitSystem)
    , m_topWellBoreVolume(RicMultiSegmentWellExportInfo::defaultDoubleValue())
    , m_linerDiameter(0.15)
    , m_roughnessFactor(5.0e-5)
    , m_lengthAndDepthText(lengthAndDepthText)
    , m_pressureDropText(pressureDropText)
    , m_hasSubGridIntersections(false)
{
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
void RicMultiSegmentWellExportInfo::setHasSubGridIntersections(bool subGridIntersections)
{
    m_hasSubGridIntersections = subGridIntersections;
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
bool RicMultiSegmentWellExportInfo::hasSubGridIntersections() const
{
    return m_hasSubGridIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMultiSegmentWellExportInfo::defaultDoubleValue()
{
    return std::numeric_limits<double>::infinity();
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
