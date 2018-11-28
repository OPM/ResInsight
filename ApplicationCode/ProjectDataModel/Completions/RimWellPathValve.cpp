/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    Equinor ASA
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

#include "RimWellPathValve.h"

#include "RiaDefines.h"
#include "RiaColorTables.h"
#include "RiaEclipseUnitTools.h"

#include "RigWellPath.h"

#include "RimMultipleValveLocations.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "cafPdmUiDoubleSliderEditor.h"

CAF_PDM_SOURCE_INIT(RimWellPathValve, "WellPathValve");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathValve::RimWellPathValve()
{
    CAF_PDM_InitObject("WellPathValve", ":/ICDValve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_type, "CompletionType", "Type    ", "", "", "");
    m_type = RiaDefines::ICD;

    CAF_PDM_InitField(&m_measuredDepth, "StartMeasuredDepth", 0.0, "Start MD", "", "", "");
    m_measuredDepth.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_multipleValveLocations, "ValveLocations", "Valve Locations", "", "", "");
    m_multipleValveLocations = new RimMultipleValveLocations;
    m_multipleValveLocations.uiCapability()->setUiTreeHidden(true);
    m_multipleValveLocations.uiCapability()->setUiTreeChildrenHidden(true);
    nameField()->uiCapability()->setUiReadOnly(true);

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathValve::~RimWellPathValve()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::setMeasuredDepthAndCount(double startMD, double spacing, int valveCount)
{
    m_measuredDepth = startMD;
    double endMD = startMD + spacing * valveCount;
    m_multipleValveLocations->initFields(RimMultipleValveLocations::VALVE_COUNT, startMD, endMD, spacing, valveCount, {});
    m_multipleValveLocations->computeRangesAndLocations();
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::geometryUpdated()
{
    m_measuredDepth = m_multipleValveLocations->valveLocations().front();

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellPathValve::valveLocations() const
{
    std::vector<double> valveDepths;
    if (m_type() == RiaDefines::ICV)
    {
        valveDepths.push_back(m_measuredDepth);
    }
    else
    {
        valveDepths = m_multipleValveLocations->valveLocations();
    }
    return valveDepths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathValve::isEnabled() const
{
    RimPerforationInterval* perforationInterval = nullptr;
    this->firstAncestorOrThisOfType(perforationInterval);
    return perforationInterval->isEnabled() && isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimWellPathValve::componentType() const
{
    return m_type();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathValve::componentLabel() const
{
    return m_type().uiText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathValve::componentTypeLabel() const
{
    return m_type().uiText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellPathValve::defaultComponentColor() const
{
    return RiaColorTables::wellPathComponentColors()[componentType()];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::startMD() const
{
    if (m_type() == RiaDefines::ICV)
    {
        return m_measuredDepth;
    }
    else if (m_multipleValveLocations()->valveLocations().empty())
    {
        return m_multipleValveLocations->rangeStart();
    }
    else
    {
        return m_multipleValveLocations()->valveLocations().front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::endMD() const
{
    if (m_type() == RiaDefines::ICV)
    {
        return m_measuredDepth + 0.5;
    }
    else if (m_multipleValveLocations()->valveLocations().empty())
    {
        return m_multipleValveLocations->rangeEnd();
    }
    else
    {
        return m_multipleValveLocations()->valveLocations().back();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPathValve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_type)
    {
        std::set<RiaDefines::WellPathComponentType> supportedTypes = { RiaDefines::ICD, RiaDefines::AICD, RiaDefines::ICV };
        for (RiaDefines::WellPathComponentType type : supportedTypes)
        {
            options.push_back(caf::PdmOptionItemInfo(CompletionTypeEnum::uiText(type), type));
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_type);

    if (m_type() == RiaDefines::ICV)
    {
        RimWellPath* wellPath;
        firstAncestorOrThisOfType(wellPath);
        if (wellPath)
        {
            if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
            {
                m_measuredDepth.uiCapability()->setUiName("Measured Depth [m]");
            }
            else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
            {
                m_measuredDepth.uiCapability()->setUiName("Measured Depth [ft]");
            }
        }
        uiOrdering.add(&m_measuredDepth);
    }
    else
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Multiple Valve Locations");
        m_multipleValveLocations->uiOrdering(uiConfigName, *group);
    }
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
      if (field == &m_measuredDepth)
      {
          caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);

          if (myAttr)
          {
              double minimumValue = 0.0, maximumValue = 0.0;

              RimPerforationInterval* perforationInterval = nullptr;
              this->firstAncestorOrThisOfType(perforationInterval);

              if (perforationInterval)
              {
                  minimumValue = perforationInterval->startMD();
                  maximumValue = perforationInterval->endMD();
              }
              else
              {
                  RimWellPath* rimWellPath = nullptr;
                  this->firstAncestorOrThisOfTypeAsserted(rimWellPath);
                  RigWellPath* wellPathGeo = rimWellPath->wellPathGeometry();
                  if (!wellPathGeo) return;

                  if (wellPathGeo->m_measuredDepths.size() > 2)
                  {
                      minimumValue = wellPathGeo->measureDepths().front();
                      maximumValue = wellPathGeo->measureDepths().back();
                  }
              }
              myAttr->m_minimum = minimumValue;
              myAttr->m_maximum = maximumValue;
          }
      }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    QString fullName = componentLabel() + QString(" %1").arg(m_measuredDepth());
    this->setName(fullName);

    if ( m_type() == RiaDefines::ICD )
    {
        this->setUiIcon(QIcon(":/ICDValve16x16.png"));
    } 
    else if ( m_type() == RiaDefines::ICV )
    {
        this->setUiIcon(QIcon(":/ICVValve16x16.png"));
    } 
    else if ( m_type() == RiaDefines::AICD )
    {
        this->setUiIcon(QIcon(":/AICDValve16x16.png"));
    }
}
