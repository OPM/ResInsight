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

#include "RigEclipseCaseData.h"
#include "RigEquil.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimGridCrossPlotCurveSet.h"
#include "RimPlotAxisAnnotation.h"
#include "RimPlotAxisProperties.h"

#include "CellFilters/RimPlotCellPropertyFilter.h"
#include "RigCaseCellResultsData.h"

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
void RimSaturationPressurePlot::assignCaseAndEquilibriumRegion(RiaDefines::PorosityModelType porosityType,
                                                               RimEclipseResultCase*         eclipseResultCase,
                                                               int                           zeroBasedEquilRegionIndex)
{
    nameConfig()->addDataSetNames = false;

    QString caseName  = eclipseResultCase->caseUserDescription();
    QString plotTitle = QString("%1 - EQLNUM %2").arg(caseName).arg(zeroBasedEquilRegionIndex + 1);

    nameConfig()->setCustomName(plotTitle);

    auto equilData = eclipseResultCase->eclipseCaseData()->equilData();
    auto eq        = equilData[zeroBasedEquilRegionIndex];

    double gasOilContactDepth   = eq.gasOilContactDepth();
    double waterOilContactDepth = eq.waterOilContactDepth();

    {
        RimGridCrossPlotCurveSet* curveSet = createCurveSet();
        curveSet->setFromCaseAndEquilibriumRegion(eclipseResultCase, "PRESSURE");
        curveSet->setCustomColor(cvf::Color3::BLUE);

        RimPlotCellPropertyFilter* cellFilter = new RimPlotCellPropertyFilter();
        {
            RimEclipseResultDefinition* resultDefinition = new RimEclipseResultDefinition();
            resultDefinition->setEclipseCase(eclipseResultCase);
            resultDefinition->setResultType(RiaDefines::STATIC_NATIVE);
            resultDefinition->setResultVariable("EQLNUM");

            cellFilter->setResultDefinition(resultDefinition);
        }

        cellFilter->setValueRange(zeroBasedEquilRegionIndex + 1, zeroBasedEquilRegionIndex + 1);

        curveSet->addCellFilter(cellFilter);
    }
    {
        RimGridCrossPlotCurveSet* curveSet = createCurveSet();
        curveSet->setFromCaseAndEquilibriumRegion(eclipseResultCase, "PDEW");
        curveSet->setCustomColor(cvf::Color3::RED);

        {
            RimPlotCellPropertyFilter* cellFilter = new RimPlotCellPropertyFilter();
            {
                RimEclipseResultDefinition* resultDefinition = new RimEclipseResultDefinition();
                resultDefinition->setEclipseCase(eclipseResultCase);
                resultDefinition->setResultType(RiaDefines::STATIC_NATIVE);
                resultDefinition->setResultVariable("EQLNUM");

                cellFilter->setResultDefinition(resultDefinition);
            }

            cellFilter->setValueRange(zeroBasedEquilRegionIndex + 1, zeroBasedEquilRegionIndex + 1);

            curveSet->addCellFilter(cellFilter);
        }

        {
            RigCaseCellResultsData* caseCellResultsData = eclipseResultCase->eclipseCaseData()->results(porosityType);
            if (caseCellResultsData)
            {
                RigEclipseResultAddress depthResultAddress(RiaDefines::STATIC_NATIVE, "DEPTH");

                double minDepth = 0.0;
                double maxDepth = 0.0;
                caseCellResultsData->minMaxCellScalarValues(depthResultAddress, minDepth, maxDepth);

                maxDepth = gasOilContactDepth;

                RimPlotCellPropertyFilter* depthCellFilter = new RimPlotCellPropertyFilter();
                {
                    RimEclipseResultDefinition* resultDefinition = new RimEclipseResultDefinition();
                    resultDefinition->setPorosityModel(porosityType);
                    resultDefinition->setEclipseCase(eclipseResultCase);
                    resultDefinition->setResultType(depthResultAddress.m_resultCatType);
                    resultDefinition->setResultVariable(depthResultAddress.m_resultName);

                    depthCellFilter->setResultDefinition(resultDefinition);
                }

                depthCellFilter->setValueRange(minDepth, maxDepth);

                curveSet->addCellFilter(depthCellFilter);
            }
        }
    }

    {
        RimGridCrossPlotCurveSet* curveSet = createCurveSet();
        curveSet->setFromCaseAndEquilibriumRegion(eclipseResultCase, "PBUB");
        curveSet->setCustomColor(cvf::Color3::GREEN);

        {
            RimPlotCellPropertyFilter* cellFilter = new RimPlotCellPropertyFilter();
            {
                RimEclipseResultDefinition* resultDefinition = new RimEclipseResultDefinition();
                resultDefinition->setEclipseCase(eclipseResultCase);
                resultDefinition->setResultType(RiaDefines::STATIC_NATIVE);
                resultDefinition->setResultVariable("EQLNUM");

                cellFilter->setResultDefinition(resultDefinition);
            }

            cellFilter->setValueRange(zeroBasedEquilRegionIndex + 1, zeroBasedEquilRegionIndex + 1);

            curveSet->addCellFilter(cellFilter);
        }
    
        {
            RigCaseCellResultsData* caseCellResultsData = eclipseResultCase->eclipseCaseData()->results(porosityType);
            if (caseCellResultsData)
            {
                RigEclipseResultAddress depthResultAddress(RiaDefines::STATIC_NATIVE, "DEPTH");

                double minDepth = 0.0;
                double maxDepth = 0.0;
                caseCellResultsData->minMaxCellScalarValues(depthResultAddress, minDepth, maxDepth);

                minDepth = gasOilContactDepth;
                maxDepth = waterOilContactDepth;

                RimPlotCellPropertyFilter* depthCellFilter = new RimPlotCellPropertyFilter();
                {
                    RimEclipseResultDefinition* resultDefinition = new RimEclipseResultDefinition();
                    resultDefinition->setPorosityModel(porosityType);
                    resultDefinition->setEclipseCase(eclipseResultCase);
                    resultDefinition->setResultType(depthResultAddress.m_resultCatType);
                    resultDefinition->setResultVariable(depthResultAddress.m_resultName);

                    depthCellFilter->setResultDefinition(resultDefinition);
                }

                depthCellFilter->setValueRange(minDepth, maxDepth);

                curveSet->addCellFilter(depthCellFilter);
            }
        }

    }

    RimPlotAxisProperties* yAxisProps = yAxisProperties();

    yAxisProps->setInvertedAxis(true);

    {
        RimPlotAxisAnnotation* annotation = new RimPlotAxisAnnotation;
        annotation->setEquilibriumData(
            eclipseResultCase, zeroBasedEquilRegionIndex, RimPlotAxisAnnotation::PL_EQUIL_GAS_OIL_CONTACT);

        yAxisProps->appendAnnotation(annotation);
    }
    {
        RimPlotAxisAnnotation* annotation = new RimPlotAxisAnnotation;
        annotation->setEquilibriumData(
            eclipseResultCase, zeroBasedEquilRegionIndex, RimPlotAxisAnnotation::PL_EQUIL_WATER_OIL_CONTACT);

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
