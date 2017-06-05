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

#include "RigFishbonesGeometry.h"
#include "RigWellPath.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "cafPdmUiListEditor.h"

#include "cvfAssert.h"
#include "cvfBoundingBox.h"

#include <cstdlib>


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

    CAF_PDM_InitField(&fishbonesColor,                  "FishbonesColor", cvf::Color3f(0.999f, 0.333f, 0.999f), "Fishbones Color", "", "", "");

    CAF_PDM_InitField(&m_lateralCountPerSub,            "LateralCountPerSub", size_t(3),    "Laterals Per Sub", "", "", "");
    CAF_PDM_InitField(&m_lateralLength,                 "LateralLength",  QString("11.0"),  "Length(s) [m]", "", "Specify multiple length values if the sub lengths differ", "");

    CAF_PDM_InitField(&m_lateralExitAngle,              "LateralExitAngle", 35.0,           "Exit Angle [deg]", "", "", "");
    CAF_PDM_InitField(&m_lateralBuildAngle,             "LateralBuildAngle", 6.0,           "Build Angle [deg/m]", "", "", "");

    CAF_PDM_InitField(&m_lateralTubingDiameter,         "LateralTubingDiameter", 8.0,       "Tubing Diameter [mm]", "", "", "");

    CAF_PDM_InitField(&m_lateralOpenHoleRoghnessFactor, "LateralOpenHoleRoghnessFactor", 0.001,   "Open Hole Roghness Factor [m]", "", "", "");
    CAF_PDM_InitField(&m_lateralTubingRoghnessFactor,   "LateralTubingRoghnessFactor", 1e-5,      "Tubing Roghness Factor [m]", "", "", "");

    CAF_PDM_InitField(&m_lateralInstallSuccessFraction, "LateralInstallSuccessFraction", 0.7,     "Install Success Rate [0..1]", "", "", "");

    CAF_PDM_InitField(&m_icdCount,                      "IcdCount", size_t(2),              "ICDs per Sub", "", "", "");
    CAF_PDM_InitField(&m_icdOrificeDiameter,            "IcdOrificeDiameter", 7.0,          "ICD Orifice Diameter [mm]", "", "", "");
    CAF_PDM_InitField(&m_icdFlowCoefficient,            "IcdFlowCoeficcient", -1.0,         "ICD Flow Coefficient", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_locationOfSubs,       "LocationOfSubs",                   "Measured Depths [m]", "", "", "");
    m_locationOfSubs.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_subsLocationMode,              "SubsLocationMode", caf::AppEnum<LocationType>(FB_SUB_COUNT_END), "Location Defined By", "", "", "");
    CAF_PDM_InitField(&m_rangeStart,                    "RangeStart",       100.0,          "Start MD [m]", "", "", "");
    CAF_PDM_InitField(&m_rangeEnd,                      "RangeEnd",         250.0,          "End MD [m]", "", "", "");
    CAF_PDM_InitField(&m_rangeSubSpacing,               "RangeSubSpacing",  13.0,           "Spacing [m]", "", "", "");
    CAF_PDM_InitField(&m_rangeSubCount,                 "RangeSubCount",    size_t(25),     "Number of Subs", "", "", "");

    CAF_PDM_InitField(&m_subsOrientationMode,           "SubsOrientationMode", caf::AppEnum<LateralsOrientationType>(FB_LATERAL_ORIENTATION_RANDOM), "Orientation", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&m_installationRotationAngles, "InstallationRotationAngles", "Installation Rotation Angles [deg]", "", "", "");
    m_installationRotationAngles.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&m_fixedInstallationRotationAngle, "FixedInstallationRotationAngle", 0.0, "  Fixed Angle [deg]", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_pipeProperties, "PipeProperties", "Pipe Properties", "", "", "");
    m_pipeProperties.uiCapability()->setUiHidden(true);
    m_pipeProperties.uiCapability()->setUiTreeChildrenHidden(true);

    m_pipeProperties = new RimFishbonesPipeProperties;

    m_name.uiCapability()->setUiReadOnly(true);

    m_rigFishbonesGeometry = std::unique_ptr<RigFisbonesGeometry>(new RigFisbonesGeometry(this));

    computeSubLateralIndices();
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
void RimFishbonesMultipleSubs::setMeasuredDepthAndCount(double measuredDepth, double spacing, int subCount)
{
    m_subsLocationMode = FB_SUB_SPACING_END;

    m_rangeStart = measuredDepth;
    m_rangeEnd = measuredDepth + spacing * subCount;
    m_rangeSubCount = subCount;

    computeRangesAndLocations();
    computeRotationAngles();
    computeSubLateralIndices();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::measuredDepth(size_t subIndex) const
{
    CVF_ASSERT(subIndex < m_locationOfSubs().size());

    return m_locationOfSubs()[subIndex];
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
double RimFishbonesMultipleSubs::tubingDiameter() const
{
    return m_lateralTubingDiameter;
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
void RimFishbonesMultipleSubs::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    bool recomputeLocations = false;

    if (changedField == &m_subsLocationMode)
    {
        if (m_subsLocationMode == FB_SUB_COUNT_END || m_subsLocationMode == FB_SUB_SPACING_END)
        {
            recomputeLocations = true;
        }
    }
    
    if (changedField == &m_rangeStart ||
        changedField == &m_rangeEnd ||
        changedField == &m_rangeSubCount ||
        changedField == &m_rangeSubSpacing)
    {
        recomputeLocations = true;
    }

    if (changedField == &m_rangeSubSpacing &&
        m_rangeSubSpacing() < 13.0)
    {
        // Minimum distance between fishbones is 13m
        m_rangeSubSpacing = 13.0;
    }

    if (recomputeLocations)
    {
        computeRangesAndLocations();
    }

    if (recomputeLocations ||
        changedField == &m_locationOfSubs ||
        changedField == &m_subsOrientationMode)
    {
        computeRotationAngles();
    }

    if (recomputeLocations ||
        changedField == &m_locationOfSubs ||
        changedField == &m_lateralInstallSuccessFraction ||
        changedField == &m_lateralCountPerSub)
    {
        computeSubLateralIndices();
    }

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::computeRangesAndLocations()
{
    if (m_subsLocationMode == FB_SUB_COUNT_END)
    {
        size_t divisor = 1;
        if (m_rangeSubCount > 2) divisor = m_rangeSubCount - 1;

        m_rangeSubSpacing = fabs(m_rangeStart - m_rangeEnd) / divisor;
    }
    else if (m_subsLocationMode == FB_SUB_SPACING_END)
    {
        m_rangeSubCount = (fabs(m_rangeStart - m_rangeEnd) / m_rangeSubSpacing) + 1;

        if (m_rangeSubCount < 1)
        {
            m_rangeSubCount = 1;
        }
    }

    if (m_subsLocationMode == FB_SUB_COUNT_END || m_subsLocationMode == FB_SUB_SPACING_END)
    {
        std::vector<double> validMeasuredDepths; 
        {
            RimWellPath* wellPath = nullptr;
            this->firstAncestorOrThisOfTypeAsserted(wellPath);
        
            RigWellPath* rigWellPathGeo = wellPath->wellPathGeometry();
            if (rigWellPathGeo && rigWellPathGeo->m_measuredDepths.size() > 1)
            {
                double firstWellPathMD = rigWellPathGeo->m_measuredDepths.front();
                double lastWellPathMD = rigWellPathGeo->m_measuredDepths.back();

                for (auto md : locationsFromStartSpacingAndCount(m_rangeStart, m_rangeSubSpacing, m_rangeSubCount))
                {
                    if (md > firstWellPathMD && md < lastWellPathMD)
                    {
                        validMeasuredDepths.push_back(md);
                    }
                }
            }
        }
        
        m_locationOfSubs = validMeasuredDepths;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Appearance");

        group->add(&fishbonesColor);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Location");

        group->add(&m_subsLocationMode);
        if (m_subsLocationMode() != FB_SUB_USER_DEFINED)
        {
            group->add(&m_rangeStart);
            group->add(&m_rangeEnd);

            if (m_subsLocationMode() == FB_SUB_COUNT_END)
            {
                group->add(&m_rangeSubCount);
                group->add(&m_rangeSubSpacing);
            }
            else if (m_subsLocationMode() == FB_SUB_SPACING_END)
            {
                group->add(&m_rangeSubSpacing);
                group->add(&m_rangeSubCount);
            }
        }

        group->add(&m_locationOfSubs);
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

    // Visibility

    if (m_subsLocationMode == FB_SUB_USER_DEFINED)
    {
        m_locationOfSubs.uiCapability()->setUiReadOnly(false);

        m_rangeSubSpacing.uiCapability()->setUiReadOnly(true);
        m_rangeSubCount.uiCapability()->setUiReadOnly(true);
        m_rangeStart.uiCapability()->setUiReadOnly(true);
        m_rangeEnd.uiCapability()->setUiReadOnly(true);
    }
    else
    {
        m_locationOfSubs.uiCapability()->setUiReadOnly(true);

        m_rangeSubSpacing.uiCapability()->setUiReadOnly(false);
        m_rangeSubCount.uiCapability()->setUiReadOnly(false);
        m_rangeStart.uiCapability()->setUiReadOnly(false);
        m_rangeEnd.uiCapability()->setUiReadOnly(false);

        if (m_subsLocationMode == FB_SUB_COUNT_END)
        {
            m_rangeSubSpacing.uiCapability()->setUiReadOnly(true);
            m_rangeSubCount.uiCapability()->setUiReadOnly(false);
        }
        else
        {
            m_rangeSubSpacing.uiCapability()->setUiReadOnly(false);
            m_rangeSubCount.uiCapability()->setUiReadOnly(true);
        }
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::initAfterRead()
{
    if (m_locationOfSubs().size() != m_installationRotationAngles().size())
    {
        computeRotationAngles();
    }
    computeSubLateralIndices();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    caf::PdmChildArrayField<RimFishbonesMultipleSubs*>* container = dynamic_cast<caf::PdmChildArrayField<RimFishbonesMultipleSubs*>*>(this->parentField());
    CVF_ASSERT(container);

    size_t index = container->index(this);
    m_name = QString("Fishbone %1").arg(index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimFishbonesMultipleSubs::boundingBoxInDomainCoords()
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

    for (size_t i = 0; i < m_locationOfSubs().size(); i++)
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
    for (size_t subIndex = 0; subIndex < m_locationOfSubs().size(); ++subIndex)
    {
        SubLateralIndex subLateralIndex{ subIndex };
        for (size_t lateralIndex = 0; lateralIndex < m_lateralCountPerSub(); ++lateralIndex)
        {
            subLateralIndex.lateralIndices.push_back(lateralIndex);
        }
        m_subLateralIndices.push_back(subLateralIndex);
    }
    double numLaterals = static_cast<double>(m_locationOfSubs().size() * m_lateralCountPerSub);
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
std::vector<double> RimFishbonesMultipleSubs::locationsFromStartSpacingAndCount(double start, double spacing, size_t count)
{
    std::vector<double> measuredDepths;
    
    for (size_t i = 0; i < count; i++)
    {
        measuredDepths.push_back(start + spacing * i);
    }

    return measuredDepths;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimFishbonesMultipleSubs::randomValueFromRange(int min, int max)
{
    int range = abs(max - min);
    int random_integer = min + int(range*rand() / (RAND_MAX + 1.0));
    
    return random_integer;
}

