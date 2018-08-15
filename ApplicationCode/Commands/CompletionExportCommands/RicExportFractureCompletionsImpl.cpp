/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicExportFractureCompletionsImpl.h"

#include "RicWellPathFractureReportItem.h"

#include "RiaLogging.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureContainmentTools.h"
#include "RimFractureTemplate.h"
#include "RimSimWellFracture.h"
#include "RimSimWellFractureCollection.h"
#include "RimSimWellInView.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseToStimPlanCalculator.h"
#include "RigEclipseToStimPlanCellTransmissibilityCalculator.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigFractureTransmissibilityEquations.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigSimWellData.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigTransmissibilityCondenser.h"
#include "RigTransmissibilityEquations.h"
#include "RigWellPath.h"
#include "RigWellPathStimplanIntersector.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicExportFractureCompletionsImpl::generateCompdatValuesForWellPath(
    RimWellPath*                                wellPath,
    RimEclipseCase*                             caseToApply,
    std::vector<RicWellPathFractureReportItem>* fractureDataForReport,
    QTextStream*                                outputStreamForIntermediateResultsText)
{
    std::vector<RimFracture*> fracturesAlongWellPath;

    if (wellPath->fractureCollection()->isChecked())
    {
        for (auto& frac : wellPath->fractureCollection()->fractures)
        {
            if (frac->isChecked())
            {
                frac->ensureValidNonDarcyProperties();

                fracturesAlongWellPath.push_back(frac);
            }
        }
    }

    return generateCompdatValues(caseToApply,
                                 wellPath->completions()->wellNameForExport(),
                                 wellPath->wellPathGeometry(),
                                 fracturesAlongWellPath,
                                 fractureDataForReport,
                                 outputStreamForIntermediateResultsText);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicExportFractureCompletionsImpl::generateCompdatValuesForSimWell(RimEclipseCase*         eclipseCase,
                                                                      const RimSimWellInView* well,
                                                                      QTextStream* outputStreamForIntermediateResultsText)
{
    std::vector<RigCompletionData> completionData;

    auto branches = well->wellPipeBranches();

    for (size_t branchIndex = 0; branchIndex < branches.size(); ++branchIndex)
    {
        std::vector<RimFracture*> fractures;
        for (RimSimWellFracture* fracture : well->simwellFractureCollection->simwellFractures())
        {
            if (fracture->isChecked() && static_cast<size_t>(fracture->branchIndex()) == branchIndex)
            {
                fractures.push_back(fracture);
            }
        }

        std::vector<RigCompletionData> branchCompletions = generateCompdatValues(
            eclipseCase, well->name(), branches[branchIndex], fractures, nullptr, outputStreamForIntermediateResultsText);

        completionData.insert(completionData.end(), branchCompletions.begin(), branchCompletions.end());
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicExportFractureCompletionsImpl::generateCompdatValues(RimEclipseCase*                             caseToApply,
                                                            const QString&                              wellPathName,
                                                            const RigWellPath*                          wellPathGeometry,
                                                            const std::vector<RimFracture*>&            fractures,
                                                            std::vector<RicWellPathFractureReportItem>* fractureDataReportItems,
                                                            QTextStream* outputStreamForIntermediateResultsText)
{
    std::vector<RigCompletionData> fractureCompletions;

    if (!caseToApply || !caseToApply->eclipseCaseData())
    {
        return fractureCompletions;
    }

    double             cDarcyInCorrectUnit = RiaEclipseUnitTools::darcysConstant(caseToApply->eclipseCaseData()->unitsType());
    const RigMainGrid* mainGrid            = caseToApply->eclipseCaseData()->mainGrid();

    // To handle several fractures in the same eclipse cell we need to keep track of the transmissibility
    // to the well from each fracture intersecting the cell and sum these transmissibilities at the end.
    // std::map <eclipseCellIndex ,map< fracture, trans> >
    std::map<size_t, std::map<const RimFracture*, double>> eclCellIdxToTransPrFractureMap;

    for (const RimFracture* fracture : fractures)
    {
        RimFractureTemplate* fracTemplate = fracture->fractureTemplate();

        if (!fracTemplate) continue;

        const RigFractureGrid* fractureGrid = fracTemplate->fractureGrid();
        if (!fractureGrid) continue;

        bool useFiniteConductivityInFracture = (fracTemplate->conductivityType() == RimFractureTemplate::FINITE_CONDUCTIVITY);

        // If finite cond chosen and conductivity not present in stimplan file, do not calculate trans for this fracture
        if (useFiniteConductivityInFracture)
        {
            if (dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate))
            {
                RimStimPlanFractureTemplate* fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate);
                if (!fracTemplateStimPlan->hasConductivity())
                {
                    RiaLogging::error("Trying to export completion data for stimPlan fracture without conductivity data for " +
                                      fracture->name());
                    RiaLogging::error("No transmissibilities will be calculated for " + fracture->name());

                    continue;
                }
            }
        }

        RigTransmissibilityCondenser transCondenser;

        //////
        // Calculate Matrix To Fracture Trans
        RigEclipseToStimPlanCalculator eclToFractureCalc(
            caseToApply, fracture->transformMatrix(), fracTemplate->skinFactor(), cDarcyInCorrectUnit, *fractureGrid);

        eclToFractureCalc.appendDataToTransmissibilityCondenser(fracture, useFiniteConductivityInFracture, &transCondenser);

        //////
        // Calculate Transmissibility in the fracture: From one StimPlan Cell to the other

        if (useFiniteConductivityInFracture)
        {
            for (size_t i = 0; i < fractureGrid->iCellCount(); i++)
            {
                for (size_t j = 0; j < fractureGrid->jCellCount(); j++)
                {
                    size_t fractureCellIndex = fractureGrid->getGlobalIndexFromIJ(i, j);

                    const RigFractureCell& fractureCell = fractureGrid->cellFromIndex(fractureCellIndex);

                    if (!fractureCell.hasNonZeroConductivity()) continue;

                    if (i < fractureGrid->iCellCount() - 1)
                    {
                        size_t                 fractureCellNeighbourXIndex = fractureGrid->getGlobalIndexFromIJ(i + 1, j);
                        const RigFractureCell& fractureCellNeighbourX = fractureGrid->cellFromIndex(fractureCellNeighbourXIndex);

                        double horizontalTransToXneigbour = RigFractureTransmissibilityEquations::centerToCenterFractureCellTrans(
                            fractureCell.getConductivityValue(),
                            fractureCell.cellSizeX(),
                            fractureCell.cellSizeZ(),
                            fractureCellNeighbourX.getConductivityValue(),
                            fractureCellNeighbourX.cellSizeX(),
                            fractureCellNeighbourX.cellSizeZ(),
                            cDarcyInCorrectUnit);

                        transCondenser.addNeighborTransmissibility(
                            {false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellIndex},
                            {false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellNeighbourXIndex},
                            horizontalTransToXneigbour);
                    }

                    if (j < fractureGrid->jCellCount() - 1)
                    {
                        size_t                 fractureCellNeighbourZIndex = fractureGrid->getGlobalIndexFromIJ(i, j + 1);
                        const RigFractureCell& fractureCellNeighbourZ = fractureGrid->cellFromIndex(fractureCellNeighbourZIndex);

                        double verticalTransToZneigbour = RigFractureTransmissibilityEquations::centerToCenterFractureCellTrans(
                            fractureCell.getConductivityValue(),
                            fractureCell.cellSizeZ(),
                            fractureCell.cellSizeX(),
                            fractureCellNeighbourZ.getConductivityValue(),
                            fractureCellNeighbourZ.cellSizeZ(),
                            fractureCellNeighbourZ.cellSizeX(),
                            cDarcyInCorrectUnit);

                        transCondenser.addNeighborTransmissibility(
                            {false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellIndex},
                            {false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellNeighbourZIndex},
                            verticalTransToZneigbour);
                    }
                }
            }
        }

        /////
        // Calculate transmissibility into the well

        if (useFiniteConductivityInFracture)
        {
            ////
            // If fracture has orientation Azimuth or Transverse, assume only radial inflow

            if (fracTemplate->orientationType() == RimFractureTemplate::AZIMUTH ||
                fracTemplate->orientationType() == RimFractureTemplate::TRANSVERSE_WELL_PATH)
            {
                std::pair<size_t, size_t> wellCellIJ    = fractureGrid->fractureCellAtWellCenter();
                size_t                    wellCellIndex = fractureGrid->getGlobalIndexFromIJ(wellCellIJ.first, wellCellIJ.second);

                const RigFractureCell& wellCell = fractureGrid->cellFromIndex(wellCellIndex);

                double radialTrans =
                    RigFractureTransmissibilityEquations::fractureCellToWellRadialTrans(wellCell.getConductivityValue(),
                                                                                        wellCell.cellSizeX(),
                                                                                        wellCell.cellSizeZ(),
                                                                                        fracture->wellRadius(),
                                                                                        fracTemplate->skinFactor(),
                                                                                        cDarcyInCorrectUnit);

                transCondenser.addNeighborTransmissibility(
                    {true, RigTransmissibilityCondenser::CellAddress::WELL, 1},
                    {false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, wellCellIndex},
                    radialTrans);
            }
            else if (fracTemplate->orientationType() == RimFractureTemplate::ALONG_WELL_PATH)
            {
                ////
                // If fracture has orientation along well, linear inflow along well and radial flow at endpoints

                RigWellPathStimplanIntersector wellFractureIntersector(wellPathGeometry, fracture);
                const std::map<size_t, RigWellPathStimplanIntersector::WellCellIntersection>& fractureWellCells =
                    wellFractureIntersector.intersections();

                for (const auto& fracCellIdxIsectDataPair : fractureWellCells)
                {
                    size_t fracWellCellIdx = fracCellIdxIsectDataPair.first;

                    RigWellPathStimplanIntersector::WellCellIntersection intersection = fracCellIdxIsectDataPair.second;

                    const RigFractureCell& fractureWellCell = fractureGrid->cellFromIndex(fracWellCellIdx);

                    double linearTrans = 0.0;
                    if (intersection.hlength > 0.0 || intersection.vlength > 0.0)
                    {
                        linearTrans = RigFractureTransmissibilityEquations::fractureCellToWellLinearTrans(
                            fractureWellCell.getConductivityValue(),
                            fractureWellCell.cellSizeX(),
                            fractureWellCell.cellSizeZ(),
                            intersection.vlength,
                            intersection.hlength,
                            fracture->perforationEfficiency(),
                            fracTemplate->skinFactor(),
                            cDarcyInCorrectUnit);
                    }

                    transCondenser.addNeighborTransmissibility(
                        {true, RigTransmissibilityCondenser::CellAddress::WELL, 1},
                        {false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fracWellCellIdx},
                        linearTrans);
                }
            }
        }

        /////
        // Insert total transmissibility from eclipse-cell to well for this fracture into the map

        std::vector<RigCompletionData> allCompletionsForOneFracture;

        std::set<RigTransmissibilityCondenser::CellAddress> externalCells = transCondenser.externalCells();
        for (RigTransmissibilityCondenser::CellAddress externalCell : externalCells)
        {
            if (externalCell.m_cellIndexSpace == RigTransmissibilityCondenser::CellAddress::ECLIPSE)
            {
                double trans = transCondenser.condensedTransmissibility(
                    externalCell, {true, RigTransmissibilityCondenser::CellAddress::WELL, 1});

                eclCellIdxToTransPrFractureMap[externalCell.m_globalCellIdx][fracture] = trans;

                RigCompletionData compDat(wellPathName,
                                          RigCompletionDataGridCell(externalCell.m_globalCellIdx, caseToApply->mainGrid()),
                                          fracture->fractureMD());

                double diameter = 2.0 * fracture->wellRadius();
                compDat.setFromFracture(trans, fracTemplate->skinFactor(), diameter);
                compDat.addMetadata(fracture->name(), QString::number(trans));
                allCompletionsForOneFracture.push_back(compDat);
            }
        }

        /////
        // Compute Non-Dracy Flow parameters

        if (fracTemplate->isNonDarcyFlowEnabled())
        {
            double dFactorForFracture = fracture->nonDarcyProperties().dFactor;
            double khForFracture      = fracture->nonDarcyProperties().conductivity;

            double sumOfTransmissibilitiesInFracture = 0.0;
            for (const auto& c : allCompletionsForOneFracture)
            {
                sumOfTransmissibilitiesInFracture += c.transmissibility();
            }

            for (auto& c : allCompletionsForOneFracture)
            {
                // NOTE : What is supposed to happen when the transmissibility is close to zero?

                double dFactorForOneConnection = dFactorForFracture * sumOfTransmissibilitiesInFracture / c.transmissibility();
                c.setDFactor(dFactorForOneConnection);

                double khForOneConnection = khForFracture * c.transmissibility() / sumOfTransmissibilitiesInFracture;

                c.setKh(khForOneConnection);
            }
        }

        if (fractureDataReportItems)
        {
            QString                       fractureTemplateName = fracTemplate->name();
            RicWellPathFractureReportItem reportItem(wellPathName, fracture->name(), fractureTemplateName);

            double transmissibility = 0.0;
            double fcd              = -1.0;
            double area             = 0.0;

            for (const auto& c : allCompletionsForOneFracture)
            {
                transmissibility += c.transmissibility();
            }

            auto cellAreas = eclToFractureCalc.eclipseCellAreas();
            for (const auto& cellArea : cellAreas)
            {
                area += cellArea.second;
            }

            reportItem.setData(transmissibility, allCompletionsForOneFracture.size(), fcd, area);

            double conductivity = 0.0;
            double width        = 0.0;
            double height       = 0.0;
            double halfLength   = 0.0;
            {
                auto* ellipseTemplate = dynamic_cast<RimEllipseFractureTemplate*>(fracTemplate);
                if (ellipseTemplate)
                {
                    conductivity = ellipseTemplate->conductivity();
                    width        = ellipseTemplate->width();
                    height       = ellipseTemplate->height();
                    halfLength   = ellipseTemplate->halfLength();
                }

                auto* stimplanTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate);
                if (stimplanTemplate)
                {
                    conductivity = stimplanTemplate->areaWeightedConductivity();
                    width        = stimplanTemplate->areaWeightedWidth();

                    height = stimplanTemplate->longestYRange();

                    double xLength = 0.0;
                    if (height > 1e-9)
                    {
                        xLength = area / height;
                    }

                    // Compute half length defined as (total area / (H/2) )
                    halfLength = xLength / 2.0;
                }
            }
            reportItem.setWidthAndConductivity(width, conductivity);
            reportItem.setHeightAndHalfLength(height, halfLength);

            double areaWeightedEclipseTransmissibility = 0.0;

            if (caseToApply && caseToApply->eclipseCaseData())
            {
                RigCaseCellResultsData* gridCellResults = caseToApply->results(RiaDefines::MATRIX_MODEL);
                if (gridCellResults)
                {
                    gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "TRANX");
                    gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "TRANY");
                    gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "TRANZ");
                }

                cvf::ref<RigResultAccessor> tranxAccessObject = RigResultAccessorFactory::createFromUiResultName(
                    caseToApply->eclipseCaseData(), 0, RiaDefines::MATRIX_MODEL, 0, "TRANX");

                cvf::ref<RigResultAccessor> tranyAccessObject = RigResultAccessorFactory::createFromUiResultName(
                    caseToApply->eclipseCaseData(), 0, RiaDefines::MATRIX_MODEL, 0, "TRANY");

                cvf::ref<RigResultAccessor> tranzAccessObject = RigResultAccessorFactory::createFromUiResultName(
                    caseToApply->eclipseCaseData(), 0, RiaDefines::MATRIX_MODEL, 0, "TRANZ");

                if (tranxAccessObject.notNull() && tranyAccessObject.notNull() && tranzAccessObject.notNull())
                {
                    for (const auto& cellArea : cellAreas)
                    {
                        double tranx = tranxAccessObject->cellScalarGlobIdx(cellArea.first);
                        double trany = tranyAccessObject->cellScalarGlobIdx(cellArea.first);
                        double tranz = tranzAccessObject->cellScalarGlobIdx(cellArea.first);

                        double transmissibilityForCell = RigTransmissibilityEquations::totalConnectionFactor(tranx, trany, tranz);

                        areaWeightedEclipseTransmissibility += transmissibilityForCell * cellArea.second / area;
                    }
                }
            }

            reportItem.setAreaWeightedTransmissibility(areaWeightedEclipseTransmissibility);

            fractureDataReportItems->push_back(reportItem);
        }

        std::copy(
            allCompletionsForOneFracture.begin(), allCompletionsForOneFracture.end(), std::back_inserter(fractureCompletions));

        if (outputStreamForIntermediateResultsText)
        {
            (*outputStreamForIntermediateResultsText)
                << "\n"
                << "\n"
                << "\n----------- All Transmissibilities " << fracture->name() << " -------------------- \n\n";

            (*outputStreamForIntermediateResultsText)
                << QString::fromStdString(transCondenser.neighborTransDebugOutput(mainGrid, fractureGrid));

            (*outputStreamForIntermediateResultsText)
                << "\n"
                << "\n"
                << "\n----------- Condensed Results " << fracture->name() << " -------------------- \n\n";

            (*outputStreamForIntermediateResultsText)
                << QString::fromStdString(transCondenser.condensedTransDebugOutput(mainGrid, fractureGrid));

            (*outputStreamForIntermediateResultsText) << "\n";
        }
    }

    return fractureCompletions;
}
