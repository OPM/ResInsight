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

#include "RifFractureExportTools.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RigEclipseCaseData.h"
#include "RigFracture.h"
#include "RigFractureTransCalc.h"
#include "RigStimPlanFracTemplateCell.h"
#include "RigMainGrid.h"
#include "RigEclipseToStimPlanCellTransmissibilityCalculator.h"
#include "RigFractureTransmissibilityEquations.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimSimWellFracture.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimWellPath.h"

#include "cafProgressInfo.h"

#include <QFile>
#include <QString>
#include <QTextStream>
#include "RigStimPlanUpscalingCalc.h"
#include "RigTransmissibilityCondenser.h"



//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifFractureExportTools::RifFractureExportTools()
{

}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifFractureExportTools::~RifFractureExportTools()
{
  
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifFractureExportTools::exportFracturesToEclipseDataInputFile(const QString& fileName,  const std::vector< RimFracture*>& fractures, RimEclipseCase* caseToApply)
{
    RiaLogging::info(QString("Computing and writing COMPDAT values to file %1").arg(fileName));

    const RigMainGrid* mainGrid = caseToApply->eclipseCaseData()->mainGrid();
    if (!mainGrid) return false;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    caf::ProgressInfo pi(fractures.size(), QString("Writing data to file %1").arg(fileName));

    size_t progress = 0;
    std::vector<size_t> ijk;

    QTextStream out(&file);
    out << "\n";
    out << "-- Exported from ResInsight" << "\n";

    QString wellName; 
    {
        RimEclipseWell* simWell = nullptr;
        fractures[0]->firstAncestorOrThisOfType(simWell);
        if ( simWell ) wellName = simWell->name;    

        RimWellPath* wellPath = nullptr;
        fractures[0]->firstAncestorOrThisOfType(wellPath);
        if ( wellPath ) wellName = wellPath->name;  
    }

    RigEclipseCaseData::UnitsType caseUnit = caseToApply->eclipseCaseData()->unitsType();
    if (caseUnit == RigEclipseCaseData::UNITS_METRIC) out << "-- Using metric unit system" << "\n";
    if (caseUnit == RigEclipseCaseData::UNITS_FIELD) out << "-- Using field unit system" << "\n";
    out << "\n";

    //Included for debug / prototyping only
    printTransmissibilityFractureToWell(fractures, out, caseToApply);
    
    printStimPlanFractureTrans(fractures, caseToApply, out);
    
    printStimPlanCellsMatrixTransContributions(fractures, caseToApply, out, wellName, mainGrid);
    
    printBackgroundDataHeaderLine(out);
    
    RiaLogging::debug(QString("Writing intermediate results from COMPDAT calculation"));

    std::map<RimFracture*, std::vector<RigFracturedEclipseCellExportData> > exportDataPrFracture;

    for (RimFracture* fracture : fractures)
    {
        RigFractureTransCalc transmissibilityCalculator(caseToApply, fracture);

        //TODO: Check that there is a fracture template available for given fracture....

        std::vector<RigFracturedEclipseCellExportData> fracDataVector = transmissibilityCalculator.computeTransmissibilityFromPolygonWithInfiniteConductivityInFracture();
        exportDataPrFracture[fracture] = fracDataVector;

        for (RigFracturedEclipseCellExportData fracData : fracDataVector)
        {
            printBackgroundData(out, wellName, fracture, mainGrid, fracData);
        }
    }

    out << "\n";

    out << qSetFieldWidth(7) << "COMPDAT" << "\n" << right << qSetFieldWidth(8);
    for (RimFracture* fracture : fractures)
    {
        RiaLogging::debug(QString("Writing COMPDAT values for fracture %1").arg(fracture->name()));
        std::vector<RigFracturedEclipseCellExportData> fracDataVector = exportDataPrFracture[fracture];

        double skinFactor = cvf::UNDEFINED_DOUBLE;
        if (fracture->attachedFractureDefinition()) skinFactor = fracture->attachedFractureDefinition()->skinFactor();
        QString fractureName = fracture->name();

        for (RigFracturedEclipseCellExportData fracData : fracDataVector)
        {
            if ( fracData.transmissibility > 0 )
            {
                size_t i, j, k;
                mainGrid->ijkFromCellIndex(fracData.reservoirCellIndex, &i, &j, &k);
                printCOMPDATvalues(out, fracData.transmissibility, i, j, k, fractureName, skinFactor,  wellName);
            }
        }
        
        //TODO: If same cell is used for multiple fractures, the sum of contributions should be added to table. 

        progress++;
        pi.setProgress(progress);
    }

    out << "/ \n";

    RiaLogging::info(QString("Competed writing COMPDAT data to file %1").arg(fileName));
    return true;
}

//--------------------------------------------------------------------------------------------------
///
// Loop over fractures in well path (all)
//   Loop over stimplancells
//     if cell Is Contributing cond > 0
//        Calculate Matrix To Fracture Trans       
//        Add to condenser
//   Loop over stimplanecells (i, j)
//      if cell Is Contributing cond > 0
//         Calculate trans to neighbor (+i, +j ) 
//         Add to condenser
//   Find well cells (Perforated Well Path, Radius, Positioned Fracture)
//   For each well cell 
//       Find path intersection
//       Use LinT + 1/2 radialT on each end if endpoints are inside cell 
//       Add to condenser
//   Add eclipse cell transmissibilities to a map <eclipsecell index, map<fractureptr, transmissibility> >
// For all transmissibilities 
//  summarize all fractures contributions pr cell, 
//  Print COMPDAT entry
//
//--------------------------------------------------------------------------------------------------
void RifFractureExportTools::exportWellPathFracturesToEclipseDataInputFile(const QString& fileName, 
                                                                           const RimWellPath* wellPath, 
                                                                           const RimEclipseCase* caseToApply)
{

    double cDarcyInCorrectUnit = caseToApply->eclipseCaseData()->darchysValue();
    std::map <size_t, std::map<RimFracture*, double>> transmissibilitiesMap; //eclipseCellIndex (size_t), fracture (RimFracture) and trans value

    //Getting fractures for well path
    std::vector<RimFracture*> fractures;
    wellPath->descendantsIncludingThisOfType(fractures);


    // Loop over fractures in well path (all)
    for (RimFracture* fracture : fractures)
    {

        RimStimPlanFractureTemplate* fracTemplateStimPlan;
        if (dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition()))
        {
            fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition());
        }
        else continue;

        RigTransmissibilityCondenser condenser;
        
         //   Loop over stimplancells
        for (const RigStimPlanFracTemplateCell stimPlanCell : fracTemplateStimPlan->getStimPlanCells())
        {
            //     if cell Is Contributing cond > 0
            if (stimPlanCell.getConductivtyValue() < 1e-7) continue;
            
            size_t stimPlanCellIndex = fracTemplateStimPlan->getGlobalIndexFromIJ(stimPlanCell.getI(), stimPlanCell.getJ());

            //        Calculate Matrix To Fracture Trans       
            RigEclipseToStimPlanCellTransmissibilityCalculator eclToStimPlanTransCalc(caseToApply,
                fracture->transformMatrix(),
                fracture->attachedFractureDefinition()->skinFactor,
                cDarcyInCorrectUnit,
                stimPlanCell);

            std::vector<size_t> stimPlanContributingEclipseCells = eclToStimPlanTransCalc.globalIndeciesToContributingEclipseCells();
            std::vector<double> stimPlanContributingEclipseCellTransmissibilities = eclToStimPlanTransCalc.contributingEclipseCellTransmissibilities();

            //        Add to condenser
            for (int i = 0; i < stimPlanContributingEclipseCells.size(); i++)
            {
                condenser.addNeighborTransmissibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, stimPlanContributingEclipseCells[i]},
                                                      { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, stimPlanCellIndex},
                                                      stimPlanContributingEclipseCellTransmissibilities[i]);
            }

        }


        //   Loop over stimplanecells (i, j)
        for (int i = 0; i < fracTemplateStimPlan->stimPlanGridNumberOfColums(); i++)
        {
            for (int j = 0; j < fracTemplateStimPlan->stimPlanGridNumberOfRows();  j++)
            {
                //TODO: Check that cond > 0
                size_t stimPlanCellIndex = fracTemplateStimPlan->getGlobalIndexFromIJ(i, j);
                const RigStimPlanFracTemplateCell stimPlanCell = fracTemplateStimPlan->stimPlanCellFromIndex(stimPlanCellIndex);

                if (stimPlanCell.getConductivtyValue() < 1e-7)
                {
                    continue;
                }

                if (i < fracTemplateStimPlan->stimPlanGridNumberOfColums() - 1)
                {
                    size_t stimPlanCellNeighbourXIndex = fracTemplateStimPlan->getGlobalIndexFromIJ(i + 1, j);
                    const RigStimPlanFracTemplateCell stimPlanCellNeighbourX = fracTemplateStimPlan->stimPlanCellFromIndex(stimPlanCellNeighbourXIndex);

                    double horizontalTransToXneigbour =
                        RigFractureTransmissibilityEquations::computeStimPlanCellTransmissibilityInFractureCenterToCenterForTwoCells(stimPlanCell.getConductivtyValue(),
                                                                                                                                     stimPlanCell.cellSizeX(),
                                                                                                                                     stimPlanCell.cellSizeZ(),
                                                                                                                                     stimPlanCellNeighbourX.getConductivtyValue(),
                                                                                                                                     stimPlanCellNeighbourX.cellSizeX(),
                                                                                                                                     stimPlanCellNeighbourX.cellSizeZ(),
                                                                                                                                     cDarcyInCorrectUnit);

                    condenser.addNeighborTransmissibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, stimPlanCellIndex },
                    { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, stimPlanCellNeighbourXIndex },
                        horizontalTransToXneigbour);

                }

                if (j < fracTemplateStimPlan->stimPlanGridNumberOfRows() - 1)
                {
                    size_t stimPlanCellNeighbourZIndex = fracTemplateStimPlan->getGlobalIndexFromIJ(i, j + 1);
                    const RigStimPlanFracTemplateCell stimPlanCellNeighbourZ = fracTemplateStimPlan->stimPlanCellFromIndex(stimPlanCellNeighbourZIndex);
                  
                    double verticalTransToZneigbour = 
                        RigFractureTransmissibilityEquations::computeStimPlanCellTransmissibilityInFractureCenterToCenterForTwoCells(stimPlanCell.getConductivtyValue(),
                                                                                                                                     stimPlanCell.cellSizeZ(),
                                                                                                                                     stimPlanCell.cellSizeX(),
                                                                                                                                     stimPlanCellNeighbourZ.getConductivtyValue(),
                                                                                                                                     stimPlanCellNeighbourZ.cellSizeZ(),
                                                                                                                                     stimPlanCellNeighbourZ.cellSizeX(),
                                                                                                                                     cDarcyInCorrectUnit);

                    condenser.addNeighborTransmissibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, stimPlanCellIndex },
                                                          { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, stimPlanCellNeighbourZIndex },
                                                          verticalTransToZneigbour);

                }
            }
        } 

        //   Find well cells (Perforated Well Path, Radius, Positioned Fracture)
        //   For each well cell 
        //       Find path intersection
        //       Use LinT + 1/2 radialT on each end if endpoints are inside cell 
        //       Add to condenser

        //Adding well cell to condenser
        std::pair<size_t, size_t> centerCellIJ = fracTemplateStimPlan->getStimPlanCellAtWellCenter();
        size_t wellCellIndex = fracTemplateStimPlan->getGlobalIndexFromIJ(centerCellIJ.first, centerCellIJ.second);
        const RigStimPlanFracTemplateCell stimPlanWellCell = fracTemplateStimPlan->stimPlanCellFromIndex(wellCellIndex);

        double wellTrans = RigFractureTransmissibilityEquations::computeRadialTransmissibilityToWellinStimPlanCell(stimPlanWellCell.getConductivtyValue(),
                                                                                                                   stimPlanWellCell.cellSizeX(),
                                                                                                                   stimPlanWellCell.cellSizeZ(),
                                                                                                                   fracture->wellRadius(),
                                                                                                                   fracTemplateStimPlan->skinFactor(),
                                                                                                                   cDarcyInCorrectUnit);

        condenser.addNeighborTransmissibility({true, RigTransmissibilityCondenser::CellAddress::WELL, 1},
                                              {false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, wellCellIndex},
                                              wellTrans);

        //    std::map <size_t, std::map<RimFracture*, double>> transmissibilitiesMap; //eclipseCellIndex (size_t), fracture (RimFracture) and trans value

        //   Add eclipse cell transmissibilities to a map <eclipsecell index, map<fractureptr, transmissibility> >

        std::set<RigTransmissibilityCondenser::CellAddress> externalCells = condenser.externalCells();

        for (RigTransmissibilityCondenser::CellAddress externalCell : externalCells)
        {
            if (externalCell.m_cellIndexSpace == RigTransmissibilityCondenser::CellAddress::ECLIPSE)
            {
                double trans = condenser.condensedTransmissibility(externalCell, { true, RigTransmissibilityCondenser::CellAddress::WELL, 1 });
                
                transmissibilitiesMap[externalCell.m_globalCellIdx][fracture] = trans;
            }
        }
    } //end loop fractures



      // For all transmissibilities 
      //  summarize all fractures contributions pr cell, 
     //  Print COMPDAT entry

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifFractureExportTools::printCOMPDATvalues(QTextStream & out,
    double transmissibility,
    size_t i, size_t j, size_t k,
    const QString& fractureName,
    double skinFactor,
    const QString& wellName)
{
    out << qSetFieldWidth(8);

    if (transmissibility == cvf::UNDEFINED_DOUBLE || skinFactor == cvf::UNDEFINED_DOUBLE)
    {
        out << "--"; //Commenting out line in output file
    }

    out << wellName;
    out << qSetFieldWidth(5);


    out << i + 1;          // 2. I location grid block, adding 1 to go to eclipse 1-based grid definition
    out << j + 1;          // 3. J location grid block, adding 1 to go to eclipse 1-based grid definition
    out << k + 1;          // 4. K location of upper connecting grid block, adding 1 to go to eclipse 1-based grid definition
    out << k + 1;          // 5. K location of lower connecting grid block, adding 1 to go to eclipse 1-based grid definition

    out << "2* ";         // Default value for 
                          //6. Open / Shut flag of connection
                          // 7. Saturation table number for connection rel perm. Default value

    out << qSetFieldWidth(12);

    // 8. Transmissibility 
    if (transmissibility != cvf::UNDEFINED_DOUBLE)
    {
        out << QString::number(transmissibility, 'e', 4);
    }
    else
    {
        out << "UNDEF";
    }

    out << qSetFieldWidth(4);
    out << "2* ";         // Default value for 
                          // 9. Well bore diameter. Set to default
                          // 10. Effective Kh (perm times width)


    if (skinFactor != cvf::UNDEFINED_DOUBLE)
    {
        out << skinFactor;    // 11. Skin factor
    }
    else //If no attached fracture definition these parameters are set to UNDEF
    {
        out << "UNDEF";
    }

    out << "/";
    out << " " << fractureName; //Fracture name as comment
    out << "\n"; // Terminating entry

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifFractureExportTools::printStimPlanCellsMatrixTransContributions(const std::vector<RimFracture *>& fractures, 
                                                                        RimEclipseCase* caseToApply, 
                                                                        QTextStream &out, 
                                                                        const QString& wellName, 
                                                                        const RigMainGrid* mainGrid)
{
    out << "StimPlan cells' matrix transmissibility and Eclipse Cell contributions \n";

    out << qSetFieldWidth(4);
    out << "-- ";

    out << qSetFieldWidth(12);
    out << "Well name ";    // 1. Well name 

    out << qSetFieldWidth(16);
    out << "Fracture name ";

    out << qSetFieldWidth(5);
    out << "Ec i";
    out << "Ec j";
    out << "Ec k";

    out << qSetFieldWidth(10);
    out << "Ecl cell";

    out << qSetFieldWidth(5);
    out << "SP i";
    out << "SP j";

    out << qSetFieldWidth(10);
    out << "Tm contr";

    out << "\n";



    for (RimFracture* fracture : fractures)
    {
        RimStimPlanFractureTemplate* fracTemplateStimPlan;
        if (dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition()))
        {
            fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition());
        }
        else continue;

        double cDarcyInCorrectUnit = caseToApply->eclipseCaseData()->darchysValue();

        std::vector<RigStimPlanFracTemplateCell> stimPlanCells = fracTemplateStimPlan->getStimPlanCells();

        for (RigStimPlanFracTemplateCell stimPlanCell : stimPlanCells)
        {
            if (stimPlanCell.getConductivtyValue() < 1e-7)
            {
                continue;
            }

            RigEclipseToStimPlanCellTransmissibilityCalculator eclToStimPlanTransCalc(caseToApply,
                fracture->transformMatrix(),
                fracture->attachedFractureDefinition()->skinFactor,
                cDarcyInCorrectUnit,
                stimPlanCell);

            std::vector<size_t> stimPlanContributingEclipseCells = eclToStimPlanTransCalc.globalIndeciesToContributingEclipseCells();
            std::vector<double> stimPlanContributingEclipseCellTransmissibilities = eclToStimPlanTransCalc.contributingEclipseCellTransmissibilities();

            for (int i = 0; i < stimPlanContributingEclipseCells.size(); i++)
            {
                out << qSetFieldWidth(4);
                out << "-- ";

                out << qSetFieldWidth(12);
                out << wellName + " ";
                out << qSetFieldWidth(16);
                out << fracture->name().left(15) + " ";

                out << qSetFieldWidth(5);
                size_t ii, jj, kk;
                mainGrid->ijkFromCellIndex(stimPlanContributingEclipseCells[i], &ii, &jj, &kk);
                out << ii + 1;         
                out << jj + 1;         
                out << kk + 1;          

                out << qSetFieldWidth(10);
                out << stimPlanContributingEclipseCells[i];

                out << qSetFieldWidth(5);
                size_t spi = stimPlanCell.getI();
                size_t spj = stimPlanCell.getJ();

                out << spi;          
                out << spj;         

                out << qSetFieldWidth(10);
                out << QString::number(stimPlanContributingEclipseCellTransmissibilities[i], 'e', 3);

                out << "\n";
            }

            //TODO: add RigFractureStimPlanCellData to m_StimPlanCellsFractureData i RigFracture???
        }
    }
    return;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifFractureExportTools::printStimPlanFractureTrans(const std::vector<RimFracture *>& fractures, RimEclipseCase* caseToApply, QTextStream &out)
{

    double cDarcyInCorrectUnit = caseToApply->eclipseCaseData()->darchysValue();

    out << "StimPlan cells' fracture transmissibility \n";

    out << qSetFieldWidth(4);
    out << "-- ";

    out << qSetFieldWidth(5);
    out << "SP i";
    out << "SP j";

    out << qSetFieldWidth(10);
    out << "Tf_hor";
    out << "Tf_vert";

    out << "\n";

    if (fractures.size() < 1) return;
    RimFracture* fracture = fractures[0];

    RimStimPlanFractureTemplate* fracTemplateStimPlan;
    if (dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition()))
    {
        fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition());
    }
    else return;

    std::vector<RigStimPlanFracTemplateCell> stimPlanCells = fracTemplateStimPlan->getStimPlanCells();

    for (RigStimPlanFracTemplateCell stimPlanCell : stimPlanCells)
    {
        if (stimPlanCell.getConductivtyValue() < 1e-7)
        {
            //If conductivity in stimPlanCell is 0, contributions might not be relevant...
            continue;
        }

        double verticalTrans = RigFractureTransmissibilityEquations::computeStimPlanCellTransmissibilityInFracture(stimPlanCell.getConductivtyValue(), 
                                                                                                                   stimPlanCell.cellSizeX(), 
                                                                                                                   stimPlanCell.cellSizeZ(), 
                                                                                                                   cDarcyInCorrectUnit);
        double horizontalTrans = RigFractureTransmissibilityEquations::computeStimPlanCellTransmissibilityInFracture(stimPlanCell.getConductivtyValue(), 
                                                                                                                     stimPlanCell.cellSizeZ(), 
                                                                                                                     stimPlanCell.cellSizeX(), 
                                                                                                                     cDarcyInCorrectUnit);

        out << qSetFieldWidth(5);
        size_t spi = stimPlanCell.getI();
        size_t spj = stimPlanCell.getJ();

        out << spi;
        out << spj;

        out << qSetFieldWidth(10);
        out << QString::number(verticalTrans, 'e', 3);
        out << QString::number(horizontalTrans, 'e', 3);

        out << "\n";
    }

