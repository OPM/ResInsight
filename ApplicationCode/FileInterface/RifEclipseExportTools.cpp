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

#include "RifEclipseExportTools.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RigEclipseCaseData.h"
#include "RigFracture.h"
#include "RigFractureTransCalc.h"
#include "RigStimPlanFracTemplateCell.h"
#include "RigMainGrid.h"
#include "RigEclipseToStimPlanCellTransmissibilityCalculator.h"

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



//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseExportTools::RifEclipseExportTools()
{

}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseExportTools::~RifEclipseExportTools()
{
  
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseExportTools::writeFracturesToTextFile(const QString& fileName,  const std::vector< RimFracture*>& fractures, RimEclipseCase* caseToApply)
{
    RiaLogging::info(QString("Computing and writing COMPDAT values to file %1").arg(fileName));

    RiaApplication* app = RiaApplication::instance();
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return false;
    RimEclipseView* activeRiv = dynamic_cast<RimEclipseView*>(activeView);
    if (!activeRiv) return false;

    const RigMainGrid* mainGrid = activeRiv->mainGrid();
    if (!mainGrid) return false;


    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    caf::ProgressInfo pi(fractures.size(), QString("Writing data to file %1").arg(fileName));
    RimEclipseWell* simWell = nullptr;
    RimWellPath* wellPath = nullptr;

    size_t progress =0;
    std::vector<size_t> ijk;

    QTextStream out(&file);
    out << "\n";
    out << "-- Exported from ResInsight" << "\n";

    RigEclipseCaseData::UnitsType caseUnit = caseToApply->eclipseCaseData()->unitsType();
    if (caseUnit == RigEclipseCaseData::UNITS_METRIC) out << "-- Using metric unit system" << "\n";
    if (caseUnit == RigEclipseCaseData::UNITS_FIELD) out << "-- Using field unit system" << "\n";
    out << "\n";

    //Included for debug / prototyping only
    printTransmissibilityFractureToWell(fractures, out, caseToApply);
    
    printStimPlanFractureTrans(fractures, out);
    
    printStimPlanCellsMatrixTransContributions(fractures, caseToApply, out, wellPath, simWell, mainGrid);
    
    printBackgroundDataHeaderLine(out);
    
    RiaLogging::debug(QString("Writing intermediate results from COMPDAT calculation"));

    for (RimFracture* fracture : fractures)
    {
        RigFractureTransCalc transmissibilityCalculator(caseToApply, fracture);

        //TODO: Check that there is a fracture template available for given fracture....
        transmissibilityCalculator.computeTransmissibilityFromPolygonWithInfiniteConductivityInFracture();
        std::vector<RigFracturedEclipseCellExportData> fracDataVector = fracture->attachedRigFracture()->fractureData();

        for (RigFracturedEclipseCellExportData fracData : fracDataVector)
        {
            printBackgroundData(out, wellPath, simWell, fracture, mainGrid, fracData);
        }
    }

    out << "\n";

    out << qSetFieldWidth(7) << "COMPDAT" << "\n" << right << qSetFieldWidth(8);
    for (RimFracture* fracture : fractures)
    {
        RiaLogging::debug(QString("Writing COMPDAT values for fracture %1").arg(fracture->name()));
        std::vector<RigFracturedEclipseCellExportData> fracDataVector = fracture->attachedRigFracture()->fractureData();

        for (RigFracturedEclipseCellExportData fracData : fracDataVector)
        {
            if (fracData.transmissibility > 0)
            {
            printCOMPDATvalues(out, fracData, fracture, wellPath, simWell, mainGrid);
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
//--------------------------------------------------------------------------------------------------
void RifEclipseExportTools::performStimPlanUpscalingAndPrintResults(const std::vector<RimFracture *>& fractures, RimEclipseCase* caseToApply, QTextStream &out, RimWellPath* wellPath, RimEclipseWell* simWell, const RigMainGrid* mainGrid)
{


    //TODO: Get these more generally: 
    QString resultName = "CONDUCTIVITY";
    QString resultUnit = "md-m";
    size_t timeStepIndex = 0;



    for (RimFracture* fracture : fractures) //For testing upscaling...
    {
        RigFractureTransCalc transmissibilityCalculator(caseToApply, fracture);
        transmissibilityCalculator.computeUpscaledPropertyFromStimPlan(resultName, resultUnit, timeStepIndex);
        std::vector<RigFracturedEclipseCellExportData> fracDataVector = fracture->attachedRigFracture()->fractureData();

        out << qSetFieldWidth(4);
        out << "-- ";

        out << qSetFieldWidth(12);
        out << "Well";

        out << qSetFieldWidth(16);
        out << "Fracture name ";


        out << qSetFieldWidth(5);
        out << "i";          // 2. I location grid block, adding 1 to go to eclipse 1-based grid definition
        out << "j";          // 3. J location grid block, adding 1 to go to eclipse 1-based grid definition
        out << "k";          // 4. K location of upper connecting grid block, adding 1 to go to eclipse 1-based grid definition

        out << qSetFieldWidth(10);
        out << "cellIndex";
        out << "condHA. ";
        out << "condAH. ";

        out << "\n";

        for (RigFracturedEclipseCellExportData fracData : fracDataVector)
        {

            out << qSetFieldWidth(4);
            out << "-- ";

            out << qSetFieldWidth(12);
            wellPath, simWell = nullptr;
            fracture->firstAncestorOrThisOfType(simWell);
            if (simWell) out << simWell->name + " ";    // 1. Well name 
            fracture->firstAncestorOrThisOfType(wellPath);
            if (wellPath) out << wellPath->name + " ";  // 1. Well name 

            out << qSetFieldWidth(16);
            out << fracture->name().left(15) + " ";


            out << qSetFieldWidth(5);
            size_t i, j, k;
            mainGrid->ijkFromCellIndex(fracData.reservoirCellIndex, &i, &j, &k);
            out << i + 1;          // 2. I location grid block, adding 1 to go to eclipse 1-based grid definition
            out << j + 1;          // 3. J location grid block, adding 1 to go to eclipse 1-based grid definition
            out << k + 1;          // 4. K location of upper connecting grid block, adding 1 to go to eclipse 1-based grid definition

            out << qSetFieldWidth(10);
            out << fracData.reservoirCellIndex;
            out << QString::number(fracData.upscaledStimPlanValueHA, 'f', 3);
            out << QString::number(fracData.upscaledStimPlanValueAH, 'f', 3);

            out << "\n";
        }
    }    
    return;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseExportTools::printStimPlanCellsMatrixTransContributions(const std::vector<RimFracture *>& fractures, RimEclipseCase* caseToApply, QTextStream &out, RimWellPath* wellPath, RimEclipseWell* simWell, const RigMainGrid* mainGrid)
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

        RigFractureTransCalc transmissibilityCalculator(caseToApply, fracture);
        double cDarcyInCorrectUnit = transmissibilityCalculator.cDarcy();

        std::vector<RigStimPlanFracTemplateCell> stimPlanCells = fracTemplateStimPlan->getStimPlanCells();

        for (RigStimPlanFracTemplateCell stimPlanCell : stimPlanCells)
        {
            if (stimPlanCell.getConductivtyValue() < 1e-7)
            {
                continue;
            }

            RigStimPlanFractureCell fracStimPlanCellData(stimPlanCell.getI(), stimPlanCell.getJ());

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
                wellPath, simWell = nullptr;
                fracture->firstAncestorOrThisOfType(simWell);
                if (simWell) out << simWell->name + " ";    // 1. Well name 
                fracture->firstAncestorOrThisOfType(wellPath);
                if (wellPath) out << wellPath->name + " ";  // 1. Well name 

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
void RifEclipseExportTools::printStimPlanFractureTrans(const std::vector<RimFracture *>& fractures, QTextStream &out)
{
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

        double verticalTrans = RigFractureTransCalc::computeStimPlanCellTransmissibilityInFracture(stimPlanCell.getConductivtyValue(), stimPlanCell.cellSizeX(), stimPlanCell.cellSizeZ());
        double horizontalTrans = RigFractureTransCalc::computeStimPlanCellTransmissibilityInFracture(stimPlanCell.getConductivtyValue(), stimPlanCell.cellSizeZ(), stimPlanCell.cellSizeX());

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
void RifEclipseExportTools::printCOMPDATvalues(QTextStream & out, RigFracturedEclipseCellExportData &fracData, RimFracture* fracture, RimWellPath* wellPath, RimEclipseWell* simWell, const RigMainGrid* mainGrid)
{
    out << qSetFieldWidth(8);
    if (fracData.transmissibility == cvf::UNDEFINED_DOUBLE || !(fracture->attachedFractureDefinition())) out << "--"; //Commenting out line in output file

    wellPath, simWell = nullptr;
    fracture->firstAncestorOrThisOfType(simWell);
    if (simWell) out << simWell->name;    // 1. Well name 
    fracture->firstAncestorOrThisOfType(wellPath);
    if (wellPath) out << wellPath->name;  // 1. Well name 

    out << qSetFieldWidth(5);

    size_t i, j, k;
    mainGrid->ijkFromCellIndex(fracData.reservoirCellIndex, &i, &j, &k);
    out << i + 1;          // 2. I location grid block, adding 1 to go to eclipse 1-based grid definition
    out << j + 1;          // 3. J location grid block, adding 1 to go to eclipse 1-based grid definition
    out << k + 1;          // 4. K location of upper connecting grid block, adding 1 to go to eclipse 1-based grid definition
    out << k + 1;          // 5. K location of lower connecting grid block, adding 1 to go to eclipse 1-based grid definition

    out << "2* ";         // Default value for 
                         //6. Open / Shut flag of connection
                         // 7. Saturation table number for connection rel perm. Default value

    out << qSetFieldWidth(12);
    // 8. Transmissibility 
    if (fracData.transmissibility != cvf::UNDEFINED_DOUBLE) out << QString::number(fracData.transmissibility, 'e', 4);
    else out << "UNDEF";

    out << qSetFieldWidth(4);
    out << "2* ";         // Default value for 
                         // 9. Well bore diameter. Set to default
                         // 10. Effective Kh (perm times width)

    if (fracture->attachedFractureDefinition())
    {
        out << fracture->attachedFractureDefinition()->skinFactor;    // 11. Skin factor
    }
    else //If no attached fracture definition these parameters are set to UNDEF
    {
        out << "UNDEF";
    }

    out << "/";
    out << " " << fracture->name(); //Fracture name as comment
    out << "\n"; // Terminating entry

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseExportTools::printBackgroundDataHeaderLine(QTextStream & out)
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
void RifEclipseExportTools::printBackgroundData(QTextStream & out, RimWellPath* wellPath, RimEclipseWell* simWell, RimFracture* fracture, const RigMainGrid* mainGrid, RigFracturedEclipseCellExportData &fracData)
{
    out << qSetFieldWidth(4);
    out << "-- ";

    out << qSetFieldWidth(12);
    wellPath, simWell = nullptr;
    fracture->firstAncestorOrThisOfType(simWell);
    if (simWell) out << simWell->name + " " ;    // 1. Well name 
    fracture->firstAncestorOrThisOfType(wellPath);
    if (wellPath) out << wellPath->name + " ";  // 1. Well name 

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
void RifEclipseExportTools::printTransmissibilityFractureToWell(const std::vector<RimFracture *>& fractures, QTextStream &out, RimEclipseCase* caseToApply)
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

            RigFractureTransCalc transmissibilityCalculator(caseToApply, fracture);
            double linTransInStimPlanCell = transmissibilityCalculator.computeLinearTransmissibilityToWellinStimPlanCell(stimPlanCell, perforationLengthVert, perforationLengthHor);

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

            //RigStimPlanCell* stimPlanCell = fracTemplateStimPlan->getStimPlanCellAtIJ(wellCenterStimPlanCellIJ.first, wellCenterStimPlanCellIJ.second);
            const RigStimPlanFracTemplateCell& stimPlanCell = fracTemplateStimPlan->stimPlanCellFromIndex(fracTemplateStimPlan->getGlobalIndexFromIJ(wellCenterStimPlanCellIJ.first, wellCenterStimPlanCellIJ.second));

            //TODO: Error - stimPlanCell blir ikke riktig... 

            RigFractureTransCalc transmissibilityCalculator(caseToApply, fracture);
            double radTransInStimPlanCell = transmissibilityCalculator.computeRadialTransmissibilityToWellinStimPlanCell(stimPlanCell);

            out << qSetFieldWidth(10);
            out << QString::number(radTransInStimPlanCell, 'f', 2);
            out << "\n";

            
        }



    }

    out << "\n";
}
