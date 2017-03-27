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
#include "RimStimPlanFractureTemplate.h"

#include <QString>
#include "RimEllipseFractureTemplate.h"
#include "cafAppEnum.h"
#include "RigCell.h"
#include "RigMainGrid.h"
#include "cvfMath.h"
#include "RimDefines.h"

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
/// 
//--------------------------------------------------------------------------------------------------
//TODO: Make static and move to another class
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

        double transmissibility;
        double fractureArea = 0.0;
        double fractureAreaWeightedlength = 0.0;
        double Ax = 0.0;
        double Ay = 0.0;
        double Az = 0.0;
        double skinfactor = 0.0;
        double transmissibility_X = 0.0;
        double transmissibility_Y = 0.0;
        double transmissibility_Z = 0.0;


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

        fractureArea = 0.0;
        for (double area : areaOfFractureParts) fractureArea += area;

        double totalAreaXLength = 0.0;
        for (double lengtXarea : lengthXareaOfFractureParts) totalAreaXLength += lengtXarea;
        fractureAreaWeightedlength = totalAreaXLength / fractureArea;

        double c = cvf::UNDEFINED_DOUBLE;
        if (m_unitForCalculation == RimDefines::UNITS_METRIC) c = 0.00852702;
        if (m_unitForCalculation == RimDefines::UNITS_FIELD)  c = 0.00112712;
        // TODO: Use value from RimReservoirCellResultsStorage?       

        skinfactor = m_fracture->attachedFractureDefinition()->skinFactor;

        double slDivPi = (skinfactor * fractureAreaWeightedlength) / cvf::PI_D;

        transmissibility_X = 8 * c * (permY * NTG) * Ay / (dx + slDivPi);
        transmissibility_Y = 8 * c * (permX * NTG) * Ax / (dy + slDivPi);
        transmissibility_Z = 8 * c * permZ * Az / (dz + slDivPi);

        transmissibility = sqrt(transmissibility_X * transmissibility_X
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
void RigFractureTransCalc::computeUpscaledPropertyFromStimPlanForEclipseCell(double &upscaledAritmStimPlanValue, double &upscaledHarmStimPlanValue, QString resultName, QString resultUnit, size_t timeStepIndex, caf::AppEnum< RimDefines::UnitSystem > unitSystem, size_t cellIndex)
{
    //TODO: A lot of common code with function for calculating transmissibility... 

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


    bool cellIsActive = activeCellInfo->isActive(cellIndex);

    cvf::Vec3d localX;
    cvf::Vec3d localY;
    cvf::Vec3d localZ;
    std::vector<std::vector<cvf::Vec3d> > planeCellPolygons;
    bool isPlanIntersected = planeCellIntersectionPolygons(cellIndex, planeCellPolygons, localX, localY, localZ);
    if (!isPlanIntersected || planeCellPolygons.size() == 0) return;

    //Transform planCell polygon(s) and averageZdirection to x/y coordinate system (where fracturePolygon/stimPlan mesh already is located)
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

    std::vector<cvf::Vec3f> fracPolygon = m_fracture->attachedFractureDefinition()->fracturePolygon(unitSystem);
    std::vector<std::vector<cvf::Vec3d> > polygonsDescribingFractureInCell;

    double area;
    std::vector<double> areaOfFractureParts;
    std::vector<double> valuesForFractureParts;

    for (std::vector<cvf::Vec3d> planeCellPolygon : planeCellPolygons)
    {

        for (int i = 0; i < stimPlanParameterValues.size(); i++)
        {
            double stimPlanParameterValue = stimPlanParameterValues[i];
            if (stimPlanParameterValue != 0)
            {
                std::vector<cvf::Vec3d> stimPlanCell = stimPlanCellsAsPolygons[i];
                std::vector<std::vector<cvf::Vec3d> >clippedStimPlanPolygons = RigCellGeometryTools::clipPolygons(stimPlanCell, planeCellPolygon);
                if (clippedStimPlanPolygons.size() > 0)
                {
                    for (auto clippedStimPlanPolygon : clippedStimPlanPolygons)
                    {
                        area = cvf::GeometryTools::polygonAreaNormal3D(clippedStimPlanPolygon).length();
                        areaOfFractureParts.push_back(area);
                        valuesForFractureParts.push_back(stimPlanParameterValue);
                    }
                }
            }
        }
    }

    if (areaOfFractureParts.size() > 0)
    {
        upscaledAritmStimPlanValue = areaWeightedArithmeticAverage(areaOfFractureParts, valuesForFractureParts);
        upscaledHarmStimPlanValue = areaWeightedHarmonicAverage(areaOfFractureParts, valuesForFractureParts);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransCalc::areaWeightedHarmonicAverage(std::vector<double> areaOfFractureParts, std::vector<double> valuesForFractureParts)
{
    //TODO: Unit test?
    double fractureCellArea = 0.0;
    for (double area : areaOfFractureParts) fractureCellArea += area;

    if (areaOfFractureParts.size() != valuesForFractureParts.size()) return cvf::UNDEFINED_DOUBLE;

    double fractureCellAreaDivvalue = 0.0;
    for (int i = 0; i < valuesForFractureParts.size(); i++)
    {
        fractureCellAreaDivvalue += areaOfFractureParts[i] / valuesForFractureParts[i];
    }

    double upscaledValueHarmonic = fractureCellArea / fractureCellAreaDivvalue;
    return upscaledValueHarmonic;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigFractureTransCalc::areaWeightedArithmeticAverage(std::vector<double> areaOfFractureParts, std::vector<double> valuesForFractureParts)
{
    //TODO: Unit test?
    double fractureCellArea = 0.0;
    for (double area : areaOfFractureParts) fractureCellArea += area;

    if (areaOfFractureParts.size() != valuesForFractureParts.size()) return cvf::UNDEFINED_DOUBLE;

    double fractureCellAreaXvalue = 0.0;
    for (int i = 0; i < valuesForFractureParts.size(); i++)
    {
        fractureCellAreaXvalue += areaOfFractureParts[i] * valuesForFractureParts[i];
    }

    double upscaledValueArithmetic = fractureCellAreaXvalue / fractureCellArea;
    return upscaledValueArithmetic;

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

        double upscaledAritmStimPlanValue = cvf::UNDEFINED_DOUBLE;
        double upscaledHarmStimPlanValue = cvf::UNDEFINED_DOUBLE;
        computeUpscaledPropertyFromStimPlanForEclipseCell(upscaledAritmStimPlanValue, upscaledHarmStimPlanValue, resultName, resultUnit, timeStepIndex, m_unitForCalculation, fracCell);

        if (upscaledAritmStimPlanValue != cvf::UNDEFINED_DOUBLE)
        {
            fracData.upscaledAritmStimPlanValue = upscaledAritmStimPlanValue;
            fracData.upscaledHarmStimPlanValue = upscaledHarmStimPlanValue;

            fracDataVec.push_back(fracData);
        }
    }

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
            double upscaledAritmStimPlanValue = cvf::UNDEFINED_DOUBLE;
            double upscaledHarmStimPlanValue = cvf::UNDEFINED_DOUBLE;
            caf::AppEnum< RimDefines::UnitSystem > unitSystem = RimDefines::UNITS_METRIC;
            computeUpscaledPropertyFromStimPlanForEclipseCell(upscaledAritmStimPlanValue, upscaledHarmStimPlanValue, resultName, resultUnit, timeStepIndex, unitSystem, fracCell);
            Kw = (upscaledAritmStimPlanValue + upscaledHarmStimPlanValue) / 2;
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

