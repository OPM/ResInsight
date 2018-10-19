/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimFishbonesMultipleSubs.h"

#include "RiaColorTables.h"
#include "RigFishbonesGeometry.h"
#include "RigWellPath.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimFishbonesCollection.h"
#include "RimMultipleValveLocations.h"

#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiListEditor.h"

#include "cvfAssert.h"
#include "cvfBoundingBox.h"

#include <cstdlib>
#include <cmath>
#include "cvfMath.h"



CAF_PDM_SOURCE_INIT(RimFishbonesMultipleSubs, "FishbonesMultipleSubs");

namespace caf {
    template<>
    void AppEnum<RimFishbonesMultipleSubs::LocationType>::setUp()
    {
        addItem(RimFishbonesMultipleSubs::FB_SUB_COUNT_END,     "FB_SUB_COUNT",     "Start/End/Number of Subs");
        addItem(RimFishbonesMultipleSubs::FB_SUB_SPACING_END,   "FB_SUB_SPACING",   "Start/End/Spacing");
        addItem(RimFishbonesMultipleSubs::FB_SUB_USER_DEFINED,  "FB_SUB_CUSTOM",    "User Specification");
        setDefault(RimFishbonesMultipleSubs::FB_SUB_COUNT_END);
    }

    template<>
    void AppEnum<RimFishbonesMultipleSubs::LateralsOrientationType>::setUp()
    {
        addItem(RimFishbonesMultipleSubs::FB_LATERAL_ORIENTATION_FIXED, "FB_LATERAL_ORIENTATION_FIXED",     "Fixed Angle");
        addItem(RimFishbonesMultipleSubs::FB_LATERAL_ORIENTATION_RANDOM, "FB_LATERAL_ORIENTATION_RANDOM",   "Random Angle");
        setDefault(RimFishbonesMultipleSubs::FB_LATERAL_ORIENTATION_RANDOM);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesMultipleSubs::RimFishbonesMultipleSubs()
{
    CAF_PDM_InitObject("FishbonesMultipleSubs", ":/FishBoneGroup16x16.png", "", "");

    CAF_PDM_InitField(&m_isActive,                      "Active", true, "Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_name,                 "Name", "Name", "", "", "");
    m_name.registerGetMethod(this, &RimFishbonesMultipleSubs::generatedName);
    m_name.uiCapability()->setUiReadOnly(true);
    m_name.xmlCapability()->setIOWritable(false);

    cvf::Color3f defaultColor = RiaColorTables::wellPathComponentColors()[RiaDefines::FISHBONES];
    CAF_PDM_InitField(&fishbonesColor,                  "Color", defaultColor, "Fishbones Color", "", "", "");

    CAF_PDM_InitField(&m_lateralCountPerSub,            "LateralCountPerSub", 3,            "Laterals Per Sub", "", "", "");
    CAF_PDM_InitField(&m_lateralLength,                 "LateralLength",  QString("11.0"),  "Length(s) [m]", "", "Specify multiple length values if the sub lengths differ", "");

    CAF_PDM_InitField(&m_lateralExitAngle,              "LateralExitAngle", 35.0,           "Exit Angle [deg]", "", "", "");
    CAF_PDM_InitField(&m_lateralBuildAngle,             "LateralBuildAngle", 6.0,           "Build Angle [deg/m]", "", "", "");

    CAF_PDM_InitField(&m_lateralTubingDiameter,         "LateralTubingDiameter", 8.0,       "Tubing Diameter [mm]", "", "", "");

    CAF_PDM_InitField(&m_lateralOpenHoleRoghnessFactor, "LateralOpenHoleRoghnessFactor", 0.001,   "Open Hole Roghness Factor [m]", "", "", "");
    CAF_PDM_InitField(&m_lateralTubingRoghnessFactor,   "LateralTubingRoghnessFactor", 1e-5,      "Tubing Roghness Factor [m]", "", "", "");

    CAF_PDM_InitField(&m_lateralInstallSuccessFraction, "LateralInstallSuccessFraction", 1.0,     "Install Success Rate [0..1]", "", "", "");

    CAF_PDM_InitField(&m_icdCount,                      "IcdCount", 2,                      "ICDs per Sub", "", "", "");
    CAF_PDM_InitField(&m_icdOrificeDiameter,            "IcdOrificeDiameter", 7.0,          "ICD Orifice Diameter [mm]", "", "", "");
    CAF_PDM_InitField(&m_icdFlowCoefficient,            "IcdFlowCoefficient", 1.5,          "ICD Flow Coefficient", "", "", "");

    initialiseObsoleteFields();
    CAF_PDM_InitFieldNoDefault(&m_valveLocations, "ValveLocations", "Valve Locations", "", "", "");
    m_valveLocations = new RimMultipleValveLocations();
    m_valveLocations.uiCapability()->setUiHidden(true);
    m_valveLocations.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitField(&m_subsOrientationMode,           "SubsOrientationMode", caf::AppEnum<LateralsOrientationType>(FB_LATERAL_ORIENTATION_RANDOM), "Orientation", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&m_installationRotationAngles, "InstallationRotationAngles", "Installation Rotation Angles [deg]", "", "", "");
    m_installationRotationAngles.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&m_fixedInstallationRotationAngle, "FixedInstallationRotationAngle", 0.0, "  Fixed Angle [deg]", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_pipeProperties, "PipeProperties", "Pipe Properties", "", "", "");
    m_pipeProperties.uiCapability()->setUiHidden(true);
    m_pipeProperties.uiCapability()->setUiTreeChildrenHidden(true);

    m_pipeProperties = new RimFishbonesPipeProperties;

    m_rigFishbonesGeometry = std::unique_ptr<RigFisbonesGeometry>(new RigFisbonesGeometry(this));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesMultipleSubs::~RimFishbonesMultipleSubs()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFishbonesMultipleSubs::isActive() const
{
    return m_isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimFishbonesMultipleSubs::generatedName() const
{
    caf::PdmChildArrayField<RimFishbonesMultipleSubs*>* container = dynamic_cast<caf::PdmChildArrayField<RimFishbonesMultipleSubs*>*>(this->parentField());
    CVF_ASSERT(container);

    size_t index = container->index(this);
    return QString("Fishbone %1").arg(index);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::setMeasuredDepthAndCount(double measuredDepth, double spacing, int subCount)
{
    m_valveLocations->setMeasuredDepthAndCount(measuredDepth, spacing, subCount);

    computeRangesAndLocations();
    computeRotationAngles();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::measuredDepth(size_t subIndex) const
{
    return m_valveLocations->measuredDepth(subIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::rotationAngle(size_t index) const
{
    if (m_subsOrientationMode == FB_LATERAL_ORIENTATION_FIXED)
    {
        return m_fixedInstallationRotationAngle;
    }
    else
    {
        CVF_ASSERT(index < m_installationRotationAngles().size());

        return m_installationRotationAngles()[index];
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::exitAngle() const
{
    return m_lateralExitAngle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::buildAngle() const
{
    return m_lateralBuildAngle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::tubingDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfTypeAsserted(wellPath);
    if (unitSystem == RiaEclipseUnitTools::UNITS_METRIC)
    {
        if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
        {
            return RiaEclipseUnitTools::inchToMeter(m_lateralTubingDiameter());
        }
        else
        {
            return m_lateralTubingDiameter() / 1000;
        }
    }
    else if (unitSystem == RiaEclipseUnitTools::UNITS_FIELD)
    {
        if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
        {
            return RiaEclipseUnitTools::meterToFeet(m_lateralTubingDiameter() / 1000);
        }
        else
        {
            return RiaEclipseUnitTools::inchToFeet(m_lateralTubingDiameter());
        }
    }
    CVF_ASSERT(false);
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::effectiveDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const
{
    double innerRadius = tubingDiameter(unitSystem) / 2;
    double outerRadius = holeDiameter(unitSystem) / 2;

    double innerArea = cvf::PI_D * innerRadius * innerRadius;
    double outerArea = cvf::PI_D * outerRadius * outerRadius;

    double effectiveArea = outerArea - innerArea;

    double effectiveRadius = cvf::Math::sqrt(effectiveArea / cvf::PI_D);
    return effectiveRadius * 2;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::openHoleRoughnessFactor(RiaEclipseUnitTools::UnitSystem unitSystem) const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfTypeAsserted(wellPath);
    if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD && unitSystem == RiaEclipseUnitTools::UNITS_METRIC)
    {
        return RiaEclipseUnitTools::feetToMeter(m_lateralOpenHoleRoghnessFactor());
    }
    else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC && unitSystem == RiaEclipseUnitTools::UNITS_FIELD)
    {
        return RiaEclipseUnitTools::meterToFeet(m_lateralOpenHoleRoghnessFactor());
    }
    return m_lateralOpenHoleRoghnessFactor();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::icdOrificeDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfTypeAsserted(wellPath);
    if (unitSystem == RiaEclipseUnitTools::UNITS_METRIC)
    {
        if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
        {
            return RiaEclipseUnitTools::inchToMeter(m_icdOrificeDiameter());
        }
        else
        {
            return m_icdOrificeDiameter() / 1000;
        }
    }
    else if (unitSystem == RiaEclipseUnitTools::UNITS_FIELD)
    {
        if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
        {
            return RiaEclipseUnitTools::meterToFeet(m_icdOrificeDiameter() / 1000);
        }
        else
        {
            return RiaEclipseUnitTools::inchToFeet(m_icdOrificeDiameter());
        }
    }
    CVF_ASSERT(false);
    return 0.0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFishbonesMultipleSubs::lateralLengths() const
{
    QStringList items = m_lateralLength().split(' ');
    double currentLength = 0.0;

    std::vector<double> lengths;
    for (int i = 0; i < static_cast<int>(m_lateralCountPerSub); i++)
    {
        if (i < items.size())
        {
            bool conversionOk = false;
            double candidateValue = items[i].toDouble(&conversionOk);
            if (conversionOk)
            {
                currentLength = candidateValue;
            }
        }

        lengths.push_back(currentLength);
    }

    return lengths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::valveLocationsUpdated()
{
    RimFishbonesCollection* collection;
    this->firstAncestorOrThisOfTypeAsserted(collection);
    computeSubLateralIndices();
    collection->recalculateStartMD();

    computeRotationAngles();
    computeSubLateralIndices();

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimFishbonesMultipleSubs::coordsForLateral(size_t subIndex, size_t lateralIndex) const
{
    std::vector<std::pair<cvf::Vec3d, double>> coordsAndMD = m_rigFishbonesGeometry->coordsForLateral(subIndex, lateralIndex);

    std::vector<cvf::Vec3d> domainCoords;
    for (const auto& coordMD : coordsAndMD)
    {
        domainCoords.push_back(coordMD.first);
    }

    return domainCoords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<cvf::Vec3d, double>> RimFishbonesMultipleSubs::coordsAndMDForLateral(size_t subIndex, size_t lateralIndex) const
{
    return m_rigFishbonesGeometry->coordsForLateral(subIndex, lateralIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::recomputeLateralLocations()
{
    computeRangesAndLocations();
    computeRotationAngles();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::setUnitSystemSpecificDefaults()
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfType(wellPath);
    if (wellPath)
    {
        if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
        {
            m_lateralLength = "11";
            m_lateralBuildAngle = 6.0;
            m_lateralTubingDiameter = 8;
            m_lateralOpenHoleRoghnessFactor = 0.001;
            m_lateralTubingRoghnessFactor = 1e-05;
            m_icdOrificeDiameter = 7;
        }
        else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
        {
            m_lateralLength = "36";
            m_lateralBuildAngle = 1.83;
            m_lateralTubingDiameter = 0.31;
            m_lateralOpenHoleRoghnessFactor = 0.0032;
            m_lateralTubingRoghnessFactor = 3.28e-05;
            m_icdOrificeDiameter = 0.28;
        }
        m_pipeProperties->setUnitSystemSpecificDefaults();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimFishbonesMultipleSubs::componentType() const
{
    return RiaDefines::FISHBONES;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFishbonesMultipleSubs::componentLabel() const
{
    return generatedName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFishbonesMultipleSubs::componentTypeLabel() const
{
    return "Fishbones";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimFishbonesMultipleSubs::defaultComponentColor() const
{
    return fishbonesColor();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::startMD() const
{
    double measuredDepth = 0.0;
    if (!m_valveLocations->locationOfValves().empty())
    {
        measuredDepth = m_valveLocations->locationOfValves().front();
    }

    return measuredDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::endMD() const
{
    double measuredDepth = 0.0;
    if (!m_valveLocations->locationOfValves().empty())
    {
        measuredDepth = m_valveLocations->locationOfValves().back();
    }

    return measuredDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_subsOrientationMode)
    {
        computeRotationAngles();
    }

    if (changedField == &m_lateralInstallSuccessFraction ||
        changedField == &m_lateralCountPerSub)
    {
        computeSubLateralIndices();
    }

    valveLocationsUpdated();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFishbonesMultipleSubs::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFishbonesMultipleSubs::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::computeRangesAndLocations()
{
    m_valveLocations->computeRangesAndLocations();
    valveLocationsUpdated();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        RimWellPath* wellPath;
        firstAncestorOrThisOfType(wellPath);
        if (wellPath)
        {
            if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
            {
                m_lateralLength.uiCapability()->setUiName("Length(s) [m]");
                m_lateralBuildAngle.uiCapability()->setUiName("Build Angle [deg/m]");
                m_lateralTubingDiameter.uiCapability()->setUiName("Tubing Diameter [mm]");
                m_lateralOpenHoleRoghnessFactor.uiCapability()->setUiName("Open Hole Roughness Factor [m]");
                m_lateralTubingRoghnessFactor.uiCapability()->setUiName("Tubing Roughness Factor [m]");

                m_icdOrificeDiameter.uiCapability()->setUiName("ICD Orifice Diameter [mm]");
            }
            else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
            {
                m_lateralLength.uiCapability()->setUiName("Length(s) [ft]");
                m_lateralBuildAngle.uiCapability()->setUiName("Build Angle [deg/ft]");
                m_lateralTubingDiameter.uiCapability()->setUiName("Tubing Diameter [in]");
                m_lateralOpenHoleRoghnessFactor.uiCapability()->setUiName("Open Hole Roughness Factor [ft]");
                m_lateralTubingRoghnessFactor.uiCapability()->setUiName("Tubing Roughness Factor [ft]");

                m_icdOrificeDiameter.uiCapability()->setUiName("ICD Orifice Diameter [in]");
            }
        }
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Appearance");

        group->add(&fishbonesColor);
    }

    {        
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Location");
        m_valveLocations->uiOrdering(uiConfigName, *group);
    }
    
    {
        caf::PdmUiGroup* lateralConfigGroup = uiOrdering.addNewGroup("Lateral Configuration");

        lateralConfigGroup->add(&m_lateralCountPerSub);
        lateralConfigGroup->add(&m_lateralLength);

        lateralConfigGroup->add(&m_lateralExitAngle);
        lateralConfigGroup->add(&m_lateralBuildAngle);
        
        lateralConfigGroup->add(&m_subsOrientationMode);
        if (m_subsOrientationMode == FB_LATERAL_ORIENTATION_FIXED)
        {
            lateralConfigGroup->add(&m_fixedInstallationRotationAngle);
        }

        lateralConfigGroup->add(&m_lateralInstallSuccessFraction);

    }

    {
        caf::PdmUiGroup* wellGroup = uiOrdering.addNewGroup("Well Properties");

        m_pipeProperties->uiOrdering(uiConfigName, *wellGroup);
    }

    {
        caf::PdmUiGroup* mswGroup = uiOrdering.addNewGroup("Multi Segment Wells");
        mswGroup->setCollapsedByDefault(true);
        mswGroup->add(&m_lateralTubingDiameter);
        mswGroup->add(&m_lateralOpenHoleRoghnessFactor);
        mswGroup->add(&m_lateralTubingRoghnessFactor);
        mswGroup->add(&m_icdCount);
        mswGroup->add(&m_icdOrificeDiameter);
        mswGroup->add(&m_icdFlowCoefficient);
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::initAfterRead()
{
    initValveLocationFromLegacyData();

    if (m_valveLocations->locationOfValves().size() != m_installationRotationAngles().size())
    {
        computeRotationAngles();
    }
    computeSubLateralIndices();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimFishbonesMultipleSubs::boundingBoxInDomainCoords() const
{
    cvf::BoundingBox bb;

    for (auto& sub : installedLateralIndices())
    {
        for (size_t lateralIndex : sub.lateralIndices)
        {
            std::vector<cvf::Vec3d> coords = coordsForLateral(sub.subIndex, lateralIndex);

            for (auto c : coords)
            {
                bb.add(c);
            }
        }
    }

    return bb;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::computeRotationAngles()
{
    std::vector<double> vals;

    for (size_t i = 0; i < m_valveLocations->locationOfValves().size(); i++)
    {
        vals.push_back(RimFishbonesMultipleSubs::randomValueFromRange(0, 360));
    }

    m_installationRotationAngles = vals;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::computeSubLateralIndices()
{
    m_subLateralIndices.clear();
    for (size_t subIndex = 0; subIndex < m_valveLocations->locationOfValves().size(); ++subIndex)
    {
        SubLateralIndex subLateralIndex;
        subLateralIndex.subIndex = subIndex;

        for (int lateralIndex = 0; lateralIndex < m_lateralCountPerSub(); ++lateralIndex)
        {
            subLateralIndex.lateralIndices.push_back(lateralIndex);
        }
        m_subLateralIndices.push_back(subLateralIndex);
    }
    double numLaterals = static_cast<double>(m_valveLocations->locationOfValves().size() * m_lateralCountPerSub);
    int numToRemove = static_cast<int>(std::round((1 - m_lateralInstallSuccessFraction) * numLaterals));
    srand(m_randomSeed());
    while (numToRemove > 0)
    {
        int subIndexToRemove;
        do {
            subIndexToRemove = rand() % m_subLateralIndices.size();
        } while (m_subLateralIndices[subIndexToRemove].lateralIndices.empty());
        int lateralIndexToRemove = rand() % m_subLateralIndices[subIndexToRemove].lateralIndices.size();
        m_subLateralIndices[subIndexToRemove].lateralIndices.erase(m_subLateralIndices[subIndexToRemove].lateralIndices.begin() + lateralIndexToRemove);
        --numToRemove;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimFishbonesMultipleSubs::randomValueFromRange(int min, int max)
{
    // See http://www.cplusplus.com/reference/cstdlib/rand/ 

    int range = abs(max - min);
    int randomNumberInRange = rand() % range;

    int randomValue = min + randomNumberInRange;

    return randomValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::initialiseObsoleteFields()
{
    CAF_PDM_InitField(&m_subsLocationMode_OBSOLETE, "SubsLocationMode", caf::AppEnum<LocationType>(FB_SUB_UNDEFINED), "Location Defined By", "", "", "");
    m_subsLocationMode_OBSOLETE.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&m_rangeStart_OBSOLETE, "RangeStart", std::numeric_limits<double>::infinity(), "Start MD [m]", "", "", "");
    m_rangeStart_OBSOLETE.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&m_rangeEnd_OBSOLETE, "RangeEnd", std::numeric_limits<double>::infinity(), "End MD [m]", "", "", "");
    m_rangeEnd_OBSOLETE.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&m_rangeSubSpacing_OBSOLETE, "RangeSubSpacing", std::numeric_limits<double>::infinity(), "Spacing [m]", "", "", "");
    m_rangeSubSpacing_OBSOLETE.xmlCapability()->setIOWritable(false);
    
    CAF_PDM_InitField(&m_rangeSubCount_OBSOLETE, "RangeSubCount", -1, "Number of Subs", "", "", "");
    m_rangeSubCount_OBSOLETE.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_locationOfSubs_OBSOLETE, "LocationOfSubs", "Measured Depths [m]", "", "", "");
    m_locationOfSubs_OBSOLETE.xmlCapability()->setIOWritable(false);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::initValveLocationFromLegacyData()
{
    RimMultipleValveLocations::LocationType locationType = RimMultipleValveLocations::VALVE_UNDEFINED;
    if (m_subsLocationMode_OBSOLETE() == FB_SUB_COUNT_END)
    {
        locationType = RimMultipleValveLocations::VALVE_COUNT;
    }
    else if (m_subsLocationMode_OBSOLETE() == FB_SUB_SPACING_END)
    {
        locationType = RimMultipleValveLocations::VALVE_SPACING;
    }
    else if (m_subsLocationMode_OBSOLETE() == FB_SUB_USER_DEFINED)
    {
        locationType = RimMultipleValveLocations::VALVE_CUSTOM;
    }
        
    m_valveLocations->initFieldsFromFishbones(locationType,
                                              m_rangeStart_OBSOLETE(),
                                              m_rangeEnd_OBSOLETE(),
                                              m_rangeSubSpacing_OBSOLETE(),
                                              m_rangeSubCount_OBSOLETE(),
                                              m_locationOfSubs_OBSOLETE());
}
