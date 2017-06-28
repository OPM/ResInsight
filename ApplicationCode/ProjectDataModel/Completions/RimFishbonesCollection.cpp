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

#include "RimFishbonesCollection.h"

#include "RifWellPathImporter.h"

#include "RigWellPath.h"

#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include <QColor>

#include <algorithm>

namespace caf {
    template<>
    void RimFishbonesCollection::PressureDropEnum::setUp()
    {
        addItem(RimFishbonesCollection::HYDROSTATIC,                       "H--", "Hydrostatic");
        addItem(RimFishbonesCollection::HYDROSTATIC_FRICTION,              "HF-", "Hydrostatic + Friction");
        addItem(RimFishbonesCollection::HYDROSTATIC_FRICTION_ACCELERATION, "HFA", "Hydrostatic + Friction + Acceleration");
        setDefault(RimFishbonesCollection::HYDROSTATIC);
    }

    template<>
    void RimFishbonesCollection::LengthAndDepthEnum::setUp()
    {
        addItem(RimFishbonesCollection::INC, "INC", "Incremental");
        addItem(RimFishbonesCollection::ABS, "ABS", "Absolute");
        setDefault(RimFishbonesCollection::INC);
    }
}

CAF_PDM_SOURCE_INIT(RimFishbonesCollection, "FishbonesCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection::RimFishbonesCollection()
{
    CAF_PDM_InitObject("Fishbones", ":/FishBones16x16.png", "", "");

    nameField()->uiCapability()->setUiHidden(true);
    this->setName("Fishbones");

    CAF_PDM_InitFieldNoDefault(&fishbonesSubs, "FishbonesSubs", "fishbonesSubs", "", "", "");

    fishbonesSubs.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellPathCollection, "WellPathCollection", "Well Paths", "", "", "");
    m_wellPathCollection = new RimFishboneWellPathCollection;
    m_wellPathCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_startMD,           "StartMD",          HUGE_VAL,   "Start MD",             "", "", "");
    CAF_PDM_InitField(&m_mainBoreDiameter,  "MainBoreDiameter", 0.216,      "Main Bore Diameter",   "", "", "");
    CAF_PDM_InitField(&m_linerDiameter,     "LinerDiameter",    0.152,      "Liner Inner Diameter", "", "", "");
    CAF_PDM_InitField(&m_roughnessFactor,   "RoughnessFactor",  1e-05,      "Roughness Factor",     "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_pressureDrop, "PressureDrop", "Pressure Drop", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_lengthAndDepth, "LengthAndDepth", "Length and Depth", "", "", "");

    manuallyModifiedStartMD = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishboneWellPathCollection* RimFishbonesCollection::wellPathCollection() const
{
    CVF_ASSERT(m_wellPathCollection);

    return m_wellPathCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_startMD)
    {
        manuallyModifiedStartMD = true;
    }

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        RimWellPath* wellPath;
        firstAncestorOrThisOfType(wellPath);
        if (wellPath)
        {
            if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
            {
                m_startMD.uiCapability()->setUiName("Start MD [m]");
                m_mainBoreDiameter.uiCapability()->setUiName("Main Bore Diameter [m]");
                m_linerDiameter.uiCapability()->setUiName("Liner Inner Diameter [m]");
                m_roughnessFactor.uiCapability()->setUiName("Roughness Factor [m]");
            }
            else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
            {
                m_startMD.uiCapability()->setUiName("Start MD [ft]");
                m_mainBoreDiameter.uiCapability()->setUiName("Main Bore Diameter [ft]");
                m_linerDiameter.uiCapability()->setUiName("Liner Inner Diameter [ft]");
                m_roughnessFactor.uiCapability()->setUiName("Roughness Factor [ft]");
            }
        }
    }

    caf::PdmUiGroup* wellGroup = uiOrdering.addNewGroup("Fishbone Well Properties");
    wellGroup->add(&m_startMD);
    wellGroup->add(&m_mainBoreDiameter);

    caf::PdmUiGroup* mswGroup = uiOrdering.addNewGroup("Multi Segment Wells");
    mswGroup->add(&m_linerDiameter);
    mswGroup->add(&m_roughnessFactor);
    mswGroup->add(&m_pressureDrop);
    mswGroup->add(&m_lengthAndDepth);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::appendFishbonesSubs(RimFishbonesMultipleSubs* subs)
{
    subs->fishbonesColor = nextFishbonesColor();
    fishbonesSubs.push_back(subs);

    subs->setUnitSystemSpecificDefaults();
    subs->recomputeLateralLocations();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimFishbonesCollection::nextFishbonesColor() const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfType(wellPath);
    cvf::Color3ub wellPathColor(wellPath->wellPathColor());
    QColor qWellPathColor = QColor(wellPathColor.r(), wellPathColor.g(), wellPathColor.b());

    if (qWellPathColor.value() == 0)
    {
        // If the color is black, using `lighter` or `darker` will not have any effect, since they multiply `value` by a percentage.
        // In this case, `value` is set specifically to make `lighter`/`darker` possible.
        qWellPathColor.setHsl(qWellPathColor.hue(), qWellPathColor.saturation(), 25);
    }

    QColor qFishbonesColor;

    int newIndex = static_cast<int>(fishbonesSubs.size());

    if (qWellPathColor.lightnessF() < 0.5)
    {
        qFishbonesColor = qWellPathColor.lighter(150 + 50 * newIndex);
    }
    else
    {
        qFishbonesColor = qWellPathColor.darker(150 + 50 * newIndex);
    }

    return cvf::Color3f::fromByteColor(qFishbonesColor.red(), qFishbonesColor.green(), qFishbonesColor.blue());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::recalculateStartMD()
{
    double minStartMD = HUGE_VAL;

    for (const RimFishbonesMultipleSubs* sub : fishbonesSubs())
    {
        for (auto& index : sub->installedLateralIndices())
        {
            minStartMD = std::min(minStartMD, sub->measuredDepth(index.subIndex) - 13.0);
        }
    }

    for (const RimFishboneWellPath* wellPath : m_wellPathCollection->wellPaths())
    {
        if (wellPath->measuredDepths().size() > 0)
        {
            minStartMD = std::min(minStartMD, wellPath->measuredDepths()[0] - 13.0);
        }
    }

    if (!manuallyModifiedStartMD || minStartMD < m_startMD())
    {
        m_startMD = minStartMD;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesCollection::mainBoreDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfTypeAsserted(wellPath);
    if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD && unitSystem == RiaEclipseUnitTools::UNITS_METRIC)
    {
        return RiaEclipseUnitTools::feetToMeter(m_mainBoreDiameter());
    }
    else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC && unitSystem == RiaEclipseUnitTools::UNITS_FIELD)
    {
        return RiaEclipseUnitTools::meterToFeet(m_mainBoreDiameter());
    }
    return m_mainBoreDiameter();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesCollection::linerDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfTypeAsserted(wellPath);
    if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD && unitSystem == RiaEclipseUnitTools::UNITS_METRIC)
    {
        return RiaEclipseUnitTools::feetToMeter(m_linerDiameter());
    }
    else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC && unitSystem == RiaEclipseUnitTools::UNITS_FIELD)
    {
        return RiaEclipseUnitTools::meterToFeet(m_linerDiameter());
    }
    return m_linerDiameter();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFishbonesCollection::roughnessFactor(RiaEclipseUnitTools::UnitSystem unitSystem) const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfTypeAsserted(wellPath);
    if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD && unitSystem == RiaEclipseUnitTools::UNITS_METRIC)
    {
        return RiaEclipseUnitTools::feetToMeter(m_roughnessFactor());
    }
    else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC && unitSystem == RiaEclipseUnitTools::UNITS_FIELD)
    {
        return RiaEclipseUnitTools::meterToFeet(m_roughnessFactor());
    }
    return m_roughnessFactor();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::setUnitSystemSpecificDefaults()
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfType(wellPath);
    if (wellPath)
    {
        if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
        {
            m_mainBoreDiameter = 0.216;
            m_linerDiameter = 0.152;
            m_roughnessFactor = 1e-05;
        }
        else
        {
            m_mainBoreDiameter = 0.708;
            m_linerDiameter = 0.5;
            m_roughnessFactor = 3.28e-05;
        }

        m_wellPathCollection->setUnitSystemSpecificDefaults();
    }
}

