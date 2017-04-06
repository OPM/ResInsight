/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RigFractureTransCalc.h"
#include "RimFractureTemplate.h"
#include "RigEclipseCaseData.h"
#include "RimEclipseCase.h"
#include "RiaLogging.h"
#include "QString"
#include "RimReservoirCellResultsStorage.h"
#include "RigResultAccessorFactory.h"
#include "RimFracture.h"
#include "RigFracture.h"
#include "cvfGeometryTools.h"
#include "RigCellGeometryTools.h"
#include "RigActiveCellInfo.h"
#include "RigFracture.h"
#include "RimStimPlanFractureTemplate.h"

#include <QString>
#include "RimEllipseFractureTemplate.h"
#include "cafAppEnum.h"
#include "RigCell.h"
#include "RigMainGrid.h"
#include "cvfMath.h"
#include "RimDefines.h"
#include "RigStimPlanCell.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFractureTransCalc::RigFractureTransCalc(RimEclipseCase* caseToApply, RimFracture* fracture)
{
    m_case = caseToApply;
    m_fracture = fracture;


    //Set correct unit system: 
    RigEclipseCaseData::UnitsType caseUnit = m_case->eclipseCaseData()->unitsType();

    if (caseUnit == RigEclipseCaseData::UNITS_METRIC)
    {
        RiaLogging::debug(QString("Calculating transmissibilities in metric units"));
        m_unitForCalculation = RimDefines::UNITS_METRIC;
    }
    else if (caseUnit == RigEclipseCaseData::UNITS_FIELD)
    {
        RiaLogging::debug(QString("Calculating transmissibilities in field units"));
        m_unitForCalculation = RimDefines::UNITS_FIELD;
    }
    else
    {
        //TODO: How to handle lab units for eclipse case?
        RiaLogging::error(QString("Unit system for case not supported for fracture export."));
        RiaLogging::error(QString("Export will be in metric units, but results might be wrong."));
        m_unitForCalculation = RimDefines::UNITS_METRIC;
    }


}



