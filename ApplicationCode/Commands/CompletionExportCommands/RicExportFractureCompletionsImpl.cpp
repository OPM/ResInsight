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
    std::vector<const RimFracture*> fracturesAlongWellPath;

    for (auto& frac : wellPath->fractureCollection()->activeFractures())
    {
        frac->ensureValidNonDarcyProperties();

        fracturesAlongWellPath.push_back(frac);
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
        std::vector<const RimFracture*> fractures;
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
                                                            const std::vector<const RimFracture*>&      fractures,
                                                            std::vector<RicWellPathFractureReportItem>* fractureDataReportItems,
                                                            QTextStream* outputStreamForIntermediateResultsText)
{
    std::vector<RigCompletionData> fractureCompletions;

    if (!caseToApply || !caseToApply->eclipseCaseData())
    {
        return fractureCompletions;
    }

    {
        // Load the data required by computations to be able to use const access only inside OpenMP loop

        std::vector<QString> resultNames = RigEclipseToStimPlanCellTransmissibilityCalculator::requiredResultNames();

        if (!caseToApply->loadStaticResultsByName(resultNames))
        {
            QString msg;
            msg += "Compdat Export : Required data missing. Required results ";

            for (const auto& r : resultNames)
            {
                msg += " ";
                msg += r;
            }
            RiaLogging::error(msg);

            return fractureCompletions;
        }
    }

    {
        // Load the data required by fracture summary header

        std::vector<QString> resultNames{"TRANX", "TRANY", "TRANZ"};

        caseToApply->loadStaticResultsByName(resultNames);
    }

    {
        // Optional results
        std::vector<QString> resultNames = RigEclipseToStimPlanCellTransmissibilityCalculator::optionalResultNames();

        caseToApply->loadStaticResultsByName(resultNames);
    }

    return generateCompdatValuesConst(
        caseToApply, wellPathName, wellPathGeometry, fractures, fractureDataReportItems, outputStreamForIntermediateResultsText);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicExportFractureCompletionsImpl::generateCompdatValuesConst(
    const RimEclipseCase*                       caseToApply,
    const QString&                              wellPathName,
    const RigWellPath*                          wellPathGeometry,
    const std::vector<const RimFracture*>&      fractures,
    std::vector<RicWellPathFractureReportItem>* fractureDataReportItems,
    QTextStream*                                outputStreamForIntermediateResultsText)
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
    // std::map<size_t, std::map<const RimFracture*, double>> eclCellIdxToTransPrFractureMap;

    std::vector<std::vector<RigCompletionData>> sharedComplForFracture(fractures.size());

#pragma omp parallel for
    for (int i = 0; i < (int)fractures.size(); i++)
    {
        const RimFracture*         fracture     = fractures[i];
        const RimFractureTemplate* fracTemplate = fracture->fractureTemplate();

        if (!fracTemplate) continue;

        const RigFractureGrid* fractureGrid = fracTemplate->fractureGrid();
        if (!fractureGrid) continue;

        bool useFiniteConductivityInFracture = (fracTemplate->conductivityType() == RimFractureTemplate::FINITE_CONDUCTIVITY);

        // If finite cond chosen and conductivity not present in stimplan file, do not calculate trans for this fracture
        if (useFiniteConductivityInFracture && !checkForStimPlanConductivity(fracTemplate, fracture))
        {
            continue;
        }

        RigTransmissibilityCondenser transCondenser;

        //////
        // Calculate Matrix To Fracture Trans
        RigEclipseToStimPlanCalculator eclToFractureCalc(
            caseToApply, fracture->transformMatrix(), fracTemplate->skinFactor(), cDarcyInCorrectUnit, *fractureGrid);

        eclToFractureCalc.appendDataToTransmissibilityCondenser(fracture, useFiniteConductivityInFracture, &transCondenser);

        if (useFiniteConductivityInFracture)
        {
            calculateInternalFractureTransmissibilities(fractureGrid, cDarcyInCorrectUnit, transCondenser);
        }

        if (useFiniteConductivityInFracture)
        {
            calculateFractureToWellTransmissibilities(fracTemplate, fractureGrid, fracture, cDarcyInCorrectUnit, wellPathGeometry, transCondenser);
        }

        /////
        // Insert total transmissibility from eclipse-cell to well for this fracture into the map
        std::map<size_t, double> matrixToWellTrans                  = calculateMatrixToWellTransmissibilities(transCondenser);
        std::vector<RigCompletionData> allCompletionsForOneFracture = generateCompdatValuesForFracture(matrixToWellTrans, wellPathName, caseToApply, fracture, fracTemplate);

        if (fracTemplate->isNonDarcyFlowEnabled())
        {
            computeNonDarcyFlowParameters(fracture, allCompletionsForOneFracture);
        }

        if (fractureDataReportItems)
        {
            QString                       fractureTemplateName = fracTemplate->name();
            RicWellPathFractureReportItem reportItem(wellPathName, fracture->name(), fractureTemplateName);

            auto cellAreas = eclToFractureCalc.eclipseCellAreas();

            double fcd              = -1.0;
            double area = sumUpCellAreas(cellAreas);
            double transmissibility = sumUpTransmissibilities(allCompletionsForOneFracture);

            reportItem.setData(transmissibility, allCompletionsForOneFracture.size(), fcd, area);

            calculateAndSetLengthsAndConductivity(fracTemplate, area, reportItem);
            calculateAndSetAreaWeightedTransmissibility(caseToApply, cellAreas, area, reportItem);

            fractureDataReportItems->push_back(reportItem);
        }

        std::copy(allCompletionsForOneFracture.begin(),
                  allCompletionsForOneFracture.end(),
                  std::back_inserter(sharedComplForFracture[i]));

        if (outputStreamForIntermediateResultsText)
        {
#pragma omp critical(critical_section_outputStreamForIntermediateResultsText)
            {
                outputIntermediateResultsText(outputStreamForIntermediateResultsText, fracture, transCondenser, mainGrid, fractureGrid);
            }
        }
    }

    for (const auto& completions : sharedComplForFracture)
    {
        std::copy(completions.begin(), completions.end(), std::back_inserter(fractureCompletions));
    }
    return fractureCompletions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportFractureCompletionsImpl::checkForStimPlanConductivity(const RimFractureTemplate* fracTemplate, const RimFracture* fracture)
{
    auto fracTemplateStimPlan = dynamic_cast<const RimStimPlanFractureTemplate*>(fracTemplate);
    if (fracTemplateStimPlan)
    {
        if (!fracTemplateStimPlan->hasConductivity())
        {
            RiaLogging::error("Trying to export completion data for stimPlan fracture without conductivity data for " +
                fracture->name());
            RiaLogging::error("No transmissibilities will be calculated for " + fracture->name());
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::calculateInternalFractureTransmissibilities(const RigFractureGrid* fractureGrid, double cDarcyInCorrectUnit, RigTransmissibilityCondenser &transCondenser)
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
                    { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellIndex },
                    { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellNeighbourXIndex },
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
                    { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellIndex },
                    { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellNeighbourZIndex },
                    verticalTransToZneigbour);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::calculateFractureToWellTransmissibilities(const RimFractureTemplate* fracTemplate, const RigFractureGrid* fractureGrid, const RimFracture* fracture, double cDarcyInCorrectUnit, const RigWellPath* wellPathGeometry, RigTransmissibilityCondenser &transCondenser)
{
    ////
    // If fracture has orientation Azimuth or Transverse, assume only radial inflow

    if (fracTemplate->orientationType() == RimFractureTemplate::AZIMUTH ||
        fracTemplate->orientationType() == RimFractureTemplate::TRANSVERSE_WELL_PATH)
    {
        std::pair<size_t, size_t> wellCellIJ = fractureGrid->fractureCellAtWellCenter();
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
            { true, RigTransmissibilityCondenser::CellAddress::WELL, 1 },
            { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, wellCellIndex },
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
                { true, RigTransmissibilityCondenser::CellAddress::WELL, 1 },
                { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fracWellCellIdx },
                linearTrans);
        }
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<size_t, double> RicExportFractureCompletionsImpl::calculateMatrixToWellTransmissibilities(RigTransmissibilityCondenser &transCondenser)
{
    std::map<size_t, double> matrixToWellTransmissibilities;

    std::set<RigTransmissibilityCondenser::CellAddress> externalCells = transCondenser.externalCells();
    for (RigTransmissibilityCondenser::CellAddress externalCell : externalCells)
    {
        if (externalCell.m_cellIndexSpace == RigTransmissibilityCondenser::CellAddress::ECLIPSE)
        {
            double trans = transCondenser.condensedTransmissibility(
                externalCell, { true, RigTransmissibilityCondenser::CellAddress::WELL, 1 });

            matrixToWellTransmissibilities.insert(std::make_pair(externalCell.m_globalCellIdx, trans));

        }
    }
    return matrixToWellTransmissibilities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicExportFractureCompletionsImpl::generateCompdatValuesForFracture(const std::map<size_t, double>& matrixToWellTransmissibilites, const QString& wellPathName, const RimEclipseCase* caseToApply, const RimFracture* fracture, const RimFractureTemplate* fracTemplate)
{
    std::vector<RigCompletionData> allCompletionsForOneFracture;
    for (const auto& matrixToWellTransmissibility : matrixToWellTransmissibilites)
    {
        size_t globalCellIndex = matrixToWellTransmissibility.first;
        double trans = matrixToWellTransmissibility.second;
        RigCompletionData compDat(wellPathName,
            RigCompletionDataGridCell(globalCellIndex, caseToApply->mainGrid()),
            fracture->fractureMD());

        double diameter = 2.0 * fracture->wellRadius();
        compDat.setFromFracture(trans, fracTemplate->skinFactor(), diameter);
        compDat.addMetadata(fracture->name(), QString::number(trans));
        allCompletionsForOneFracture.push_back(compDat);
    }
    return allCompletionsForOneFracture;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::computeNonDarcyFlowParameters(const RimFracture* fracture, std::vector<RigCompletionData> allCompletionsForOneFracture)
{
    double dFactorForFracture = fracture->nonDarcyProperties().dFactor;
    double khForFracture = fracture->nonDarcyProperties().conductivity;

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicExportFractureCompletionsImpl::sumUpCellAreas(const std::map<size_t, double>& cellAreas)
{
    double area = 0.0;

    for (const auto& cellArea : cellAreas)
    {
        area += cellArea.second;
    }
    return area;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicExportFractureCompletionsImpl::sumUpTransmissibilities(const std::vector<RigCompletionData>& allCompletionsForOneFracture)
{
    double transmissibility = 0.0;
    for (const auto& c : allCompletionsForOneFracture)
    {
        transmissibility += c.transmissibility();
    }
    return transmissibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::calculateAndSetLengthsAndConductivity(const RimFractureTemplate*     fracTemplate,
    double                         area,
    RicWellPathFractureReportItem& reportItem)
{
    double                          conductivity = 0.0;
    double                          width = 0.0;
    double                          height = 0.0;
    double                          halfLength = 0.0;
    RiaEclipseUnitTools::UnitSystem unitSystem = RiaEclipseUnitTools::UNITS_METRIC;

    {
        auto* ellipseTemplate = dynamic_cast<const RimEllipseFractureTemplate*>(fracTemplate);
        if (ellipseTemplate)
        {
            unitSystem = ellipseTemplate->fractureTemplateUnit();
            conductivity = ellipseTemplate->conductivity();
            width = ellipseTemplate->width();
            height = ellipseTemplate->height();
            halfLength = ellipseTemplate->halfLength();
        }

        auto* stimplanTemplate = dynamic_cast<const RimStimPlanFractureTemplate*>(fracTemplate);
        if (stimplanTemplate)
        {
            unitSystem = stimplanTemplate->fractureTemplateUnit();
            conductivity = stimplanTemplate->areaWeightedConductivity();
            width = stimplanTemplate->areaWeightedWidth();

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
    reportItem.setUnitSystem(unitSystem);
    reportItem.setWidthAndConductivity(width, conductivity);
    reportItem.setHeightAndHalfLength(height, halfLength);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::calculateAndSetAreaWeightedTransmissibility(const RimEclipseCase*          caseToApply,
                                                                                   std::map<size_t, double>       cellAreas,
                                                                                   double                         area,
                                                                                   RicWellPathFractureReportItem& reportItem)
{
    double areaWeightedEclipseTransmissibility = 0.0;

    if (caseToApply && caseToApply->eclipseCaseData())
    {
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::outputIntermediateResultsText(QTextStream*                  outputStreamForIntermediateResultsText,
    const RimFracture*            fracture,
    RigTransmissibilityCondenser& transCondenser,
    const RigMainGrid*            mainGrid,
    const RigFractureGrid*        fractureGrid)
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
