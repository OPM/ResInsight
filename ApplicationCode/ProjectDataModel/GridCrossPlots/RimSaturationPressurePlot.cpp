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

#include "RimSaturationPressurePlot.h"

#include "RimEclipseResultCase.h"
#include "RimGridCrossPlotCurveSet.h"
#include "RimPlotAxisAnnotation.h"
#include "RimPlotAxisProperties.h"

CAF_PDM_SOURCE_INIT(RimSaturationPressurePlot, "RimSaturationPressurePlot");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSaturationPressurePlot::RimSaturationPressurePlot()
{
    CAF_PDM_InitObject("Saturation Pressure Plot", ":/SummaryXPlotLight16x16.png", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSaturationPressurePlot::assignCaseAndEquilibriumRegion(RimEclipseResultCase* eclipseResultCase, int equilibriumRegion)
{
    {
        RimGridCrossPlotCurveSet* curveSet = createCurveSet();
        curveSet->setFromCaseAndEquilibriumRegion(eclipseResultCase, "PRESSURE");
    }
    {
        RimGridCrossPlotCurveSet* curveSet = createCurveSet();
        curveSet->setFromCaseAndEquilibriumRegion(eclipseResultCase, "PDEW");
    }
    {
        RimGridCrossPlotCurveSet* curveSet = createCurveSet();
        curveSet->setFromCaseAndEquilibriumRegion(eclipseResultCase, "PBUB");
    }

    RimPlotAxisProperties* yAxisProps = yAxisProperties();

    yAxisProps->setInvertedAxis(true);

    {
        RimPlotAxisAnnotation* annotation = new RimPlotAxisAnnotation;
        annotation->setEquilibriumData(eclipseResultCase, equilibriumRegion, RimPlotAxisAnnotation::PL_EQUIL_GAS_OIL_CONTACT);

        yAxisProps->appendAnnotation(annotation);
    }
    {
        RimPlotAxisAnnotation* annotation = new RimPlotAxisAnnotation;
        annotation->setEquilibriumData(eclipseResultCase, equilibriumRegion, RimPlotAxisAnnotation::PL_EQUIL_WATER_OIL_CONTACT);

        yAxisProps->appendAnnotation(annotation);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSaturationPressurePlot::initAfterRead()
{
    yAxisProperties()->showAnnotationObjectsInProjectTree();

    RimGridCrossPlot::initAfterRead();
}
