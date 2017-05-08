/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimProject.h"
#include "RigFishbonesGeometry.h"

#include "cafPdmUiListEditor.h"

#include "cvfAssert.h"

#include <cstdlib>


CAF_PDM_SOURCE_INIT(RimFishbonesMultipleSubs, "FishbonesMultipleSubs");

namespace caf {
    template<>
    void AppEnum<RimFishbonesMultipleSubs::LocationType>::setUp()
    {
        addItem(RimFishbonesMultipleSubs::FB_SUB_COUNT_END,     "FB_SUB_COUNT",     "Start/End/Number of Subs");
        addItem(RimFishbonesMultipleSubs::FB_SUB_SPACING_END,   "FB_SUB_SPACING",   "Start/End/Spacing");
        addItem(RimFishbonesMultipleSubs::FB_SUB_USER_DEFINED,  "FB_SUB_CUSTOM",    "User Specification");
        setDefault(RimFishbonesMultipleSubs::FB_SUB_USER_DEFINED);
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
    CAF_PDM_InitObject("FishbonesMultipleSubs", ":/Default.png", "", "");

    CAF_PDM_InitField(&m_lateralCountPerSub,            "LateralCountPerSub", size_t(3),    "Laterals Per Sub", "", "", "");
    CAF_PDM_InitField(&m_lateralLength,                 "LateralLength",  QString("12.0"),  "Length(s) [m]", "", "Specify multiple length values if the sub lengths differ", "");

    CAF_PDM_InitField(&m_lateralExitAngle,              "LateralExitAngle", 35.0,           "Exit Angle [deg]", "", "", "");
    CAF_PDM_InitField(&m_lateralBuildAngle,             "LateralBuildAngle", 5.0,           "Build Angle [deg/m]", "", "", "");

    CAF_PDM_InitField(&m_lateralHoleRadius,             "LateralHoleRadius", 12.0,          "Hole Radius [mm]", "", "", "");
    CAF_PDM_InitField(&m_lateralTubingRadius,           "LateralTubingRadius", 8.0,         "Tubing Radius [mm]", "", "", "");

    CAF_PDM_InitField(&m_lateralOpenHoleRoghnessFactor, "LateralOpenHoleRoghnessFactor", 0.001,   "Open Hole Roghness Factor [m]", "", "", "");
    CAF_PDM_InitField(&m_lateralTubingRoghnessFactor,   "LateralTubingRoghnessFactor", 1e-5,      "Tubing Roghness Factor [m]", "", "", "");

    CAF_PDM_InitField(&m_lateralLengthFraction,         "LateralLengthFraction", 0.8,       "Length Fraction [0..1]", "", "", "");
    CAF_PDM_InitField(&m_lateralInstallFraction,        "LateralInstallFraction", 0.7,      "Install Fraction [0..1]", "", "", "");
    CAF_PDM_InitField(&m_skinFactor,                    "SkinFactor", 1.0,                  "Skin Factor [0..1]", "", "", "");

    CAF_PDM_InitField(&m_icdCount,                      "IcdCount", size_t(2),              "ICD Count", "", "", "");
    CAF_PDM_InitField(&m_icdOrificeRadius,              "IcdOrificeRadius", 8.0,            "ICD Orifice Radius [mm]", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_locationOfSubs,       "LocationOfSubs",                   "Measured Depths [m]", "", "", "");
    m_locationOfSubs.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_subsLocationMode,              "SubsLocationMode", caf::AppEnum<LocationType>(FB_SUB_USER_DEFINED), "Location Defined By", "", "", "");
    CAF_PDM_InitField(&m_rangeStart,                    "RangeStart",       100.0,          "Start MD [m]", "", "", "");
    CAF_PDM_InitField(&m_rangeEnd,                      "RangeEnd",         250.0,          "End MD [m]", "", "", "");
    CAF_PDM_InitField(&m_rangeSubSpacing,               "RangeSubSpacing",  40.0,           "Spacing [m]", "", "", "");
    CAF_PDM_InitField(&m_rangeSubCount,                 "RangeSubCount",    size_t(25),     "Number of Subs", "", "", "");

    CAF_PDM_InitField(&m_subsOrientationMode,           "SubsOrientationMode", caf::AppEnum<LateralsOrientationType>(FB_LATERAL_ORIENTATION_RANDOM), "Orientation", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&m_installationRotationAngles, "InstallationRotationAngles", "Installation Rotation Angles [deg]", "", "", "");
    m_installationRotationAngles.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&m_fixedInstallationRotationAngle, "FixedInstallationRotationAngle", 0.0, "  Fixed Angle [deg]", "", "", "");

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
std::vector<double> RimFishbonesMultipleSubs::locationOfSubs() const
{
    return m_locationOfSubs;
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
double RimFishbonesMultipleSubs::tubingRadius() const
{
    return m_lateralTubingRadius;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesMultipleSubs::lateralCountPerSub() const
{
    return m_lateralCountPerSub;
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

    if (recomputeLocations)
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
            std::vector<double> measuredDepths = locationsFromStartSpacingAndCount(m_rangeStart, m_rangeSubSpacing, m_rangeSubCount);
            m_locationOfSubs = measuredDepths;
        }
    }

    if (recomputeLocations ||
        changedField == &m_locationOfSubs ||
        changedField == &m_subsOrientationMode)
    {
        computeRotationAngles();
    }

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesMultipleSubs::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_name); // From RimNamedObject

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

        {
            caf::PdmUiGroup* wellGroup = lateralConfigGroup->addNewGroup("Well Properties");

            wellGroup->add(&m_lateralHoleRadius);
            wellGroup->add(&m_skinFactor);
        }

        {
            caf::PdmUiGroup* successGroup = lateralConfigGroup->addNewGroup("Installation Success Fractions");
            successGroup->add(&m_lateralLengthFraction);
            successGroup->add(&m_lateralInstallFraction);
        }
    }

    {
        caf::PdmUiGroup* mswGroup = uiOrdering.addNewGroup("Multi Segment Wells");
        mswGroup->setCollapsedByDefault(true);
        mswGroup->add(&m_lateralTubingRadius);
        mswGroup->add(&m_lateralOpenHoleRoghnessFactor);
        mswGroup->add(&m_lateralTubingRoghnessFactor);
        mswGroup->add(&m_icdCount);
        mswGroup->add(&m_icdOrificeRadius);
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

