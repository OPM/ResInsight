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
#include "RigCaseCellResultsData.h"

#include "RimProject.h"
#include "RimWellPath.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiLineEditor.h"

CAF_PDM_SOURCE_INIT(RimPerforationInterval, "Perforation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPerforationInterval::RimPerforationInterval()
{
    CAF_PDM_InitObject("Perforation", ":/PerforationInterval16x16.png", "", "");

    CAF_PDM_InitField(&m_startMD,        "StartMeasuredDepth", 0.0,   "Start MD [m]", "", "", "");
    CAF_PDM_InitField(&m_endMD,          "EndMeasuredDepth",   0.0,   "End MD [m]", "", "", "");
    CAF_PDM_InitField(&m_diameter,       "Diameter",           0.216, "Diameter [m]", "", "", "");
    CAF_PDM_InitField(&m_skinFactor,     "SkinFactor",         0.0,   "Skin Factor", "", "", "");
    CAF_PDM_InitField(&m_startOfHistory, "StartOfHistory",     true,  "Start of History", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_date,  "StartDate",                 "Start Date", "", "", "");
    m_date.uiCapability()->setUiEditorTypeName(caf::PdmUiLineEditor::uiEditorTypeName());

    nameField()->uiCapability()->setUiReadOnly(true);
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
void RimPerforationInterval::setStartOfHistory()
{
    m_startOfHistory = true;

    m_date.uiCapability()->setUiReadOnly(m_startOfHistory());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::setDate(const QDate& date)
{
    m_startOfHistory = false;
    m_date = QDateTime(date);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::setDiameter(double diameter)
{
    m_diameter = diameter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::setSkinFactor(double skinFactor)
{
    m_skinFactor = skinFactor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimPerforationInterval::isActiveOnDate(const QDateTime& date) const
{
    if (m_startOfHistory())
    {
        return true;
    }
    return m_date() < date;
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

    if (changedField == &m_startOfHistory)
    {
        m_date.uiCapability()->setUiReadOnly(m_startOfHistory());
    }

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    this->setName(QString("%1 - %2").arg(m_startMD).arg(m_endMD));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    m_date.uiCapability()->setUiReadOnly(m_startOfHistory());

    uiOrdering.add(&m_startMD);
    uiOrdering.add(&m_endMD);
    uiOrdering.add(&m_diameter);
    uiOrdering.add(&m_skinFactor);
    uiOrdering.add(&m_startOfHistory);
    uiOrdering.add(&m_date);

    uiOrdering.skipRemainingFields();
}