//--------------------------------------------------------------------------------------------------
/// TODO: Document equation
//--------------------------------------------------------------------------------------------------
void RigFractureTransCalc::computeTransmissibility()
{
    if (m_fracture->attachedFractureDefinition()->fractureConductivity == RimFractureTemplate::FINITE_CONDUCTIVITY)
    {
        RiaLogging::warning(QString("Transimssibility for finite conductity in fracture not yet implemented."));
        RiaLogging::warning(QString("Performing calculation for infinite conductivity instead."));
    }

    RigEclipseCaseData* eclipseCaseData = m_case->eclipseCaseData();

    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    RimReservoirCellResultsStorage* gridCellResults = m_case->results(porosityModel);

    size_t scalarSetIndex;
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DX");
    cvf::ref<RigResultAccessor> dataAccessObjectDx = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "DX"); //assuming 0 time step and main grid (so grid index =0) 
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DY");
    cvf::ref<RigResultAccessor> dataAccessObjectDy = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "DY"); //assuming 0 time step and main grid (so grid index =0) 
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DZ");
    cvf::ref<RigResultAccessor> dataAccessObjectDz = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "DZ"); //assuming 0 time step and main grid (so grid index =0) 

    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "PERMX");
    cvf::ref<RigResultAccessor> dataAccessObjectPermX = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "PERMX"); //assuming 0 time step and main grid (so grid index =0) 
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "PERMY");
    cvf::ref<RigResultAccessor> dataAccessObjectPermY = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "PERMY"); //assuming 0 time step and main grid (so grid index =0) 
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "PERMZ");
    cvf::ref<RigResultAccessor> dataAccessObjectPermZ = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "PERMZ"); //assuming 0 time step and main grid (so grid index =0) 
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "NTG");
    cvf::ref<RigResultAccessor> dataAccessObjectNTG = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "NTG"); //assuming 0 time step and main grid (so grid index =0) 

    RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(porosityModel);

    std::vector<RigFractureData> fracDataVec;
    std::vector<size_t> fracCells = m_fracture->getPotentiallyFracturedCells();

    for (size_t fracCell : fracCells)
    {
        bool cellIsActive = activeCellInfo->isActive(fracCell);

        double permX = dataAccessObjectPermX->cellScalarGlobIdx(fracCell);
        double permY = dataAccessObjectPermY->cellScalarGlobIdx(fracCell);
        double permZ = dataAccessObjectPermZ->cellScalarGlobIdx(fracCell);

        double dx = dataAccessObjectDx->cellScalarGlobIdx(fracCell);
        double dy = dataAccessObjectDy->cellScalarGlobIdx(fracCell);
        double dz = dataAccessObjectDz->cellScalarGlobIdx(fracCell);

        double NTG = dataAccessObjectNTG->cellScalarGlobIdx(fracCell);

        cvf::Vec3d localX;
        cvf::Vec3d localY;
        cvf::Vec3d localZ;
        std::vector<std::vector<cvf::Vec3d> > planeCellPolygons;
        bool isPlanIntersected = planeCellIntersectionPolygons(fracCell, planeCellPolygons, localX, localY, localZ);
        if (!isPlanIntersected || planeCellPolygons.size() == 0) continue;

        //Transform planCell polygon(s) and averageZdirection to x/y coordinate system (where fracturePolygon already is located)
        cvf::Mat4f invertedTransMatrix = m_fracture->transformMatrix().getInverted();
        for (std::vector<cvf::Vec3d> & planeCellPolygon : planeCellPolygons)
        {
            for (cvf::Vec3d& v : planeCellPolygon)
            {
                v.transformPoint(static_cast<cvf::Mat4d>(invertedTransMatrix));
            }
        }

        cvf::Vec3d localZinFracPlane;
        localZinFracPlane = localZ;
        localZinFracPlane.transformVector(static_cast<cvf::Mat4d>(invertedTransMatrix));
        cvf::Vec3d directionOfLength = cvf::Vec3d::ZERO;
        directionOfLength.cross(localZinFracPlane, cvf::Vec3d(0, 0, 1));
        directionOfLength.normalize();

        RigFractureData fracData;
        fracData.reservoirCellIndex = fracCell;

        std::vector<cvf::Vec3f> fracPolygon = m_fracture->attachedFractureDefinition()->fracturePolygon(m_unitForCalculation);

        std::vector<cvf::Vec3d> fracPolygonDouble;
        for (auto v : fracPolygon) fracPolygonDouble.push_back(static_cast<cvf::Vec3d>(v));

        std::vector<std::vector<cvf::Vec3d> > polygonsDescribingFractureInCell;
        cvf::Vec3d areaVector;

        for (std::vector<cvf::Vec3d> planeCellPolygon : planeCellPolygons)
        {
            std::vector<std::vector<cvf::Vec3d> >clippedPolygons = RigCellGeometryTools::clipPolygons(planeCellPolygon, fracPolygonDouble);
            for (std::vector<cvf::Vec3d> clippedPolygon : clippedPolygons)
            {
                polygonsDescribingFractureInCell.push_back(clippedPolygon);
            }
        }

        double area;
        std::vector<double> areaOfFractureParts;
        double length;
        std::vector<double> lengthXareaOfFractureParts;
        double Ax = 0.0;
        double Ay = 0.0;
        double Az = 0.0;

        for (std::vector<cvf::Vec3d> fracturePartPolygon : polygonsDescribingFractureInCell)
        {
            areaVector = cvf::GeometryTools::polygonAreaNormal3D(fracturePartPolygon);
            area = areaVector.length();
            areaOfFractureParts.push_back(area);
            length = RigCellGeometryTools::polygonAreaWeightedLength(directionOfLength, fracturePartPolygon);
            lengthXareaOfFractureParts.push_back(length * area);

            cvf::Plane fracturePlane;
            cvf::Mat4f m = m_fracture->transformMatrix();
            bool isCellIntersected = false;

            fracturePlane.setFromPointAndNormal(static_cast<cvf::Vec3d>(m.translation()),
                static_cast<cvf::Vec3d>(m.col(2)));

            Ax += abs(area*(fracturePlane.normal().dot(localY)));
            Ay += abs(area*(fracturePlane.normal().dot(localX)));
            Az += abs(area*(fracturePlane.normal().dot(localZ)));
            //TODO: resulting values have only been checked for vertical fracture...
        }

        double fractureArea = 0.0;
        for (double area : areaOfFractureParts) fractureArea += area;

        double totalAreaXLength = 0.0;
        for (double lengtXarea : lengthXareaOfFractureParts) totalAreaXLength += lengtXarea;
        
        double fractureAreaWeightedlength = totalAreaXLength / fractureArea;
        double skinfactor = m_fracture->attachedFractureDefinition()->skinFactor;

        double transmissibility_X = calculateMatrixTransmissibility(permY, NTG, Ay, dx, skinfactor, fractureAreaWeightedlength);
        double transmissibility_Y = calculateMatrixTransmissibility(permX, NTG, Ax, dy, skinfactor, fractureAreaWeightedlength);
        double transmissibility_Z = calculateMatrixTransmissibility(permZ, 1.0, Az, dz, skinfactor, fractureAreaWeightedlength);

        double transmissibility = sqrt(transmissibility_X * transmissibility_X
            + transmissibility_Y * transmissibility_Y
            + transmissibility_Z * transmissibility_Z);

        fracData.transmissibility = transmissibility;
        fracData.transmissibilities = cvf::Vec3d(transmissibility_X, transmissibility_Y, transmissibility_Z);

        fracData.totalArea = fractureArea;
        fracData.projectedAreas = cvf::Vec3d(Ax, Ay, Az);
        fracData.fractureLenght = fractureAreaWeightedlength;

        fracData.cellSizes = cvf::Vec3d(dx, dy, dz);
        fracData.permeabilities = cvf::Vec3d(permX, permY, permZ);
        fracData.NTG = NTG;
        fracData.skinFactor = skinfactor;
        fracData.cellIsActive = cellIsActive;

        //Since we loop over all potentially fractured cells, we only keep FractureData for cells where fracture have an non-zero area. 
        if (fractureArea > 1e-5)
        {
            fracDataVec.push_back(fracData);
        }

    }

    m_fracture->setFractureData(fracDataVec);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFractureTransCalc::calculateStimPlanCellsMatrixTransmissibility(RigStimPlanCell* stimPlanCell, RigFractureStimPlanCellData* fracStimPlanCellData)
{

    //Not calculating flow into fracture if stimPlan cell cond value is 0 (assumed to be outside the fracture):
    if (stimPlanCell->getConductivtyValue() < 1e-7) return;

    RigEclipseCaseData* eclipseCaseData = m_case->eclipseCaseData();

    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    RimReservoirCellResultsStorage* gridCellResults = m_case->results(porosityModel);

    size_t scalarSetIndex;
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DX");
    cvf::ref<RigResultAccessor> dataAccessObjectDx = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "DX"); //assuming 0 time step and main grid (so grid index =0) 
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DY");
    cvf::ref<RigResultAccessor> dataAccessObjectDy = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "DY"); //assuming 0 time step and main grid (so grid index =0) 
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DZ");
    cvf::ref<RigResultAccessor> dataAccessObjectDz = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "DZ"); //assuming 0 time step and main grid (so grid index =0) 

    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "PERMX");
    cvf::ref<RigResultAccessor> dataAccessObjectPermX = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "PERMX"); //assuming 0 time step and main grid (so grid index =0) 
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "PERMY");
    cvf::ref<RigResultAccessor> dataAccessObjectPermY = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "PERMY"); //assuming 0 time step and main grid (so grid index =0) 
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "PERMZ");
    cvf::ref<RigResultAccessor> dataAccessObjectPermZ = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "PERMZ"); //assuming 0 time step and main grid (so grid index =0) 
    scalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "NTG");
    cvf::ref<RigResultAccessor> dataAccessObjectNTG = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, "NTG"); //assuming 0 time step and main grid (so grid index =0) 

    RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(porosityModel);


    std::vector<cvf::Vec3d> stimPlanPolygon = stimPlanCell->getPolygon();
    std::vector<size_t> fracCells = m_fracture->getPotentiallyFracturedCells();

    for (size_t fracCell : fracCells)
    {
        bool cellIsActive = activeCellInfo->isActive(fracCell);
        if (!cellIsActive) continue;

        double permX = dataAccessObjectPermX->cellScalarGlobIdx(fracCell);
        double permY = dataAccessObjectPermY->cellScalarGlobIdx(fracCell);
        double permZ = dataAccessObjectPermZ->cellScalarGlobIdx(fracCell);

        double dx = dataAccessObjectDx->cellScalarGlobIdx(fracCell);
        double dy = dataAccessObjectDy->cellScalarGlobIdx(fracCell);
        double dz = dataAccessObjectDz->cellScalarGlobIdx(fracCell);

        double NTG = dataAccessObjectNTG->cellScalarGlobIdx(fracCell);

        cvf::Vec3d localX;
        cvf::Vec3d localY;
        cvf::Vec3d localZ;
        std::vector<std::vector<cvf::Vec3d> > planeCellPolygons;
        bool isPlanIntersected = planeCellIntersectionPolygons(fracCell, planeCellPolygons, localX, localY, localZ);
        if (!isPlanIntersected || planeCellPolygons.size() == 0) continue;

        //Transform planCell polygon(s) and averageZdirection to x/y coordinate system (where fracturePolygon already is located)
        cvf::Mat4f invertedTransMatrix = m_fracture->transformMatrix().getInverted();
        for (std::vector<cvf::Vec3d> & planeCellPolygon : planeCellPolygons)
        {
            for (cvf::Vec3d& v : planeCellPolygon)
            {
                v.transformPoint(static_cast<cvf::Mat4d>(invertedTransMatrix));
            }
        }

        cvf::Vec3d localZinFracPlane;
        localZinFracPlane = localZ;
        localZinFracPlane.transformVector(static_cast<cvf::Mat4d>(invertedTransMatrix));
        cvf::Vec3d directionOfLength = cvf::Vec3d::ZERO;
        directionOfLength.cross(localZinFracPlane, cvf::Vec3d(0, 0, 1));
        directionOfLength.normalize();

        std::vector<std::vector<cvf::Vec3d> > polygonsForStimPlanCellInEclipseCell;
        cvf::Vec3d areaVector;

        for (std::vector<cvf::Vec3d> planeCellPolygon : planeCellPolygons)
        {
            std::vector<std::vector<cvf::Vec3d> >clippedPolygons = RigCellGeometryTools::clipPolygons(planeCellPolygon, stimPlanPolygon);
            for (std::vector<cvf::Vec3d> clippedPolygon : clippedPolygons)
            {
                polygonsForStimPlanCellInEclipseCell.push_back(clippedPolygon);
            }
        }

        if (polygonsForStimPlanCellInEclipseCell.size() == 0) continue;

        double area;
        std::vector<double> areaOfFractureParts;
        double length;
        std::vector<double> lengthXareaOfFractureParts;
        double Ax = 0.0, Ay = 0.0, Az = 0.0;

        for (std::vector<cvf::Vec3d> fracturePartPolygon : polygonsForStimPlanCellInEclipseCell)
        {
            areaVector = cvf::GeometryTools::polygonAreaNormal3D(fracturePartPolygon);
            area = areaVector.length();
            areaOfFractureParts.push_back(area);

            //TODO: the l in the sl/pi term in the denominator of the Tmj expression should be the length of the full Eclipse cell
            //In the current form the implementation gives correct result only if s=0 (fracture templte skin factor). 
            length = RigCellGeometryTools::polygonAreaWeightedLength(directionOfLength, fracturePartPolygon);
            lengthXareaOfFractureParts.push_back(length * area);

            cvf::Plane fracturePlane;
            cvf::Mat4f m = m_fracture->transformMatrix();
            bool isCellIntersected = false;

            fracturePlane.setFromPointAndNormal(static_cast<cvf::Vec3d>(m.translation()),
                static_cast<cvf::Vec3d>(m.col(2)));

            Ax += abs(area*(fracturePlane.normal().dot(localY)));
            Ay += abs(area*(fracturePlane.normal().dot(localX)));
            Az += abs(area*(fracturePlane.normal().dot(localZ)));
        }

        double fractureArea = 0.0;
        for (double area : areaOfFractureParts) fractureArea += area;

        double totalAreaXLength = 0.0;
        for (double lengtXarea : lengthXareaOfFractureParts) totalAreaXLength += lengtXarea;

        double fractureAreaWeightedlength = totalAreaXLength / fractureArea;
        double skinfactor = m_fracture->attachedFractureDefinition()->skinFactor;



        double transmissibility_X = calculateMatrixTransmissibility(permY, NTG, Ay, dx, skinfactor, fractureAreaWeightedlength);
        double transmissibility_Y = calculateMatrixTransmissibility(permX, NTG, Ax, dy, skinfactor, fractureAreaWeightedlength);
        double transmissibility_Z = calculateMatrixTransmissibility(permZ, 1.0, Az, dz, skinfactor, fractureAreaWeightedlength);

        double transmissibility = sqrt(transmissibility_X * transmissibility_X
            + transmissibility_Y * transmissibility_Y
            + transmissibility_Z * transmissibility_Z);




        fracStimPlanCellData->addContributingEclipseCell(fracCell, transmissibility);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigFractureTransCalc::planeCellIntersectionPolygons(size_t cellindex, std::vector<std::vector<cvf::Vec3d> > & polygons,
    cvf::Vec3d & localX, cvf::Vec3d & localY, cvf::Vec3d & localZ)
{

    cvf::Plane fracturePlane;
    cvf::Mat4f m = m_fracture->transformMatrix();
    bool isCellIntersected = false;

    fracturePlane.setFromPointAndNormal(static_cast<cvf::Vec3d>(m.translation()),
        static_cast<cvf::Vec3d>(m.col(2)));

    const RigMainGrid* mainGrid = m_case->eclipseCaseData()->mainGrid();
    if (!mainGrid) return false;

    RigCell cell = mainGrid->globalCellArray()[cellindex];
    if (cell.isInvalid()) return mainGrid;

    if (cellindex == 186234)
    {
        cvf::Vec3d cellcenter = cell.center();
    }

    //Copied (and adapted) from RigEclipseWellLogExtractor
    cvf::Vec3d hexCorners[8];
    const std::vector<cvf::Vec3d>& nodeCoords = mainGrid->nodes();
    const caf::SizeTArray8& cornerIndices = cell.cornerIndices();

    hexCorners[0] = nodeCoords[cornerIndices[0]];
    hexCorners[1] = nodeCoords[cornerIndices[1]];
    hexCorners[2] = nodeCoords[cornerIndices[2]];
    hexCorners[3] = nodeCoords[cornerIndices[3]];
    hexCorners[4] = nodeCoords[cornerIndices[4]];
    hexCorners[5] = nodeCoords[cornerIndices[5]];
    hexCorners[6] = nodeCoords[cornerIndices[6]];
    hexCorners[7] = nodeCoords[cornerIndices[7]];

    //Find line-segments where cell and fracture plane intersects
    std::list<std::pair<cvf::Vec3d, cvf::Vec3d > > intersectionLineSegments;

    isCellIntersected = RigCellGeometryTools::planeHexCellIntersection(hexCorners, fracturePlane, intersectionLineSegments);

    RigCellGeometryTools::createPolygonFromLineSegments(intersectionLineSegments, polygons);

    RigCellGeometryTools::findCellLocalXYZ(hexCorners, localX, localY, localZ);

    return isCellIntersected;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RigFractureTransCalc::flowAcrossLayersUpscaling(QString resultName, QString resultUnit, size_t timeStepIndex, RimDefines::UnitSystem unitSystem, size_t eclipseCellIndex)
{
    RimStimPlanFractureTemplate* fracTemplateStimPlan;
    if (dynamic_cast<RimStimPlanFractureTemplate*>(m_fracture->attachedFractureDefinition()))
    {
        fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(m_fracture->attachedFractureDefinition());
    }
    else return std::make_pair(cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE);

    std::vector<RigStimPlanCell> stimPlanCells = fracTemplateStimPlan->getStimPlanCells();

    cvf::Vec3d localX, localY, localZ; //Not used in calculation here, but needed for function to find planCellPolygons
    std::vector<std::vector<cvf::Vec3d> > planeCellPolygons;
    bool isPlanIntersected = planeCellIntersectionPolygons(eclipseCellIndex, planeCellPolygons, localX, localY, localZ);
    if (!isPlanIntersected || planeCellPolygons.size() == 0) return  std::make_pair(cvf::UNDEFINED_DOUBLE, cvf::UNDEFINED_DOUBLE);

    //Transform planCell polygon(s) and averageZdirection to x/y coordinate system (where fracturePolygon/stimPlan mesh already is located)
    cvf::Mat4f invertedTransMatrix = m_fracture->transformMatrix().getInverted();
    for (std::vector<cvf::Vec3d> & planeCellPolygon : planeCellPolygons)
    {
        for (cvf::Vec3d& v : planeCellPolygon)
        {
            v.transformPoint(static_cast<cvf::Mat4d>(invertedTransMatrix));
        }
    }

    cvf::Vec3d directionAcrossLayers;
    cvf::Vec3d directionAlongLayers;
    directionAcrossLayers = cvf::Vec3d(0.0, -1.0, 0.0);
    directionAlongLayers = cvf::Vec3d(1.0, 0.0, 0.0);

    std::vector<cvf::Vec3f> fracPolygon = m_fracture->attachedFractureDefinition()->fracturePolygon(unitSystem);
    std::vector<std::vector<cvf::Vec3d> > polygonsDescribingFractureInCell;

    std::vector<double> upscaledConductivitiesHA;
    std::vector<double> upscaledConductivitiesAH;

    for (std::vector<cvf::Vec3d> planeCellPolygon : planeCellPolygons)
    {
//         //For debug only, to compare with results in Excel: 
//         std::vector<cvf::Vec3d> planeCellPolygonHacked;
//         if (eclipseCellIndex == 134039)
//         {
//             planeCellPolygonHacked.push_back(cvf::Vec3d(-12.0, -3.5, 0.0));
//             planeCellPolygonHacked.push_back(cvf::Vec3d(12.0, -3.5, 0.0));
//             planeCellPolygonHacked.push_back(cvf::Vec3d(12.0, 19.0, 0.0));
//             planeCellPolygonHacked.push_back(cvf::Vec3d(-12.0, 19.0, 0.0));
//             planeCellPolygon = planeCellPolygonHacked;
//         }

        double condHA = computeHAupscale(fracTemplateStimPlan, stimPlanCells, planeCellPolygon, directionAlongLayers, directionAcrossLayers);
        upscaledConductivitiesHA.push_back(condHA);

        double condAH = computeAHupscale(fracTemplateStimPlan, stimPlanCells, planeCellPolygon, directionAlongLayers, directionAcrossLayers);
        upscaledConductivitiesAH.push_back(condAH);
    }

    return std::make_pair(arithmeticAverage(upscaledConductivitiesHA), arithmeticAverage(upscaledConductivitiesAH));
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransCalc::computeHAupscale(RimStimPlanFractureTemplate* fracTemplateStimPlan, std::vector<RigStimPlanCell> stimPlanCells, std::vector<cvf::Vec3d> planeCellPolygon, cvf::Vec3d directionAlongLayers, cvf::Vec3d directionAcrossLayers)
{
    std::vector<double> DcolSum;
    std::vector<double> lavgCol;
    std::vector<double> CondHarmCol;

    for (size_t j = 0; j < fracTemplateStimPlan->stimPlanGridNumberOfColums(); j++)
    {
        std::vector<double> conductivitiesInStimPlanCells;
        std::vector<double> lengthsLiOfStimPlanCol;
        std::vector<double> heightsDiOfStimPlanCells;

        std::vector<RigStimPlanCell*> stimPlanCellsCol = getColOfStimPlanCells(stimPlanCells, j);
        for (RigStimPlanCell* stimPlanCell : stimPlanCellsCol)
        {
            if (stimPlanCell->getConductivtyValue() > 1e-7)
            {
                std::vector<std::vector<cvf::Vec3d> >clippedStimPlanPolygons = RigCellGeometryTools::clipPolygons(stimPlanCell->getPolygon(), planeCellPolygon);
                if (clippedStimPlanPolygons.size() > 0)
                {
                    for (auto clippedStimPlanPolygon : clippedStimPlanPolygons)
                    {
                        conductivitiesInStimPlanCells.push_back(stimPlanCell->getConductivtyValue());
                        lengthsLiOfStimPlanCol.push_back(RigCellGeometryTools::polygonAreaWeightedLength(directionAlongLayers, clippedStimPlanPolygon));
                        heightsDiOfStimPlanCells.push_back(RigCellGeometryTools::polygonAreaWeightedLength(directionAcrossLayers, clippedStimPlanPolygon));
                    }
                }
            }
        }
        //Regne ut average
        double sumDiDivCondLi = 0.0;
        double sumDi = 0.0;
        double sumLiDi = 0.0;
        for (int i = 0; i < conductivitiesInStimPlanCells.size(); i++)
        {
            sumDiDivCondLi += heightsDiOfStimPlanCells[i] / (conductivitiesInStimPlanCells[i] * lengthsLiOfStimPlanCol[i]);
            sumDi += heightsDiOfStimPlanCells[i];
            sumLiDi += heightsDiOfStimPlanCells[i] * lengthsLiOfStimPlanCol[i];
        }

        if (sumDiDivCondLi != 0)
        {
            DcolSum.push_back(sumDi);
            double lAvgValue = sumLiDi / sumDi;
            lavgCol.push_back(lAvgValue);
            double harmMeanForCol = (sumDi / lAvgValue) * (1 / sumDiDivCondLi);
            CondHarmCol.push_back(harmMeanForCol); 
        }
    }

    //Do arithmetic upscaling based on harmonic upscaled values for coloums
    double sumCondHLiDivDi = 0.0;
    double sumLi = 0.0;
    double sumDiLi = 0.0;
    for (int i = 0; i < CondHarmCol.size(); i++)
    {
        sumLi += lavgCol[i];
        sumDiLi += DcolSum[i] * lavgCol[i];
        sumCondHLiDivDi += CondHarmCol[i] * lavgCol[i] / DcolSum[i];
    }
    double Davg = sumDiLi / sumLi;
    double condHA = (Davg / sumLi) * sumCondHLiDivDi;

    return condHA;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransCalc::computeAHupscale(RimStimPlanFractureTemplate* fracTemplateStimPlan, std::vector<RigStimPlanCell> stimPlanCells, std::vector<cvf::Vec3d> planeCellPolygon, cvf::Vec3d directionAlongLayers, cvf::Vec3d directionAcrossLayers)
{
    std::vector<double> DrowAvg;
    std::vector<double> liRowSum;
    std::vector<double> CondAritRow;
    
    for (size_t j = 0; j < fracTemplateStimPlan->stimPlanGridNumberOfRows(); j++)
    {
        std::vector<double> conductivitiesInStimPlanCells;
        std::vector<double> lengthsLiOfStimPlanCol;
        std::vector<double> heightsDiOfStimPlanCells;

        std::vector<RigStimPlanCell*> stimPlanCellsCol = getRowOfStimPlanCells(stimPlanCells, j);
        for (RigStimPlanCell* stimPlanCell : stimPlanCellsCol)
        {
            if (stimPlanCell->getConductivtyValue() > 1e-7)
            {
                std::vector<std::vector<cvf::Vec3d> >clippedStimPlanPolygons = RigCellGeometryTools::clipPolygons(stimPlanCell->getPolygon(), planeCellPolygon);
                if (clippedStimPlanPolygons.size() > 0)
                {
                    for (auto clippedStimPlanPolygon : clippedStimPlanPolygons)
                    {
                        conductivitiesInStimPlanCells.push_back(stimPlanCell->getConductivtyValue());
                        lengthsLiOfStimPlanCol.push_back(RigCellGeometryTools::polygonAreaWeightedLength(directionAlongLayers, clippedStimPlanPolygon));
                        heightsDiOfStimPlanCells.push_back(RigCellGeometryTools::polygonAreaWeightedLength(directionAcrossLayers, clippedStimPlanPolygon));
                    }
                }
            }
        }
        //Calculate sums needed for (arithmetic) average for coloum
        double sumCondLiDivDi = 0.0;
        double sumDi = 0.0;
        double sumLiDi = 0.0;
        double sumLi = 0.0;
        for (int i = 0; i < conductivitiesInStimPlanCells.size(); i++)
        {
            sumCondLiDivDi += (conductivitiesInStimPlanCells[i] * lengthsLiOfStimPlanCol[i]) / heightsDiOfStimPlanCells[i];
            sumDi += heightsDiOfStimPlanCells[i];
            sumLiDi += heightsDiOfStimPlanCells[i] * lengthsLiOfStimPlanCol[i];
            sumLi += lengthsLiOfStimPlanCol[i];
        }

        if (sumCondLiDivDi != 0)
        {
            //Calculating art avg
            double dAvg = sumLiDi / sumLi;
            DrowAvg.push_back(dAvg);
            liRowSum.push_back(sumLi);
            CondAritRow.push_back(dAvg / sumLi * sumCondLiDivDi);
        }
    }

    //Do harmonic upscaling based on arithmetric upscaled values for coloums
    double sumDiDivCondALi = 0.0;
    double sumDi = 0.0;
    double sumDiLi = 0.0;
    for (int i = 0; i < CondAritRow.size(); i++)
    {
        sumDi += DrowAvg[i];
        sumDiLi += DrowAvg[i] * liRowSum[i];
        sumDiDivCondALi += DrowAvg[i] / (CondAritRow[i] * liRowSum[i]);

    }
    double Lavg = sumDiLi / sumDi;
    double condAH = (sumDi / Lavg) * (1 / sumDiDivCondALi);

    return condAH;



}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransCalc::arithmeticAverage(std::vector<double> values)
{
    if (values.size() == 0) return cvf::UNDEFINED_DOUBLE;

    double sumValue = 0.0;
    size_t numberOfValues = 0;
    for (double value : values)
    {
        sumValue += value;
        numberOfValues++;
    }

    return sumValue / numberOfValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFractureTransCalc::computeUpscaledPropertyFromStimPlan( QString resultName, QString resultUnit, size_t timeStepIndex)
{


    RimStimPlanFractureTemplate* fracTemplateStimPlan;
    if (dynamic_cast<RimStimPlanFractureTemplate*>(m_fracture->attachedFractureDefinition()))
    {
        fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(m_fracture->attachedFractureDefinition());
    }
    else return;


    std::vector<std::vector<cvf::Vec3d> > stimPlanCellsAsPolygons;
    std::vector<double> stimPlanParameterValues;
    fracTemplateStimPlan->getStimPlanDataAsPolygonsAndValues(stimPlanCellsAsPolygons, stimPlanParameterValues, resultName, resultUnit, timeStepIndex);

    //TODO: A lot of common code with function above... Can be cleaned up...?
    std::vector<size_t> fracCells = m_fracture->getPotentiallyFracturedCells();

    RigEclipseCaseData* eclipseCaseData = m_case->eclipseCaseData();
    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    RimReservoirCellResultsStorage* gridCellResults = m_case->results(porosityModel);
    RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(porosityModel);

    std::vector<RigFractureData> fracDataVec;

    for (size_t fracCell : fracCells)
    {

        RigFractureData fracData;
        fracData.reservoirCellIndex = fracCell;
            
        std::pair<double, double> upscaledCondFlowAcrossLayers = flowAcrossLayersUpscaling(resultName, resultUnit, timeStepIndex, m_unitForCalculation, fracCell);
                
        double upscaledStimPlanValueHA = upscaledCondFlowAcrossLayers.first;
        double upscaledStimPlanValueAH = upscaledCondFlowAcrossLayers.second;
        
        if (upscaledStimPlanValueHA != cvf::UNDEFINED_DOUBLE)
        {
            fracData.upscaledStimPlanValueHA = upscaledStimPlanValueHA;
            fracData.upscaledStimPlanValueAH = upscaledStimPlanValueAH;

            fracDataVec.push_back(fracData);
        }
    }


    //TODO: Hvis fracture allerede har en sånn vektor, trenger vi vel ikke en til for RigFractureTransCalc...? 
    //Trenger bare funksjoner for å sette / hente ut data for en gitt Eclipse celle... 
    m_fracture->setFractureData(fracDataVec);



}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFractureTransCalc::computeFlowInFracture()
{

    //TODO: A lot of common code with function for calculating transmissibility... 

    RimStimPlanFractureTemplate* fracTemplateStimPlan;
    RimEllipseFractureTemplate* fracTemplateEllipse;
    if (dynamic_cast<RimStimPlanFractureTemplate*>(m_fracture->attachedFractureDefinition()))
    {
        fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(m_fracture->attachedFractureDefinition());
    }
    else if (dynamic_cast<RimEllipseFractureTemplate*>(m_fracture->attachedFractureDefinition()))
    {
        fracTemplateEllipse = dynamic_cast<RimEllipseFractureTemplate*>(m_fracture->attachedFractureDefinition());
    }
    else return;

    //TODO: A lot of common code with function above... Can be cleaned up...?
    std::vector<size_t> fracCells = m_fracture->getPotentiallyFracturedCells();


    RigEclipseCaseData* eclipseCaseData = m_case->eclipseCaseData();

    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    RimReservoirCellResultsStorage* gridCellResults = m_case->results(porosityModel);
    RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(porosityModel);


    std::vector<RigFractureData> fracDataVec;

    for (size_t fracCell : fracCells)
    {
        double Kw;  // Conductivity (Permeability K times width w)

        if (fracTemplateEllipse)
        {
            double K = fracTemplateEllipse->fractureConductivity();     // Permeability
            double w = fracTemplateEllipse->width();                    // Width

            //TODO: UNit handling...
            if (fracTemplateEllipse->fractureTemplateUnit() == RimDefines::UNITS_FIELD)
            {
                w = RimDefines::inchToFeet(w);
            }
    
            Kw = K * w;
        }
        else if (fracTemplateStimPlan)
        {
            QString resultName = "CONDUCTIVITY";
            QString resultUnit;
            if (fracTemplateStimPlan->fractureTemplateUnit() == RimDefines::UNITS_METRIC) resultUnit = "md-m";
            if (fracTemplateStimPlan->fractureTemplateUnit() == RimDefines::UNITS_FIELD)  resultUnit = "md-ft";

            size_t timeStepIndex = 0; //TODO... 
            caf::AppEnum< RimDefines::UnitSystem > unitSystem = RimDefines::UNITS_METRIC;
            std::pair<double, double> upscaledValues = flowAcrossLayersUpscaling(resultName, resultUnit, timeStepIndex, unitSystem, fracCell);
            Kw = (upscaledValues.first + upscaledValues.second) / 2;
        }

        Kw = convertConductivtyValue(Kw, fracTemplateEllipse->fractureTemplateUnit(), m_unitForCalculation);
    }


    m_fracture->setFractureData(fracDataVec);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFractureTransCalc::computeFlowIntoTransverseWell()
{

    //TODO: A lot of common code with function for calculating transmissibility... 

    if (m_fracture->attachedFractureDefinition()->orientation == RimFractureTemplate::ALONG_WELL_PATH) return;

    double areaScalingFactor = 1.0;
    if (m_fracture->attachedFractureDefinition()->orientation == RimFractureTemplate::AZIMUTH)
    {
        areaScalingFactor = 1 / cvf::Math::cos((m_fracture->azimuth() - (m_fracture->wellAzimuthAtFracturePosition()-90) ));
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigStimPlanCell*> RigFractureTransCalc::getRowOfStimPlanCells(std::vector<RigStimPlanCell> allStimPlanCells, size_t i)
{
    std::vector<RigStimPlanCell*> stimPlanCellRow;

    for (RigStimPlanCell stimPlanCell : allStimPlanCells)
    {
        if (stimPlanCell.getI() == i)
        {
            stimPlanCellRow.push_back(&stimPlanCell);
        }
    }

    return stimPlanCellRow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigStimPlanCell*> RigFractureTransCalc::getColOfStimPlanCells(std::vector<RigStimPlanCell> allStimPlanCells, size_t j)
{
    std::vector<RigStimPlanCell*> stimPlanCellCol;

    for (RigStimPlanCell stimPlanCell : allStimPlanCells)
    {
        if (stimPlanCell.getJ() == j)
        {
            stimPlanCellCol.push_back(&stimPlanCell);
        }
    }

    return stimPlanCellCol;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransCalc::convertConductivtyValue(double Kw, RimDefines::UnitSystem fromUnit, RimDefines::UnitSystem toUnit)
{

    if (fromUnit == toUnit) return Kw;

    else if (fromUnit == RimDefines::UNITS_METRIC && toUnit == RimDefines::UNITS_FIELD)
    {
        return RimDefines::meterToFeet(Kw);
    }
    else if (fromUnit == RimDefines::UNITS_METRIC && toUnit == RimDefines::UNITS_FIELD)
    {
        return RimDefines::feetToMeter(Kw);
    }

    return cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransCalc::calculateMatrixTransmissibility(double perm, double NTG, double A, double cellSizeLength, double skinfactor, double fractureAreaWeightedlength)
{
    double transmissibility; 

    double c = cvf::UNDEFINED_DOUBLE;
    if (m_unitForCalculation == RimDefines::UNITS_METRIC) c = 0.00852702;
    if (m_unitForCalculation == RimDefines::UNITS_FIELD)  c = 0.00112712;
    // TODO: Use value from RimReservoirCellResultsStorage?       


    double slDivPi = (skinfactor * fractureAreaWeightedlength) / cvf::PI_D;
    transmissibility = 8 * c * (perm * NTG) * A / (cellSizeLength + slDivPi);

    return transmissibility;
}
