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

CAF_PDM_SOURCE_INIT(RimFishbonesCollection, "FishbonesCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection::RimFishbonesCollection()
{
    CAF_PDM_InitObject("Fishbones", ":/FishBones16x16.png", "", "");

    m_name.uiCapability()->setUiHidden(true);
    m_name = "Fishbones";

    CAF_PDM_InitFieldNoDefault(&fishbonesSubs, "FishbonesSubs", "fishbonesSubs", "", "", "");

    fishbonesSubs.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellPathCollection, "WellPathCollection", "Well Paths", "", "", "");
    m_wellPathCollection = new RimFishboneWellPathCollection;
    m_wellPathCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_startMD,           "StartMD",          HUGE_VAL,   "Start MD",             "", "", "");
    CAF_PDM_InitField(&m_mainBoreDiameter,  "MainBoreDiameter", 0.0,        "Main Bore Diameter",   "", "", "");
    CAF_PDM_InitField(&m_linerDiameter,     "LinerDiameter",    0.0,        "Liner Diameter",       "", "", "");
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
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesCollection::appendFishbonesSubs(RimFishbonesMultipleSubs* subs)
{
    subs->fishbonesColor = nextFishbonesColor();
    fishbonesSubs.push_back(subs);

    recalculateStartMD();
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

    if (minStartMD < m_startMD())
    {
        m_startMD = minStartMD;
    }
}

