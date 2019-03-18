/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimSaturationPressurePlotCollection.h"

#include "RigEclipseCaseData.h"
#include "RigEquil.h"

#include "RimEclipseResultCase.h"
#include "RimSaturationPressurePlot.h"

CAF_PDM_SOURCE_INIT(RimSaturationPressurePlotCollection, "RimSaturationPressurePlotCollection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSaturationPressurePlotCollection::RimSaturationPressurePlotCollection()
{
    CAF_PDM_InitObject("Saturation Pressure Plots", ":/SummaryXPlotsLight16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_saturationPressurePlots, "SaturationPressurePlots", "Saturation Pressure Plots", "", "", "");
    m_saturationPressurePlots.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSaturationPressurePlotCollection::~RimSaturationPressurePlotCollection() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSaturationPressurePlotCollection::createSaturationPressurePlots(RimEclipseResultCase* eclipseResultCase)
{
    if (!eclipseResultCase) return;

    RigEclipseCaseData* eclipseCaseData = eclipseResultCase->eclipseCaseData();
    if (!eclipseCaseData) return;

    std::vector<RigEquil> equilData = eclipseCaseData->equilData();
    for (size_t i = 0; i < equilData.size(); i++)
    {
        RimSaturationPressurePlot* plot = new RimSaturationPressurePlot();
        plot->setAsPlotMdiWindow();

        int equilibriumRegion = static_cast<int>(i) + 1;
        plot->assignCaseAndEquilibriumRegion(eclipseResultCase, equilibriumRegion);

        m_saturationPressurePlots.push_back(plot);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSaturationPressurePlot*> RimSaturationPressurePlotCollection::plots()
{
    return m_saturationPressurePlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSaturationPressurePlotCollection::deleteAllChildObjects()
{
    m_saturationPressurePlots.deleteAllChildObjects();
}
