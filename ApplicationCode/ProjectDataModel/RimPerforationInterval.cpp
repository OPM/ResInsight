/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
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

#include "RimPerforationInterval.h"

#include "RigWellPath.h"

#include "RimProject.h"
#include "RimWellPath.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiTextEditor.h"

CAF_PDM_SOURCE_INIT(RimPerforationInterval, "Perforation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPerforationInterval::RimPerforationInterval()
{
    CAF_PDM_InitObject("Perforation", ":/Default.png", "", "");

    CAF_PDM_InitField(&m_startMD, "StartMeasuredDepth", 0.0, "Start MD [m]", "", "", "");
    CAF_PDM_InitField(&m_endMD, "EndMeasuredDepth", 0.0, "End MD [m]", "", "", "");
    CAF_PDM_InitField(&m_diameter, "Diameter", 0.0, "Diameter [m]", "", "", "");
    CAF_PDM_InitField(&m_skinFactor, "SkinFactor", 0.0, "Skin Factor", "", "", "");

    m_name.uiCapability()->setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPerforationInterval::~RimPerforationInterval()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::setStartAndEndMD(double startMD, double endMD)
{
    m_startMD = startMD;
    m_endMD = endMD;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimPerforationInterval::boundingBoxInDomainCoords()
{
    cvf::BoundingBox bb;

    RimWellPath* wellPath = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(wellPath);

    RigWellPath* rigWellPath = wellPath->wellPathGeometry();
    if (rigWellPath)
    {
        bb.add(rigWellPath->interpolatedPointAlongWellPath(startMD()));
        bb.add(rigWellPath->interpolatedPointAlongWellPath(endMD()));
    }

    return bb;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    m_name = QString("%1 - %2").arg(m_startMD).arg(m_endMD);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_startMD);
    uiOrdering.add(&m_endMD);
    uiOrdering.add(&m_diameter);
    uiOrdering.add(&m_skinFactor);

    uiOrdering.skipRemainingFields();
}

