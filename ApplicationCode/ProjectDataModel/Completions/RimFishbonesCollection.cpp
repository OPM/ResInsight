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
#include <cmath>

CAF_PDM_SOURCE_INIT(RimFishbonesCollection, "FishbonesCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection::RimFishbonesCollection()
{
    CAF_PDM_InitObject("Fishbones", ":/FishBones16x16.png", "", "");

    nameField()->uiCapability()->setUiHidden(true);
    this->setName("Fishbones");

    CAF_PDM_InitFieldNoDefault(&m_fishbonesSubs, "FishbonesSubs", "fishbonesSubs", "", "", "");

    m_fishbonesSubs.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellPathCollection, "WellPathCollection", "Imported Laterals", "", "", "");
    m_wellPathCollection = new RimFishboneWellPathCollection;
    m_wellPathCollection.uiCapability()->setUiHidden(true);

    
    CAF_PDM_InitField(&m_startMD, "StartMD", HUGE_VAL, "Start MD", "", "", "");
    CAF_PDM_InitField(&m_mainBoreDiameter,  "MainBoreDiameter", 0.216,      "Main Bore Diameter",   "", "", "");
    CAF_PDM_InitField(&m_skinFactor, "MainBoreSkinFactor", 0., "Main Bore Skin Factor [0..1]", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_mswParameters, "MswParameters", "Multi Segment Well Parameters", "", "", "");
    m_mswParameters = new RimMswCompletionParameters;
    m_mswParameters.uiCapability()->setUiTreeHidden(true);
    m_mswParameters.uiCapability()->setUiTreeChildrenHidden(true);
    manuallyModifiedStartMD = false;

    // Moved to RimMswCompletionParameters and obsoleted
    CAF_PDM_InitField(&m_linerDiameter_OBSOLETE, "LinerDiameter", 0.152, "Liner Inner Diameter", "", "", "");
    CAF_PDM_InitField(&m_roughnessFactor_OBSOLETE, "RoughnessFactor", 1e-05, "Roughness Factor", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_pressureDrop_OBSOLETE, "PressureDrop", "Pressure Drop", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_lengthAndDepth_OBSOLETE, "LengthAndDepth", "Length and Depth", "", "", "");
    m_linerDiameter_OBSOLETE.xmlCapability()->setIOWritable(false);
    m_roughnessFactor_OBSOLETE.xmlCapability()->setIOWritable(false);
    m_pressureDrop_OBSOLETE.xmlCapability()->setIOWritable(false);
    m_lengthAndDepth_OBSOLETE.xmlCapability()->setIOWritable(false);
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
    if (changedField == &m_isChecked)
    {
        proj->reloadCompletionTypeResultsInAllViews();
    }
    else
    {
        proj->createDisplayModelAndRedrawAllViews();
    }
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
            }
            else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
            {
                m_startMD.uiCapability()->setUiName("Start MD [ft]");
                m_mainBoreDiameter.uiCapability()->setUiName("Main Bore Diameter [ft]");
            }
        }
    }

    caf::PdmUiGroup* wellGroup = uiOrdering.addNewGroup("Fishbone Well Properties");
    wellGroup->add(&m_startMD);
    wellGroup->add(&m_mainBoreDiameter);
    wellGroup->add(&m_skinFactor);
    caf::PdmUiGroup* mswGroup = uiOrdering.addNewGroup("Multi Segment Well Properties");
    m_mswParameters->uiOrdering(uiConfigName, *mswGroup);
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::initAfterRead()
{
    if (m_linerDiameter_OBSOLETE() != m_linerDiameter_OBSOLETE.defaultValue())
    {
        m_mswParameters->setLinerDiameter(m_linerDiameter_OBSOLETE());
    }
    if (m_roughnessFactor_OBSOLETE() != m_roughnessFactor_OBSOLETE.defaultValue())
    {
        m_mswParameters->setRoughnessFactor(m_roughnessFactor_OBSOLETE());
    }
    if (m_pressureDrop_OBSOLETE() != m_pressureDrop_OBSOLETE.defaultValue())
    {
        m_mswParameters->setPressureDrop(m_pressureDrop_OBSOLETE());
    }
    if (m_lengthAndDepth_OBSOLETE() != m_lengthAndDepth_OBSOLETE.defaultValue())
    {
        m_mswParameters->setLengthAndDepth(m_lengthAndDepth_OBSOLETE());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::appendFishbonesSubs(RimFishbonesMultipleSubs* subs)
{
    subs->fishbonesColor = nextFishbonesColor();
    m_fishbonesSubs.push_back(subs);

    subs->setUnitSystemSpecificDefaults();
    subs->recomputeLateralLocations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimMswCompletionParameters* RimFishbonesCollection::mswParameters() const
{
    return m_mswParameters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimFishbonesMultipleSubs*> RimFishbonesCollection::fishbonesSubs() const
{
    return m_fishbonesSubs.childObjects();
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

    int newIndex = static_cast<int>(m_fishbonesSubs.size());

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

    for (const RimFishbonesMultipleSubs* sub : m_fishbonesSubs())
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
double RimFishbonesCollection::startMD() const
{
    return m_startMD;
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
void RimFishbonesCollection::setUnitSystemSpecificDefaults()
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfType(wellPath);
    if (wellPath)
    {
        if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
        {
            m_mainBoreDiameter = 0.216;
        }
        else
        {
            m_mainBoreDiameter = 0.708;
        }

        m_wellPathCollection->setUnitSystemSpecificDefaults();
    }
    m_mswParameters->setUnitSystemSpecificDefaults();
}

