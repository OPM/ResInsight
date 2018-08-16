#include "RicMultiSegmentWellExportInfo.h"

#include "RigWellPath.h"
#include "RimWellPath.h"

#include <algorithm>
#include <limits>


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellSubSegmentCellIntersection::RicWellSubSegmentCellIntersection(const QString& gridName, 
                                                                                         size_t globalCellIndex,
                                                                                         const cvf::Vec3st& gridLocalCellIJK,
                                                                                         const cvf::Vec3d& lengthsInCell)
    : m_gridName(gridName)
    , m_globalCellIndex(globalCellIndex)
    , m_gridLocalCellIJK(gridLocalCellIJK)
    , m_lengthsInCell(lengthsInCell)
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RicWellSubSegmentCellIntersection::gridName() const
{
    return m_gridName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicWellSubSegmentCellIntersection::globalCellIndex() const
{
    return m_globalCellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RicWellSubSegmentCellIntersection::gridLocalCellIJK() const
{
    return m_gridLocalCellIJK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d& RicWellSubSegmentCellIntersection::lengthsInCell() const
{
    return m_lengthsInCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellSubSegment::RicWellSubSegment(double startMD,
                                                                 double deltaMD,
                                                                 double startTVD,
                                                                 double deltaTVD)
    : m_startMD(startMD)
    , m_deltaMD(deltaMD)
    , m_startTVD(startTVD)
    , m_deltaTVD(deltaTVD)
    , m_segmentNumber(-1)
    , m_attachedSegmentNumber(-1)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSubSegment::startMD() const
{
    return m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSubSegment::deltaMD() const
{
    return m_deltaMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSubSegment::startTVD() const
{
    return m_startTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellSubSegment::deltaTVD() const
{
    return m_deltaTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicWellSubSegment::segmentNumber() const
{
    return m_segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicWellSubSegment::attachedSegmentNumber() const
{
    return m_attachedSegmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSubSegment::setSegmentNumber(int segmentNumber)
{
    m_segmentNumber = segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSubSegment::setAttachedSegmentNumber(int attachedSegmentNumber)
{
    m_attachedSegmentNumber = attachedSegmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSubSegment::addIntersection(const RicWellSubSegmentCellIntersection& intersection)
{
    m_intersections.push_back(intersection);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RicWellSubSegmentCellIntersection>& RicWellSubSegment::intersections() const
{
    return m_intersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicWellSubSegmentCellIntersection>& RicWellSubSegment::intersections()
{
    return m_intersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellSegmentCompletion::RicWellSegmentCompletion(RigCompletionData::CompletionType completionType,
                                                   size_t                            index /* = cvf::UNDEFINED_SIZE_T */,
                                                   int                               branchNumber /*= 0*/)
    : m_completionType(completionType)
    , m_index(index)
    , m_branchNumber(branchNumber)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicWellSegmentCompletion::completionType() const
{
    return m_completionType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicWellSegmentCompletion::index() const
{
    return m_index;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicWellSegmentCompletion::branchNumber() const
{
    return m_branchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentCompletion::setBranchNumber(int branchNumber)
{
    m_branchNumber = branchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellSegmentCompletion::addSubSegment(const RicWellSubSegment& subSegment)
{
    m_subSegments.push_back(subSegment);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicWellSubSegment>& RicWellSegmentCompletion::subSegments()
{
    return m_subSegments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RicWellSubSegment>& RicWellSegmentCompletion::subSegments() const
{
    return m_subSegments;
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
const std::vector<RicWellSegmentCompletion>& RicWellSegmentLocation::completions() const
{
    return m_completions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicWellSegmentCompletion>& RicWellSegmentLocation::completions()
{
    return m_completions;
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
void RicWellSegmentLocation::addCompletion(const RicWellSegmentCompletion& completion)
{
    m_completions.push_back(completion);
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
