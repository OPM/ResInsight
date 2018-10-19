/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Equinor ASA
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

#include "RimMultipleValveLocations.h"

#include "RigWellPath.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimPerforationInterval.h"
#include "RimWellPath.h"
#include "RimWellPathValve.h"

#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiListEditor.h"

CAF_PDM_SOURCE_INIT(RimMultipleValveLocations, "RimMultipleValveLocations");

namespace caf {
    template<>
    void AppEnum<RimMultipleValveLocations::LocationType>::setUp()
    {
        addItem(RimMultipleValveLocations::VALVE_COUNT, "VALVE_COUNT", "Start/End/Number of Subs");
        addItem(RimMultipleValveLocations::VALVE_SPACING, "VALVE_SPACING", "Start/End/Spacing");
        addItem(RimMultipleValveLocations::VALVE_CUSTOM, "VALVE_CUSTOM", "User Specification");
        setDefault(RimMultipleValveLocations::VALVE_COUNT);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultipleValveLocations::RimMultipleValveLocations()
{
    CAF_PDM_InitObject("RimMultipleValveLocations", ":/FishBoneGroup16x16.png", "", "");


    CAF_PDM_InitField(&m_locationType, "LocationMode", caf::AppEnum<LocationType>(VALVE_COUNT), "Location Defined By", "", "", "");
    CAF_PDM_InitField(&m_rangeStart, "RangeStart", 100.0, "Start MD [m]", "", "", "");
    m_rangeStart.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleValueEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_rangeEnd, "RangeEnd", 250.0, "End MD [m]", "", "", "");
    m_rangeEnd.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleValueEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_rangeValveSpacing, "ValveSpacing", "Spacing [m]", "", "", "");
    m_rangeValveSpacing.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleValueEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_rangeValveCount, "RangeValveCount", 13, "Number of Valves", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_locationOfValves, "LocationOfValves", "Measured Depths [m]", "", "", "");
    m_locationOfValves.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleValveLocations::measuredDepth(size_t valveIndex) const
{
    CVF_ASSERT(valveIndex < m_locationOfValves().size());

    return m_locationOfValves()[valveIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleValveLocations::rangeStart() const
{
    return m_rangeStart;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleValveLocations::rangeEnd() const
{
    return m_rangeEnd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimMultipleValveLocations::valveLocations() const
{
    return m_locationOfValves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleValveLocations::setLocationType(LocationType locationType)
{
    m_locationType = locationType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleValveLocations::computeRangesAndLocations()
{
    if (m_locationType == VALVE_COUNT)
    {
        int divisor = 1;
        if (m_rangeValveCount > 2) divisor = m_rangeValveCount - 1;

        m_rangeValveSpacing = fabs(m_rangeStart - m_rangeEnd) / divisor;
        if (m_rangeValveSpacing < minimumSpacingMeters())
        {
            m_rangeValveSpacing = minimumSpacingMeters();
            m_rangeValveCount = rangeCountFromSpacing();
        }
    }
    else if (m_locationType == VALVE_SPACING)
    {
        m_rangeValveCount = rangeCountFromSpacing();
    }

    if (m_locationType == VALVE_COUNT || m_locationType == VALVE_SPACING)
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

                for (auto md : locationsFromStartSpacingAndCount(m_rangeStart, m_rangeValveSpacing, m_rangeValveCount))
                {
                    if (md >= firstWellPathMD && md <= lastWellPathMD)
                    {
                        validMeasuredDepths.push_back(md);
                    }
                }
            }
        }

        m_locationOfValves = validMeasuredDepths;
    }

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleValveLocations::initFields(LocationType locationType, double rangeStart, double rangeEnd, double valveSpacing, int valveCount, const std::vector<double>& locationOfValves)
{
    if (locationType != VALVE_UNDEFINED)
    {
        m_locationType = locationType;
    }
    if (rangeStart != std::numeric_limits<double>::infinity())
    {
        m_rangeStart = rangeStart;
    }
    if (rangeEnd!= std::numeric_limits<double>::infinity())
    {
        m_rangeEnd = rangeEnd;
    }
    if (valveSpacing != std::numeric_limits<double>::infinity())
    {
        m_rangeValveSpacing = valveSpacing;      
    }
    if (valveCount != -1)
    {
        m_rangeValveCount = valveCount;
    }
    if (!locationOfValves.empty())
    {
        m_locationOfValves = locationOfValves;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleValveLocations::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        RimWellPath* wellPath;
        firstAncestorOrThisOfType(wellPath);
        if (wellPath)
        {
            if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
            {
                m_locationOfValves.uiCapability()->setUiName("Measured Depths [m]");
                m_rangeStart.uiCapability()->setUiName("Start MD [m]");
                m_rangeEnd.uiCapability()->setUiName("End MD [m]");
                m_rangeValveSpacing.uiCapability()->setUiName("Spacing [m]");
            }
            else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
            {
                m_locationOfValves.uiCapability()->setUiName("Measured Depths [ft]");
                m_rangeStart.uiCapability()->setUiName("Start MD [ft]");
                m_rangeEnd.uiCapability()->setUiName("End MD [ft]");
                m_rangeValveSpacing.uiCapability()->setUiName("Spacing [ft]");
            }
        }
    }

    {
        uiOrdering.add(&m_locationType);
        if (m_locationType() != VALVE_CUSTOM)
        {
            uiOrdering.add(&m_rangeStart);
            uiOrdering.add(&m_rangeEnd);

            if (m_locationType() == VALVE_COUNT)
            {
                uiOrdering.add(&m_rangeValveCount);
                uiOrdering.add(&m_rangeValveSpacing);
            }
            else if (m_locationType() == VALVE_SPACING)
            {
                uiOrdering.add(&m_rangeValveSpacing);
                uiOrdering.add(&m_rangeValveCount);
            }
        }

        uiOrdering.add(&m_locationOfValves);
    }

    if (m_locationType() == VALVE_CUSTOM)
    {
        m_locationOfValves.uiCapability()->setUiReadOnly(false);

        m_rangeValveSpacing.uiCapability()->setUiReadOnly(true);
        m_rangeValveCount.uiCapability()->setUiReadOnly(true);
        m_rangeStart.uiCapability()->setUiReadOnly(true);
        m_rangeEnd.uiCapability()->setUiReadOnly(true);
    }
    else
    {
        m_locationOfValves.uiCapability()->setUiReadOnly(true);

        m_rangeValveSpacing.uiCapability()->setUiReadOnly(false);
        m_rangeValveCount.uiCapability()->setUiReadOnly(false);
        m_rangeStart.uiCapability()->setUiReadOnly(false);
        m_rangeEnd.uiCapability()->setUiReadOnly(false);

        if (m_locationType() == VALVE_COUNT)
        {
            m_rangeValveSpacing.uiCapability()->setUiReadOnly(true);
            m_rangeValveCount.uiCapability()->setUiReadOnly(false);
        }
        else
        {
            m_rangeValveSpacing.uiCapability()->setUiReadOnly(false);
            m_rangeValveCount.uiCapability()->setUiReadOnly(true);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleValveLocations::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    bool recomputeLocations = false;

    if (changedField == &m_locationType)
    {
        if (m_locationType == VALVE_COUNT || m_locationType == VALVE_SPACING)
        {
            recomputeLocations = true;
        }
    }

    if (changedField == &m_rangeStart ||
        changedField == &m_rangeEnd ||
        changedField == &m_rangeValveCount ||
        changedField == &m_rangeValveSpacing)
    {
        recomputeLocations = true;
        m_rangeStart = cvf::Math::clamp(m_rangeStart(), rangeMin(), rangeMax());
        m_rangeEnd   = cvf::Math::clamp(m_rangeEnd(),   rangeMin(), rangeMax());
    }

    if (changedField == &m_rangeValveSpacing)
    {
        double minimumDistanceMeter = minimumSpacingMeters();

        RimWellPath* wellPath = nullptr;
        this->firstAncestorOrThisOfTypeAsserted(wellPath);
        if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
        {
            double minimumDistanceFeet = RiaEclipseUnitTools::meterToFeet(minimumDistanceMeter);
            m_rangeValveSpacing = cvf::Math::clamp(m_rangeValveSpacing(), minimumDistanceFeet, std::max(m_rangeValveSpacing(), minimumDistanceFeet));
        }
        else
        {
            m_rangeValveSpacing = cvf::Math::clamp(m_rangeValveSpacing(), minimumDistanceMeter, std::max(m_rangeValveSpacing(), minimumDistanceMeter));
        }
    }

    if (recomputeLocations)
    {
        computeRangesAndLocations();
    }

    RimWellPathComponentInterface* parentCompletion = nullptr;
    this->firstAncestorOrThisOfType(parentCompletion);
    caf::PdmObject* pdmParent = dynamic_cast<caf::PdmObject*>(parentCompletion);

    if (recomputeLocations || changedField == &m_locationOfValves)
    {
        if (parentCompletion)
        {
            RimFishbonesMultipleSubs* fishbones = dynamic_cast<RimFishbonesMultipleSubs*>(parentCompletion);
            RimWellPathValve*         valve     = dynamic_cast<RimWellPathValve*>(parentCompletion);
            if (fishbones)
            {
                fishbones->geometryUpdated();
            }
            else if (valve)
            {
                valve->geometryUpdated();
            }
            
        }
    }
    
    if (pdmParent)
    {
        pdmParent->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMultipleValveLocations::rangeCountFromSpacing() const
{
    int rangeCount = (fabs(m_rangeStart - m_rangeEnd) / m_rangeValveSpacing) + 1;

    if (rangeCount < 1)
    {
        rangeCount = 1;
    }
    return rangeCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleValveLocations::minimumSpacingMeters() const
{
    // Minimum distance between fishbones is 13.0m
    // Use 10.0m to allow for some flexibility
    return 10.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleValveLocations::rangeMin() const
{
    const RimPerforationInterval* perfInterval = nullptr;
    this->firstAncestorOrThisOfType(perfInterval);

    if (perfInterval)
    {
        return perfInterval->startMD();
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleValveLocations::rangeMax() const
{
    const RimPerforationInterval* perfInterval = nullptr;
    this->firstAncestorOrThisOfType(perfInterval);

    if (perfInterval)
    {
        return perfInterval->endMD();
    }

    RimWellPath* wellPath = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(wellPath);

    RigWellPath* rigWellPathGeo = wellPath->wellPathGeometry();
    if (rigWellPathGeo && !rigWellPathGeo->m_measuredDepths.empty())
    {
        double lastWellPathMD = rigWellPathGeo->m_measuredDepths.back();
        return lastWellPathMD;
    }
    return std::numeric_limits<double>::infinity();
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimMultipleValveLocations::locationsFromStartSpacingAndCount(double start, double spacing, size_t count)
{
    std::vector<double> measuredDepths;

    for (size_t i = 0; i < count; i++)
    {
        measuredDepths.push_back(start + spacing * i);
    }

    return measuredDepths;
}
