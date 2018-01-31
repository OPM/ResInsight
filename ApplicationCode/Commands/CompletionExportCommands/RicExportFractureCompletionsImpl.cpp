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

#include "RiaLogging.h"

#include "RicExportCompletionDataSettingsUi.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimSimWellFracture.h"
#include "RimSimWellFractureCollection.h"
#include "RimSimWellInView.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "RigEclipseCaseData.h"
#include "RigTransmissibilityCondenser.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigEclipseToStimPlanCellTransmissibilityCalculator.h"
#include "RigFractureTransmissibilityEquations.h"
#include "RigWellPathStimplanIntersector.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigWellPath.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicExportFractureCompletionsImpl::generateCompdatValuesForWellPath(RimWellPath* wellPath, 
                                                                                                  const RicExportCompletionDataSettingsUi& settings, 
                                                                                                  QTextStream* outputStreamForIntermediateResultsText)
{
    RimEclipseCase* caseToApply = settings.caseToApply();

    std::vector<RimFracture*> fracturesAlongWellPath;

    if (wellPath->fractureCollection()->isChecked())
    {
        for (const auto& frac : wellPath->fractureCollection()->fractures)
        {
            if (frac->isChecked())
            {
                fracturesAlongWellPath.push_back(frac);
            }
        }
    }

    return generateCompdatValues(caseToApply,
                                 wellPath->completions()->wellNameForExport(),
                                 wellPath->wellPathGeometry(),
                                 fracturesAlongWellPath,
                                 outputStreamForIntermediateResultsText);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicExportFractureCompletionsImpl::generateCompdatValuesForSimWell(RimEclipseCase* eclipseCase,
                                                                                                 const RimSimWellInView* well,
                                                                                                 QTextStream* outputStreamForIntermediateResultsText)
{
    std::vector<RigCompletionData> completionData;

    std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;
    
    RimEclipseView* view = nullptr;
    well->firstAncestorOrThisOfTypeAsserted(view);
    
    size_t timeStep = view->currentTimeStep();

    well->calculateWellPipeDynamicCenterLine(timeStep, pipeBranchesCLCoords, pipeBranchesCellIds);

    for (size_t branchIndex = 0; branchIndex < pipeBranchesCLCoords.size(); ++branchIndex)
    {
        RigSimulationWellCoordsAndMD coordsAndMD(pipeBranchesCLCoords[branchIndex]);
        RigWellPath wellPathGeometry;
        wellPathGeometry.m_wellPathPoints = coordsAndMD.wellPathPoints();
        wellPathGeometry.m_measuredDepths = coordsAndMD.measuredDepths();

        std::vector<RimFracture*> fractures;
        for (RimSimWellFracture* fracture : well->simwellFractureCollection->simwellFractures())
        {
            if (fracture->isChecked() && static_cast<size_t>(fracture->branchIndex()) == branchIndex)
            {
                fractures.push_back(fracture);
            }
        }

        std::vector<RigCompletionData> branchCompletions = generateCompdatValues(eclipseCase, well->name(), &wellPathGeometry, fractures, outputStreamForIntermediateResultsText);

        completionData.insert(completionData.end(), branchCompletions.begin(), branchCompletions.end());
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicExportFractureCompletionsImpl::generateCompdatValues(RimEclipseCase* caseToApply,
                                                                                       const QString& wellPathName,
                                                                                       const RigWellPath* wellPathGeometry,
                                                                                       const std::vector<RimFracture*>& fractures,
                                                                                       QTextStream* outputStreamForIntermediateResultsText)
{
    double cDarcyInCorrectUnit = RiaEclipseUnitTools::darcysConstant(caseToApply->eclipseCaseData()->unitsType());
    const RigMainGrid* mainGrid = caseToApply->eclipseCaseData()->mainGrid();

    // To handle several fractures in the same eclipse cell we need to keep track of the transmissibility 
    // to the well from each fracture intersecting the cell and sum these transmissibilities at the end.
    // std::map <eclipseCellIndex ,map< fracture, trans> > 
    std::map <size_t, std::map<RimFracture*, double> > eclCellIdxToTransPrFractureMap; 
    std::vector<RigCompletionData> fractureCompletions; 

    for (RimFracture* fracture : fractures)
    {
        RimFractureTemplate* fracTemplate = fracture->fractureTemplate();

        if (!fracTemplate) continue;

        const RigFractureGrid* fractureGrid = fracTemplate->fractureGrid();
        if (!fractureGrid) continue;

        bool useFiniteConductivityInFracture = (fracTemplate->conductivityType() == RimFractureTemplate::FINITE_CONDUCTIVITY);

        //If finite cond chosen and conductivity not present in stimplan file, do not calculate trans for this fracture
        if (useFiniteConductivityInFracture)
        {
            if (dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate))
            {
                RimStimPlanFractureTemplate* fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate);
                if (!fracTemplateStimPlan->hasConductivity())
                {
                    RiaLogging::error("Trying to export completion data for stimPlan fracture without conductivity data for " + fracture->name());
                    RiaLogging::error("No transmissibilities will be calculated for " + fracture->name());

                    continue;
                }
            }
        }


        using CellIdxSpace = RigTransmissibilityCondenser::CellAddress;
        RigTransmissibilityCondenser transCondenser;

        //////
        // Calculate Matrix To Fracture Trans       

        const std::vector<RigFractureCell>& fractureCells = fractureGrid->fractureCells();

        for (const RigFractureCell& fractureCell : fractureCells)
        {
            if (!fractureCell.hasNonZeroConductivity()) continue;

            RigEclipseToStimPlanCellTransmissibilityCalculator eclToFractureTransCalc(caseToApply,
                                                                                      fracture->transformMatrix(),
                                                                                      fracture->fractureTemplate()->skinFactor,
                                                                                      cDarcyInCorrectUnit,
                                                                                      fractureCell);

            const std::vector<size_t>& fractureCellContributingEclipseCells                  = eclToFractureTransCalc.globalIndeciesToContributingEclipseCells();
            const std::vector<double>& fractureCellContributingEclipseCellTransmissibilities = eclToFractureTransCalc.contributingEclipseCellTransmissibilities();
            
            size_t stimPlanCellIndex = fractureGrid->getGlobalIndexFromIJ(fractureCell.getI(), fractureCell.getJ());

            for ( size_t i = 0; i < fractureCellContributingEclipseCells.size(); i++ )
            {
                if ( fracture->isEclipseCellWithinContainment(caseToApply->eclipseCaseData()->mainGrid(), fractureCellContributingEclipseCells[i]) )
                {
                    if ( useFiniteConductivityInFracture )
                    {
                        transCondenser.addNeighborTransmissibility({ true,  CellIdxSpace::ECLIPSE, fractureCellContributingEclipseCells[i] },
                        { false, CellIdxSpace::STIMPLAN, stimPlanCellIndex },
                                                                   fractureCellContributingEclipseCellTransmissibilities[i]);

                    }
                    else
                    {
                        transCondenser.addNeighborTransmissibility({ true, CellIdxSpace::ECLIPSE, fractureCellContributingEclipseCells[i] },
                        { true, CellIdxSpace::WELL, 1 },
                                                                   fractureCellContributingEclipseCellTransmissibilities[i]);
                    }
                }
            }

        }

        //////
        // Calculate Transmissibility in the fracture: From one StimPlan Cell to the other

        if (useFiniteConductivityInFracture)
        {
            for (size_t i = 0; i < fractureGrid->iCellCount(); i++)
            {
                for (size_t j = 0; j < fractureGrid->jCellCount();  j++)
                {
                    size_t fractureCellIndex = fractureGrid->getGlobalIndexFromIJ(i, j);
                    
                    const RigFractureCell& fractureCell = fractureGrid->cellFromIndex(fractureCellIndex);

                    if (!fractureCell.hasNonZeroConductivity()) continue;

                    if ( i < fractureGrid->iCellCount() - 1 )
                    {
                        size_t fractureCellNeighbourXIndex = fractureGrid->getGlobalIndexFromIJ(i + 1, j);
                        const RigFractureCell& fractureCellNeighbourX = fractureGrid->cellFromIndex(fractureCellNeighbourXIndex);

                        double horizontalTransToXneigbour =
                            RigFractureTransmissibilityEquations::centerToCenterFractureCellTrans(fractureCell.getConductivtyValue(),
                                                                                                    fractureCell.cellSizeX(),
                                                                                                    fractureCell.cellSizeZ(),
                                                                                                    fractureCellNeighbourX.getConductivtyValue(),
                                                                                                    fractureCellNeighbourX.cellSizeX(),
                                                                                                    fractureCellNeighbourX.cellSizeZ(),
                                                                                                    cDarcyInCorrectUnit);

                        transCondenser.addNeighborTransmissibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellIndex },
                                                                    { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellNeighbourXIndex },
                                                                    horizontalTransToXneigbour);
                    }

                    if ( j < fractureGrid->jCellCount() - 1 )
                    {
                        size_t fractureCellNeighbourZIndex = fractureGrid->getGlobalIndexFromIJ(i, j + 1);
                        const RigFractureCell& fractureCellNeighbourZ = fractureGrid->cellFromIndex(fractureCellNeighbourZIndex);

                        double verticalTransToZneigbour = 
                            RigFractureTransmissibilityEquations::centerToCenterFractureCellTrans(fractureCell.getConductivtyValue(),
                                                                                                    fractureCell.cellSizeZ(),
                                                                                                    fractureCell.cellSizeX(),
                                                                                                    fractureCellNeighbourZ.getConductivtyValue(),
                                                                                                    fractureCellNeighbourZ.cellSizeZ(),
                                                                                                    fractureCellNeighbourZ.cellSizeX(),
                                                                                                    cDarcyInCorrectUnit);

                        transCondenser.addNeighborTransmissibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellIndex },
                                                                    { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellNeighbourZIndex },
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
            //If fracture has orientation Azimuth or Transverse, assume only radial inflow

            if (   fracture->fractureTemplate()->orientationType() == RimFractureTemplate::AZIMUTH
                || fracture->fractureTemplate()->orientationType() == RimFractureTemplate::TRANSVERSE_WELL_PATH)
            {
                const RigFractureGrid* fracGrid = fracture->fractureTemplate()->fractureGrid();
                if (fracGrid)
                {
                    std::pair<size_t, size_t>  wellCellIJ = fracGrid->fractureCellAtWellCenter();
                    size_t wellCellIndex = fracGrid->getGlobalIndexFromIJ(wellCellIJ.first, wellCellIJ.second);

                    const RigFractureCell& wellCell = fractureGrid->cellFromIndex(wellCellIndex);

                    double radialTrans = RigFractureTransmissibilityEquations::fractureCellToWellRadialTrans(wellCell.getConductivtyValue(),
                                                                                                             wellCell.cellSizeX(),
                                                                                                             wellCell.cellSizeZ(),
                                                                                                             fracture->wellRadius(caseToApply->eclipseCaseData()->unitsType()),
                                                                                                             fracTemplate->skinFactor(),
                                                                                                             cDarcyInCorrectUnit);

                        transCondenser.addNeighborTransmissibility({ true, RigTransmissibilityCondenser::CellAddress::WELL, 1 },
                                                                   { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, wellCellIndex },
                                                                   radialTrans);
                }
            }
            else if (fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH)
            {

                ////
                //If fracture has orientation along well, linear inflow along well and radial flow at endpoints

                RigWellPathStimplanIntersector wellFractureIntersector(wellPathGeometry, fracture);
                const std::map<size_t, RigWellPathStimplanIntersector::WellCellIntersection >& fractureWellCells = wellFractureIntersector.intersections();

                for (const auto& fracCellIdxIsectDataPair : fractureWellCells)
                {
                    size_t fracWellCellIdx = fracCellIdxIsectDataPair.first;

                    RigWellPathStimplanIntersector::WellCellIntersection intersection = fracCellIdxIsectDataPair.second;

                    const RigFractureCell& fractureWellCell = fractureGrid->cellFromIndex(fracWellCellIdx);

                    double linearTrans = 0.0;
                    if (intersection.hlength > 0.0 || intersection.vlength > 0.0)
                    {
                        linearTrans = RigFractureTransmissibilityEquations::fractureCellToWellLinearTrans(fractureWellCell.getConductivtyValue(),
                                                                                                            fractureWellCell.cellSizeX(),
                                                                                                            fractureWellCell.cellSizeZ(),
                                                                                                            intersection.vlength,
                                                                                                            intersection.hlength,
                                                                                                            fracture->perforationEfficiency(),
                                                                                                            fracTemplate->skinFactor(),
                                                                                                            cDarcyInCorrectUnit);
                    }

                    transCondenser.addNeighborTransmissibility({ true, RigTransmissibilityCondenser::CellAddress::WELL, 1 },
                                                                { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fracWellCellIdx },
                                                                linearTrans);
                }
            }
        }

        /////
        // Insert total transmissibility from eclipse-cell to well for this fracture into the map 

        std::set<RigTransmissibilityCondenser::CellAddress> externalCells = transCondenser.externalCells();
        for (RigTransmissibilityCondenser::CellAddress externalCell : externalCells)
        {
            if (externalCell.m_cellIndexSpace == RigTransmissibilityCondenser::CellAddress::ECLIPSE)
            {
                double trans = transCondenser.condensedTransmissibility(externalCell, { true, RigTransmissibilityCondenser::CellAddress::WELL, 1 });
                    
                eclCellIdxToTransPrFractureMap[externalCell.m_globalCellIdx][fracture] = trans;

                RigCompletionData compDat(wellPathName, RigCompletionDataGridCell(externalCell.m_globalCellIdx, caseToApply->mainGrid()));
                compDat.setFromFracture(trans, fracture->fractureTemplate()->skinFactor());
                compDat.addMetadata(fracture->name(), QString::number(trans));
                fractureCompletions.push_back(compDat);
            }
        }

        if ( outputStreamForIntermediateResultsText )
        {
            (*outputStreamForIntermediateResultsText) << "\n" << "\n" << "\n----------- All Transimissibilities " << fracture->name() << " -------------------- \n\n";
            (*outputStreamForIntermediateResultsText) << QString::fromStdString(transCondenser.neighborTransDebugOutput(mainGrid, fractureGrid));
            (*outputStreamForIntermediateResultsText) << "\n" << "\n" << "\n----------- Condensed Results " << fracture->name() << " -------------------- \n\n";
            (*outputStreamForIntermediateResultsText) << QString::fromStdString(transCondenser.condensedTransDebugOutput(mainGrid, fractureGrid));
            (*outputStreamForIntermediateResultsText) << "\n" ;
        }
    } 

    return fractureCompletions;
}

