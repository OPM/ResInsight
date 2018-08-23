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

#include "cafPdmUiDateEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"

CAF_PDM_SOURCE_INIT(RimPerforationInterval, "Perforation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPerforationInterval::RimPerforationInterval()
{
    CAF_PDM_InitObject("Perforation", ":/PerforationInterval16x16.png", "", "");

    CAF_PDM_InitField(&m_startMD,        "StartMeasuredDepth", 0.0,                             "Start MD", "", "", "");
    CAF_PDM_InitField(&m_endMD,          "EndMeasuredDepth",   0.0,                             "End MD", "", "", "");
    CAF_PDM_InitField(&m_diameter,       "Diameter",           0.216,                           "Diameter", "", "", "");
    CAF_PDM_InitField(&m_skinFactor,     "SkinFactor",         0.0,                             "Skin Factor", "", "", "");

    CAF_PDM_InitField(&m_startOfHistory_OBSOLETE,     "StartOfHistory",     true,                            "All Timesteps", "", "", "");
    m_startOfHistory_OBSOLETE.xmlCapability()->setIOWritable(false);
    
    CAF_PDM_InitField(&m_useCustomStartDate, "UseCustomStartDate", false, "Custom Start Date", "", "", "");
    CAF_PDM_InitField(&m_startDate,         "StartDate",          QDateTime::currentDateTime(),    "Start Date", "", "", "");

    CAF_PDM_InitField(&m_useCustomEndDate,  "UseCustomEndDate", false,                          "Custom End Date", "", "", "");
    CAF_PDM_InitField(&m_endDate,           "EndDate",            QDateTime::currentDateTime(), "End Date", "", "", "");

    nameField()->uiCapability()->setUiReadOnly(true);

    m_startMD.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    m_endMD.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
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
void RimPerforationInterval::enableCustomStartDate(bool enable)
{
    m_useCustomStartDate = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::setCustomStartDate(const QDate& date)
{
    if (date.isValid())
    {
        m_startDate = QDateTime(date);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::enableCustomEndDate(bool enable)
{
    m_useCustomEndDate = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::setCustomEndDate(const QDate& date)
{
    if (date.isValid())
    {
        m_endDate = QDateTime(date);
    }
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
double RimPerforationInterval::startMD() const
{
    return m_startMD();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimPerforationInterval::endMD() const
{
    return m_endMD();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimPerforationInterval::diameter(RiaEclipseUnitTools::UnitSystem unitSystem) const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfTypeAsserted(wellPath);
    if (unitSystem == RiaEclipseUnitTools::UNITS_METRIC && wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
    {
        return RiaEclipseUnitTools::feetToMeter(m_diameter());
    }
    else if (unitSystem == RiaEclipseUnitTools::UNITS_FIELD && wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
    {
        return RiaEclipseUnitTools::meterToFeet(m_diameter());
    }
    return m_diameter();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimPerforationInterval::skinFactor() const
{
    return m_skinFactor();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimPerforationInterval::isActiveOnDate(const QDateTime& date) const
{
    if (m_useCustomStartDate() && date < m_startDate()) return false;

    if (m_useCustomEndDate() && date > m_endDate()) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimPerforationInterval::boundingBoxInDomainCoords() const
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
void RimPerforationInterval::setUnitSystemSpecificDefaults()
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfType(wellPath);
    if (wellPath)
    {
        if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
        {
            m_diameter = 0.216;
        }
        else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
        {
            m_diameter = 0.709;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
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
    {
        RimWellPath* wellPath;
        firstAncestorOrThisOfType(wellPath);
        if (wellPath)
        {
            if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
            {
                m_startMD.uiCapability()->setUiName("Start MD [m]");
                m_endMD.uiCapability()->setUiName("End MD [m]");
                m_diameter.uiCapability()->setUiName("Diameter [m]");
            }
            else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
            {
                m_startMD.uiCapability()->setUiName("Start MD [ft]");
                m_endMD.uiCapability()->setUiName("End MD [ft]");
                m_diameter.uiCapability()->setUiName("Diameter [ft]");
            }
        }
    }
    
    uiOrdering.add(&m_startMD);
    uiOrdering.add(&m_endMD);
    uiOrdering.add(&m_diameter);
    uiOrdering.add(&m_skinFactor);
    
    uiOrdering.add(&m_useCustomStartDate);
    uiOrdering.add(&m_startDate);
    m_startDate.uiCapability()->setUiReadOnly(!m_useCustomStartDate);

    uiOrdering.add(&m_useCustomEndDate);
    uiOrdering.add(&m_endDate);
    m_endDate.uiCapability()->setUiReadOnly(!m_useCustomEndDate);

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_startDate || field == &m_endDate)
    {
        caf::PdmUiDateEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDateEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->dateFormat = "dd MMM yyyy";
        }
    }
    else if (field == &m_startMD || field == &m_endMD)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);

        if (myAttr)
        {
            RimWellPath* rimWellPath = nullptr;
            this->firstAncestorOrThisOfType(rimWellPath);
            if (!rimWellPath) return;

            RigWellPath* wellPathGeo = rimWellPath->wellPathGeometry();
            if (!wellPathGeo) return;

            if (wellPathGeo->m_measuredDepths.size() > 2)
            {
                myAttr->m_minimum = wellPathGeo->m_measuredDepths.front();
                myAttr->m_maximum = wellPathGeo->m_measuredDepths.back();
            }
        }
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationInterval::initAfterRead()
{
    if (!m_startOfHistory_OBSOLETE)
    {
        m_useCustomStartDate = true;
    }
}

