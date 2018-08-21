/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Equinor ASA
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
#include "RimMswCompletionParameters.h"

#include "RimWellPath.h"


namespace caf {
    template<>
    void RimMswCompletionParameters::PressureDropEnum::setUp()
    {
        addItem(RimMswCompletionParameters::HYDROSTATIC, "H--", "Hydrostatic");
        addItem(RimMswCompletionParameters::HYDROSTATIC_FRICTION, "HF-", "Hydrostatic + Friction");
        addItem(RimMswCompletionParameters::HYDROSTATIC_FRICTION_ACCELERATION, "HFA", "Hydrostatic + Friction + Acceleration");
        setDefault(RimMswCompletionParameters::HYDROSTATIC);
    }

    template<>
    void RimMswCompletionParameters::LengthAndDepthEnum::setUp()
    {
        addItem(RimMswCompletionParameters::INC, "INC", "Incremental");
        addItem(RimMswCompletionParameters::ABS, "ABS", "Absolute");
        setDefault(RimMswCompletionParameters::INC);
    }
}

CAF_PDM_SOURCE_INIT(RimMswCompletionParameters, "RimMswCompletionParameters");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters::RimMswCompletionParameters()
{
    CAF_PDM_InitObject("MSW Completion Parameters", ":/CompletionsSymbol16x16.png", "", "");
    CAF_PDM_InitField(&m_linerDiameter, "LinerDiameter", std::numeric_limits<double>::infinity(), "Liner Inner Diameter", "", "", "");
    CAF_PDM_InitField(&m_roughnessFactor, "RoughnessFactor", std::numeric_limits<double>::infinity(), "Roughness Factor", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_pressureDrop, "PressureDrop", "Pressure Drop", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_lengthAndDepth, "LengthAndDepth", "Length and Depth", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters::~RimMswCompletionParameters()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::linerDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const
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
double RimMswCompletionParameters::defaultLinerDiameter(RiaEclipseUnitTools::UnitSystem unitSystem)
{
    if (unitSystem == RiaEclipseUnitTools::UNITS_METRIC)
    {
        return 0.152;
    }
    else
    {
        return 0.5;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimMswCompletionParameters::roughnessFactor(RiaEclipseUnitTools::UnitSystem unitSystem) const
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
double RimMswCompletionParameters::defaultRoughnessFactor(RiaEclipseUnitTools::UnitSystem unitSystem)
{
    if (unitSystem == RiaEclipseUnitTools::UNITS_METRIC)
    {
        return 1.0e-5;
    }
    else
    {
        return 3.28e-5;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters::PressureDropEnum RimMswCompletionParameters::pressureDrop() const
{
    return m_pressureDrop();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters::LengthAndDepthEnum RimMswCompletionParameters::lengthAndDepth() const
{
    return m_lengthAndDepth();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setLinerDiameter(double diameter)
{
    m_linerDiameter = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setRoughnessFactor(double roughnessFactor)
{
    m_roughnessFactor = roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setPressureDrop(PressureDropType pressureDropType)
{
    m_pressureDrop = pressureDropType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setLengthAndDepth(LengthAndDepthType lengthAndDepthType)
{
    m_lengthAndDepth = lengthAndDepthType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        RimWellPath* wellPath;
        firstAncestorOrThisOfType(wellPath);
        if (wellPath)
        {
            if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
            {
                m_linerDiameter.uiCapability()->setUiName("Liner Inner Diameter [m]");
                m_roughnessFactor.uiCapability()->setUiName("Roughness Factor [m]");
            }
            else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
            {
                m_linerDiameter.uiCapability()->setUiName("Liner Inner Diameter [ft]");
                m_roughnessFactor.uiCapability()->setUiName("Roughness Factor [ft]");
            }
        }
    }

    uiOrdering.add(&m_linerDiameter);
    uiOrdering.add(&m_roughnessFactor);
    uiOrdering.add(&m_pressureDrop);
    uiOrdering.add(&m_lengthAndDepth);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::initAfterRead()
{
    if (m_linerDiameter() == std::numeric_limits<double>::infinity() &&
        m_roughnessFactor() == std::numeric_limits<double>::infinity())
    {
        setUnitSystemSpecificDefaults();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswCompletionParameters::setUnitSystemSpecificDefaults()
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfType(wellPath);
    if (wellPath)
    {
        m_linerDiameter   = defaultLinerDiameter(wellPath->unitSystem());
        m_roughnessFactor = defaultRoughnessFactor(wellPath->unitSystem());
    }
}