return;

}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifFractureExportTools::printBackgroundDataHeaderLine(QTextStream & out)
{
    out << "-- Background data for calculation" << "\n\n";


    //Write header line
    out << qSetFieldWidth(4);
    out << "--";
    out << qSetFieldWidth(12);
    out << "Well ";

    out << qSetFieldWidth(16);
    out << "Fracture ";

    out << qSetFieldWidth(5);
    out << "i";
    out << "j";
    out << "k";

    out << qSetFieldWidth(12);
    out << "Ax";
    out << "Ay";
    out << "Az";
    out << "TotArea";

    out << "skinfac";
    out << "FracLen";

    out << qSetFieldWidth(10);
    out << "DX";
    out << "DY";
    out << "DZ";

    out << qSetFieldWidth(12);
    out << "PermX";
    out << "PermY";
    out << "PermZ";

    out << qSetFieldWidth(8);
    out << "NTG";

    out << qSetFieldWidth(12);
    out << "T_x";
    out << "T_y";
    out << "T_z";

    out << qSetFieldWidth(15);
    out << "Transm";

    out << qSetFieldWidth(20);
    out << "Status";

    out << "\n";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifFractureExportTools::printBackgroundData(QTextStream & out, const QString& wellName, RimFracture* fracture, const RigMainGrid* mainGrid, RigFracturedEclipseCellExportData &fracData)
{
    out << qSetFieldWidth(4);
    out << "-- ";

    out << qSetFieldWidth(12);

    out << wellName + " ";
    out << qSetFieldWidth(16);
    out << fracture->name().left(15) + " ";


    out << qSetFieldWidth(5);
    size_t i, j, k;
    mainGrid->ijkFromCellIndex(fracData.reservoirCellIndex, &i, &j, &k);
    out << i + 1;          // 2. I location grid block, adding 1 to go to eclipse 1-based grid definition
    out << j + 1;          // 3. J location grid block, adding 1 to go to eclipse 1-based grid definition
    out << k + 1;          // 4. K location of upper connecting grid block, adding 1 to go to eclipse 1-based grid definition

    out << qSetFieldWidth(12);
    //Use f for float, e for exponent float and g for best choice of these two. 
    out << QString::number(fracData.projectedAreas.x(), 'g', 4);
    out << QString::number(fracData.projectedAreas.y(), 'g', 4);
    out << QString::number(fracData.projectedAreas.z(), 'g', 4);
    out << QString::number(fracData.totalArea, 'g', 4);

    out << QString::number(fracData.skinFactor, 'f', 2);
    out << QString::number(fracData.fractureLenght, 'g', 3);

    out << qSetFieldWidth(10);
    out << QString::number(fracData.cellSizes.x(), 'f', 2);
    out << QString::number(fracData.cellSizes.y(), 'f', 2);
    out << QString::number(fracData.cellSizes.z(), 'f', 2);

    out << qSetFieldWidth(12);
    out << QString::number(fracData.permeabilities.x(), 'e', 3);
    out << QString::number(fracData.permeabilities.y(), 'e', 3);
    out << QString::number(fracData.permeabilities.z(), 'e', 3);

    out << qSetFieldWidth(8);
    out << QString::number(fracData.NTG, 'f', 2);

    out << qSetFieldWidth(12);
    out << QString::number(fracData.transmissibilities.x(), 'e', 3);
    out << QString::number(fracData.transmissibilities.y(), 'e', 3);
    out << QString::number(fracData.transmissibilities.z(), 'e', 3);

    out << qSetFieldWidth(15);
    out << QString::number(fracData.transmissibility, 'e', 3);

    if (!fracData.cellIsActive)
    {
        out << qSetFieldWidth(20);
        out << " INACTIVE CELL ";
    }

    else if (fracData.cellIsActive && fracData.transmissibility > 0)
    {
        out << qSetFieldWidth(20);
        out << " ACTIVE CELL ";
    }
    else
    {
        out << qSetFieldWidth(20);
        out << " INVALID DATA ";
    }

    out << "\n";

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifFractureExportTools::printTransmissibilityFractureToWell(const std::vector<RimFracture *>& fractures, QTextStream &out, RimEclipseCase* caseToApply)
{
    out << "-- Transmissibility From Fracture To Well \n";

    out << qSetFieldWidth(12);
    out << "Well name ";   

    out << qSetFieldWidth(16);
    out << "Fracture name ";
    out << "Inflow type ";

    out << qSetFieldWidth(5);
    out << " i ";
    out << " j ";

    out << "Tw";
    out << "\n";

    for (RimFracture* fracture : fractures)
    {
        out << qSetFieldWidth(12);
        RimEclipseWell* simWell = nullptr;
        RimWellPath* wellPath = nullptr;
        fracture->firstAncestorOrThisOfType(simWell);
        if (simWell) out << simWell->name + " ";    // 1. Well name 
        fracture->firstAncestorOrThisOfType(wellPath);
        if (wellPath) out << wellPath->name + " ";  // 1. Well name 

        out << qSetFieldWidth(16);
        out << fracture->name().left(15) + " ";


        if (fracture->attachedFractureDefinition()->orientation == RimFractureTemplate::ALONG_WELL_PATH)
        {
            out << "Linear inflow";
            out << qSetFieldWidth(5);

            RimStimPlanFractureTemplate* fracTemplateStimPlan;
            if (dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition()))
            {
                fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition());
            }
            else continue;

            //TODO: Can be removed when implementation of dip angle is more general: 
            RimSimWellFracture* simWellFrac;
            if (dynamic_cast<RimSimWellFracture*>(fracture))
            {
                simWellFrac = dynamic_cast<RimSimWellFracture*>(fracture);
            }
            else continue;

            double wellDip = simWellFrac->wellDipAtFracturePosition();

            double perforationLengthVert = fracture->perforationLength * cos(wellDip);
            double perforationLengthHor  = fracture->perforationLength * sin(wellDip);
            
            std::pair<size_t, size_t> wellCenterStimPlanCellIJ = fracTemplateStimPlan->getStimPlanCellAtWellCenter();
            out << qSetFieldWidth(5);
            out << wellCenterStimPlanCellIJ.first;
            out << wellCenterStimPlanCellIJ.second;


            //RigStimPlanCell* stimPlanCell = fracTemplateStimPlan->getStimPlanCellAtIJ(wellCenterStimPlanCellIJ.first, wellCenterStimPlanCellIJ.second);
            const RigStimPlanFracTemplateCell& stimPlanCell = fracTemplateStimPlan->stimPlanCellFromIndex(fracTemplateStimPlan->getGlobalIndexFromIJ(wellCenterStimPlanCellIJ.first, wellCenterStimPlanCellIJ.second));

            double linTransInStimPlanCell = RigFractureTransmissibilityEquations::computeLinearTransmissibilityToWellinStimPlanCell(stimPlanCell.getConductivtyValue(),
                                                                                                                                    stimPlanCell.cellSizeX(),
                                                                                                                                    stimPlanCell.cellSizeZ(),
                                                                                                                                    perforationLengthVert,
                                                                                                                                    perforationLengthHor,
                                                                                                                                    fracture->perforationEfficiency,
                                                                                                                                    fracture->attachedFractureDefinition()->skinFactor(),
                                                                                                                                    caseToApply->eclipseCaseData()->darchysValue());

            out << qSetFieldWidth(10);
            out << QString::number(linTransInStimPlanCell, 'f', 2);
            out << "\n";
        }


        if (fracture->attachedFractureDefinition()->orientation == RimFractureTemplate::TRANSVERSE_WELL_PATH
            || fracture->attachedFractureDefinition()->orientation == RimFractureTemplate::AZIMUTH)
        {
            out << "Radial inflow";

            RimStimPlanFractureTemplate* fracTemplateStimPlan;
            if (dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition()))
            {
                fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition());
            }
            else continue;

            std::pair<size_t, size_t> wellCenterStimPlanCellIJ = fracTemplateStimPlan->getStimPlanCellAtWellCenter();
            out << qSetFieldWidth(5);
            out << wellCenterStimPlanCellIJ.first;
            out << wellCenterStimPlanCellIJ.second;

            const RigStimPlanFracTemplateCell& stimPlanCell = fracTemplateStimPlan->stimPlanCellFromIndex(fracTemplateStimPlan->getGlobalIndexFromIJ(wellCenterStimPlanCellIJ.first, wellCenterStimPlanCellIJ.second));

            double radTransInStimPlanCell = RigFractureTransmissibilityEquations::computeRadialTransmissibilityToWellinStimPlanCell(stimPlanCell.getConductivtyValue(),
                                                                                                                                    stimPlanCell.cellSizeX(),
                                                                                                                                    stimPlanCell.cellSizeZ(),
                                                                                                                                    fracture->wellRadius(),
                                                                                                                                    fracture->attachedFractureDefinition()->skinFactor(),
                                                                                                                                    caseToApply->eclipseCaseData()->darchysValue());

            out << qSetFieldWidth(10);
            out << QString::number(radTransInStimPlanCell, 'f', 2);
            out << "\n";

            
        }



    }

    out << "\n";
}
