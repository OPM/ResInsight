/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RimWellBoreStabilityPlot.h"

#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechCase.h"
#include "RimTools.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogFile.h"

#include "cafPdmBase.h"
#include "cafPdmObject.h"
#include "cafPdmUiGroup.h"

namespace caf
{
template<>
void RimWellBoreStabilityPlot::ParameterSourceEnum::setUp()
{
    addItem(RigGeoMechWellLogExtractor::AUTO, "AUTO", "Automatic");
    addItem(RigGeoMechWellLogExtractor::GRID, "GRID", "Grid");
    addItem(RigGeoMechWellLogExtractor::LAS_FILE, "LAS_FILE", "LAS File");
    addItem(RigGeoMechWellLogExtractor::ELEMENT_PROPERTY_TABLE, "ELEMENT_PROPERTY_TABLE", "Element Property Table");
    addItem(RigGeoMechWellLogExtractor::USER_DEFINED, "USER_DEFINED", "User Defined");
    addItem(RigGeoMechWellLogExtractor::HYDROSTATIC_PP, "HYDROSTATIC_PP", "Hydrostatic");
    setDefault(RigGeoMechWellLogExtractor::AUTO);
}
} // End namespace caf

CAF_PDM_SOURCE_INIT(RimWellBoreStabilityPlot, "WellBoreStabilityPlot");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellBoreStabilityPlot::RimWellBoreStabilityPlot()
{
    CAF_PDM_InitObject("Well Bore Stability Plot", ":/WellLogPlot16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(
        &m_porePressureSource, "PorePressureSource", "Pore Pressure", "", "Data source for Pore Pressure", "");
    CAF_PDM_InitFieldNoDefault(
        &m_poissonRatioSource, "PoissionRatioSource", "Poisson Ratio", "", "Data source for Poisson Ratio", "");
    CAF_PDM_InitFieldNoDefault(&m_ucsSource, "UcsSource", "Uniaxial Compressive Strength", "", "Data source for UCS", "");

    CAF_PDM_InitField(&m_userDefinedPoissionRatio, "UserPoissionRatio", 0.25, "", "", "User defined Poisson Ratio", "");
    m_userDefinedPoissionRatio.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    // Typical UCS: http://ceae.colorado.edu/~amadei/CVEN5768/PDF/NOTES8.pdf
    // Typical UCS for Shale is 5 - 100 MPa -> 50 - 1000 bar.
    CAF_PDM_InitField(&m_userDefinedUcs, "UserUcs", 100.0, "", "", "User defined UCS [bar]", "");
    m_userDefinedUcs.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor::WbsParameterSource RimWellBoreStabilityPlot::porePressureSource() const
{
    return m_porePressureSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor::WbsParameterSource RimWellBoreStabilityPlot::poissonRatioSource() const
{
    return m_poissonRatioSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor::WbsParameterSource RimWellBoreStabilityPlot::ucsSource() const
{
    return m_ucsSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellBoreStabilityPlot::userDefinedPoissonRatio() const
{
    return m_userDefinedPoissionRatio();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellBoreStabilityPlot::userDefinedUcs() const
{
    return m_userDefinedUcs();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* parameterSources = uiOrdering.addNewGroup("Parameter Sources");
    parameterSources->add(&m_porePressureSource);
    parameterSources->add(&m_poissonRatioSource);
    parameterSources->add(&m_userDefinedPoissionRatio, {false, 1, 1});
    parameterSources->add(&m_ucsSource);
    parameterSources->add(&m_userDefinedUcs, {false, 1, 1});

    m_userDefinedPoissionRatio.uiCapability()->setUiReadOnly(m_poissonRatioSource() != RigGeoMechWellLogExtractor::USER_DEFINED);
    m_userDefinedUcs.uiCapability()->setUiReadOnly(m_ucsSource() != RigGeoMechWellLogExtractor::USER_DEFINED);
    RimWellLogPlot::defineUiOrdering(uiConfigName, uiOrdering);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellBoreStabilityPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                              bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options = RimWellLogPlot::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);

    RimWellPath*    wellPath    = m_commonDataSource->wellPathToApply();
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(m_commonDataSource->caseToApply());
    int             timeStep    = m_commonDataSource->timeStepToApply();

    RigFemPartResultsCollection* femPartResults = nullptr;
    if (geoMechCase)
    {
        femPartResults = geoMechCase->geoMechData()->femPartResults();
    }

    if (fieldNeedingOptions == &m_porePressureSource)
    {
        for (auto source : RigGeoMechWellLogExtractor::supportedSourcesForPorePressure())
        {
            if (source == RigGeoMechWellLogExtractor::LAS_FILE)
            {
                if (wellPath && !RimWellLogFile::findMdAndChannelValuesForWellPath(wellPath, "PP").empty())
                {
                    options.push_back(caf::PdmOptionItemInfo(ParameterSourceEnum::uiText(source), source));
                }
            }
            else if (source == RigGeoMechWellLogExtractor::ELEMENT_PROPERTY_TABLE)
            {
                RigFemResultAddress resAddr(RIG_ELEMENT, "POR", "");
                if (timeStep > 0 && femPartResults && !femPartResults->resultValues(resAddr, 0, timeStep).empty())
                {
                    options.push_back(caf::PdmOptionItemInfo(ParameterSourceEnum::uiText(source), source));
                }
            }
            else
            {
                options.push_back(caf::PdmOptionItemInfo(ParameterSourceEnum::uiText(source), source));
            }
        }
    }
    else if (fieldNeedingOptions == &m_poissonRatioSource)
    {
        for (auto source : RigGeoMechWellLogExtractor::supportedSourcesForPoissonRatio())
        {
            if (source == RigGeoMechWellLogExtractor::LAS_FILE)
            {
                if (wellPath && !RimWellLogFile::findMdAndChannelValuesForWellPath(wellPath, "POISSON_RATIO").empty())
                {
                    options.push_back(caf::PdmOptionItemInfo(ParameterSourceEnum::uiText(source), source));
                }
            }
            else if (source == RigGeoMechWellLogExtractor::ELEMENT_PROPERTY_TABLE)
            {
                RigFemResultAddress resAddr(RIG_ELEMENT, "RATIO", "");
                if (timeStep > 0 && femPartResults && !femPartResults->resultValues(resAddr, 0, timeStep).empty())
                {
                    options.push_back(caf::PdmOptionItemInfo(ParameterSourceEnum::uiText(source), source));
                }
            }
            else
            {
                options.push_back(caf::PdmOptionItemInfo(ParameterSourceEnum::uiText(source), source));
            }
        }
    }
    else if (fieldNeedingOptions == &m_ucsSource)
    {
        for (auto source : RigGeoMechWellLogExtractor::supportedSourcesForUcs())
        {
            if (source == RigGeoMechWellLogExtractor::LAS_FILE)
            {
                if (wellPath && !RimWellLogFile::findMdAndChannelValuesForWellPath(wellPath, "UCS").empty())
                {
                    options.push_back(caf::PdmOptionItemInfo(ParameterSourceEnum::uiText(source), source));
                }
            }
            else if (source == RigGeoMechWellLogExtractor::ELEMENT_PROPERTY_TABLE)
            {
                RigFemResultAddress resAddr(RIG_ELEMENT, "UCS", "");
                if (timeStep > 0 && femPartResults && !femPartResults->resultValues(resAddr, 0, timeStep).empty())
                {
                    options.push_back(caf::PdmOptionItemInfo(ParameterSourceEnum::uiText(source), source));
                }
            }
            else
            {
                options.push_back(caf::PdmOptionItemInfo(ParameterSourceEnum::uiText(source), source));
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                const QVariant&            oldValue,
                                                const QVariant&            newValue)
{
    RimWellLogPlot::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_porePressureSource || changedField == &m_poissonRatioSource || changedField == &m_ucsSource ||
        changedField == &m_userDefinedPoissionRatio || changedField == &m_userDefinedUcs)
    {
        this->loadDataAndUpdate();
    }
}
