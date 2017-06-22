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

#include "RicExportCompletionDataSettingsUi.h"

#include "RimEclipseCase.h"
#include "RimFracture.h"
#include "RimWellPath.h"
#include "RimFractureTemplate.h"
#include "RimEclipseWell.h"
#include "RimSimWellFractureCollection.h"
#include "RimSimWellFracture.h"

#include "RigEclipseCaseData.h"
#include "RigTransmissibilityCondenser.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigEclipseToStimPlanCellTransmissibilityCalculator.h"
#include "RigFractureTransmissibilityEquations.h"
#include "RigWellPathStimplanIntersector.h"
#include "RigMainGrid.h"
#include "RigSingleWellResultsData.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigWellPath.h"

#include <vector>
#include "RiaLogging.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicExportFractureCompletionsImpl::generateCompdatValuesForWellPath(RimWellPath* wellPath, 
                                                                                                  const RicExportCompletionDataSettingsUi& settings, 
                                                                                                  QTextStream* outputStreamForIntermediateResultsText)
{
    RimEclipseCase* caseToApply = settings.caseToApply();

    std::vector<RimFracture*> fracturesAlongWellPath;
    wellPath->descendantsIncludingThisOfType(fracturesAlongWellPath);

    return generateCompdatValues(caseToApply,
                                 wellPath->name(),
                                 wellPath->wellPathGeometry(),
                                 fracturesAlongWellPath,
                                 outputStreamForIntermediateResultsText);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicExportFractureCompletionsImpl::generateCompdatValuesForSimWell(RimEclipseCase* eclipseCase,
                                                                                                 const RimEclipseWell* well,
                                                                                                 size_t timeStep,
                                                                                                 QTextStream* outputStreamForIntermediateResultsText)
{
    std::vector<RigCompletionData> completionData;

    std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;
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
            if (static_cast<size_t>(fracture->branchIndex()) == branchIndex)
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
                                                                                       const std::vector<RimFracture*> fractures,
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

        bool useFiniteConductivityInFracture = (fracture->fractureTemplate()->conductivityType() == RimFractureTemplate::FINITE_CONDUCTIVITY);

        using CellIdxSpace = RigTransmissibilityCondenser::CellAddress;

        RimFractureTemplate* fracTemplate = fracture->fractureTemplate();
        const RigFractureGrid* fractureGrid = fracTemplate->fractureGrid();

        RigTransmissibilityCondenser transCondenser;

        //////
        // Calculate Matrix To Fracture Trans       

        const std::vector<RigFractureCell>& fractureCells = fractureGrid->fractureCells();
        std::set<size_t> blockedFractureCellIndices;

        for (const RigFractureCell fractureCell : fractureCells)
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

            bool hasAnyContribEclCellWithinContainment = false;
 
            for ( size_t i = 0; i < fractureCellContributingEclipseCells.size(); i++ )
            {
                if ( fracture->isEclipseCellWithinContainment(caseToApply->eclipseCaseData()->mainGrid(), fractureCellContributingEclipseCells[i]) )
                {
                    hasAnyContribEclCellWithinContainment = true;
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

            if (!hasAnyContribEclCellWithinContainment) 
            {
                blockedFractureCellIndices.insert(stimPlanCellIndex);
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
                    
                    if (blockedFractureCellIndices.count(fractureCellIndex)) continue;

                    const RigFractureCell& fractureCell = fractureGrid->cellFromIndex(fractureCellIndex);

                    if (!fractureCell.hasNonZeroConductivity()) continue;

                    if ( i < fractureGrid->iCellCount() - 1 )
                    {
                        size_t fractureCellNeighbourXIndex = fractureGrid->getGlobalIndexFromIJ(i + 1, j);
                        if ( !blockedFractureCellIndices.count(fractureCellNeighbourXIndex) )
                        {

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
                    }

                    if ( j < fractureGrid->jCellCount() - 1 )
                    {
                        size_t fractureCellNeighbourZIndex = fractureGrid->getGlobalIndexFromIJ(i, j + 1);
                        if ( !blockedFractureCellIndices.count(fractureCellNeighbourZIndex) )
                        {
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
                std::pair<size_t, size_t>  wellCellIJ = fracGrid->fractureCellAtWellCenter();
                size_t wellCellIndex = fracGrid->getGlobalIndexFromIJ(wellCellIJ.first, wellCellIJ.second);

                if(!blockedFractureCellIndices.count(wellCellIndex))
                {
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
                else
                {
                    RiaLogging::warning(QString("Fracture well cell is not located within the settings provided by Fracture Containment.") 
                                        + "\n      Fracture named: " + fracture->name());
                }
            }
            else if (fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH)
            {

                ////
                //If fracture has orientation along well, linear inflow along well and radial flow at endpoints

                RigWellPathStimplanIntersector wellFractureIntersector(wellPathGeometry, fracture);
                const std::map<size_t, RigWellPathStimplanIntersector::WellCellIntersection >& fractureWellCells =  wellFractureIntersector.intersections();

                for (const auto& fracCellIdxIsectDataPair : fractureWellCells)
                {
                    size_t fracWellCellIdx = fracCellIdxIsectDataPair.first;

                    if(!blockedFractureCellIndices.count(fracWellCellIdx))
                    {
                        RigWellPathStimplanIntersector::WellCellIntersection intersection = fracCellIdxIsectDataPair.second;

                        const RigFractureCell& fractureWellCell = fractureGrid->cellFromIndex(fracWellCellIdx);

                    double radialTrans = 0.0;
                    if (intersection.endpointCount)
                    {
                        radialTrans = RigFractureTransmissibilityEquations::fractureCellToWellRadialTrans(fractureWellCell.getConductivtyValue(),
                                                                                                          fractureWellCell.cellSizeX(),
                                                                                                          fractureWellCell.cellSizeZ(),
                                                                                                          fracture->wellRadius(caseToApply->eclipseCaseData()->unitsType()),
                                                                                                          fracTemplate->skinFactor(),
                                                                                                          cDarcyInCorrectUnit);
                    }

                        double linearTrans = 0.0;
                        if (intersection.hlength > 0.0 || intersection.vlength > 0.0 )
                        {
                            linearTrans = RigFractureTransmissibilityEquations::fractureCellToWellLinearTrans(fractureWellCell.getConductivtyValue(),
                                                                                                              fractureWellCell.cellSizeX(),
                                                                                                              fractureWellCell.cellSizeZ(),
                                                                                                              intersection.vlength,
                                                                                                              intersection.hlength ,
                                                                                                              fracture->perforationEfficiency, 
                                                                                                              fracTemplate->skinFactor(),
                                                                                                              cDarcyInCorrectUnit);
                        }

                        double totalWellTrans = 0.5 * intersection.endpointCount * radialTrans + linearTrans;

                        transCondenser.addNeighborTransmissibility( { true, RigTransmissibilityCondenser::CellAddress::WELL, 1 },
                                                                    { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fracWellCellIdx },
                                                                     totalWellTrans);
                    }
                    else
                    {
                        RiaLogging::warning(QString("Fracture well cell is not located within the settings provided by Fracture Containment.") 
                                            + "\n      Fracture named: " + fracture->name());
                    }
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
                size_t i, j, k;
                mainGrid->ijkFromCellIndex(externalCell.m_globalCellIdx, &i, &j, &k);

                RigCompletionData compDat(wellPathName, {i,j,k} );
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


