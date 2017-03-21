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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// RigFractureData::RigFractureData()
// {
// 
// }

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFractureTransCalc::RigFractureTransCalc()
{
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
//TODO: Make static and move to another class
void RigFractureTransCalc::computeTransmissibility(RimEclipseCase* caseToApply, RimFracture* fracture)
{
    //Get correct unit system: 
    RigEclipseCaseData::UnitsType caseUnit = caseToApply->eclipseCaseData()->unitsType();
    RimDefines::UnitSystem unitForExport;

    if (caseUnit == RigEclipseCaseData::UNITS_METRIC)
    {
        RiaLogging::debug(QString("Calculating transmissibilities in metric units"));
        unitForExport = RimDefines::UNITS_METRIC;
    }
    else if (caseUnit == RigEclipseCaseData::UNITS_FIELD)
    {
        RiaLogging::debug(QString("Calculating transmissibilities in field units"));
        unitForExport = RimDefines::UNITS_FIELD;
    }
    else
    {
        RiaLogging::error(QString("Unit system for case not supported for fracture export."));
        return;
    }

    if (fracture->attachedFractureDefinition()->fractureConductivity == RimFractureTemplate::FINITE_CONDUCTIVITY)
    {
        RiaLogging::warning(QString("Transimssibility for finite conductity in fracture not yet implemented."));
        RiaLogging::warning(QString("Performing calculation for infinite conductivity instead."));
    }


    RigEclipseCaseData* eclipseCaseData = caseToApply->eclipseCaseData();

    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    RimReservoirCellResultsStorage* gridCellResults = caseToApply->results(porosityModel);

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
    std::vector<size_t> fracCells = fracture->getPotentiallyFracturedCells();

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
        bool isPlanIntersected = fracture->planeCellIntersectionPolygons(fracCell, planeCellPolygons, localX, localY, localZ);
        if (!isPlanIntersected || planeCellPolygons.size() == 0) continue;

        //Transform planCell polygon(s) and averageZdirection to x/y coordinate system (where fracturePolygon already is located)
        cvf::Mat4f invertedTransMatrix =  fracture->transformMatrix().getInverted();
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


        std::vector<cvf::Vec3f> fracPolygon = fracture->attachedFractureDefinition()->fracturePolygon(unitForExport);

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
            cvf::Mat4f m = fracture->transformMatrix();
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
        if (unitForExport == RimDefines::UNITS_METRIC) c = 0.00852702;
        if (unitForExport == RimDefines::UNITS_FIELD)  c = 0.00112712;
        // TODO: Use value from RimReservoirCellResultsStorage?       

        skinfactor = fracture->attachedFractureDefinition()->skinFactor;

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

    fracture->setFractureData(fracDataVec);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFractureTransCalc::computeUpscaledPropertyFromStimPlanForEclipseCell(double &upscaledAritmStimPlanValue, double &upscaledHarmStimPlanValue, RimFracture* fracture, RimEclipseCase* caseToApply, QString resultName, QString resultUnit, size_t timeStepIndex, caf::AppEnum< RimDefines::UnitSystem > unitSystem, size_t cellIndex)
{
    //TODO: A lot of common code with function for calculating transmissibility... 

    RimStimPlanFractureTemplate* fracTemplateStimPlan;
    if (dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition()))
    {
        fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition());
    }
    else return;

    //TODO: UNITS! 

    std::vector<std::vector<cvf::Vec3d> > stimPlanCellsAsPolygons;
    std::vector<double> stimPlanParameterValues;
    fracTemplateStimPlan->getStimPlanDataAsPolygonsAndValues(stimPlanCellsAsPolygons, stimPlanParameterValues, resultName, resultUnit, timeStepIndex);

    //TODO: A lot of common code with function above... Can be cleaned up...?
    std::vector<size_t> fracCells = fracture->getPotentiallyFracturedCells();


    RigEclipseCaseData* eclipseCaseData = caseToApply->eclipseCaseData();

    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    RimReservoirCellResultsStorage* gridCellResults = caseToApply->results(porosityModel);
    RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(porosityModel);


    bool cellIsActive = activeCellInfo->isActive(cellIndex);

    cvf::Vec3d localX;
    cvf::Vec3d localY;
    cvf::Vec3d localZ;
    std::vector<std::vector<cvf::Vec3d> > planeCellPolygons;
    bool isPlanIntersected = fracture->planeCellIntersectionPolygons(cellIndex, planeCellPolygons, localX, localY, localZ);
    if (!isPlanIntersected || planeCellPolygons.size() == 0) return;

    //Transform planCell polygon(s) and averageZdirection to x/y coordinate system (where fracturePolygon/stimPlan mesh already is located)
    cvf::Mat4f invertedTransMatrix = fracture->transformMatrix().getInverted();
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

    std::vector<cvf::Vec3f> fracPolygon = fracture->attachedFractureDefinition()->fracturePolygon(unitSystem);
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
void RigFractureTransCalc::computeUpscaledPropertyFromStimPlan(RimEclipseCase* caseToApply, RimFracture* fracture, QString resultName, QString resultUnit, size_t timeStepIndex)
{

    //TODO: A lot of common code with function for calculating transmissibility... 

    RimStimPlanFractureTemplate* fracTemplateStimPlan;
    if (dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition()))
    {
        fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition());
    }
    else return;

    //Get correct unit system: 
    RigEclipseCaseData::UnitsType caseUnit = caseToApply->eclipseCaseData()->unitsType();
    RimDefines::UnitSystem unitForExport;

    if (caseUnit == RigEclipseCaseData::UNITS_METRIC)
    {
        RiaLogging::debug(QString("Calculating upscaled stimPlan values in metric units"));
        unitForExport = RimDefines::UNITS_METRIC;
    }
    else if (caseUnit == RigEclipseCaseData::UNITS_FIELD)
    {
        RiaLogging::debug(QString("Calculating upscaled stimPlan values in field units"));
        unitForExport = RimDefines::UNITS_FIELD;
    }
    else
    {
        RiaLogging::error(QString("Unit system for case not supported for fracture export."));
        return;
    }

    std::vector<std::vector<cvf::Vec3d> > stimPlanCellsAsPolygons;
    std::vector<double> stimPlanParameterValues;
    fracTemplateStimPlan->getStimPlanDataAsPolygonsAndValues(stimPlanCellsAsPolygons, stimPlanParameterValues, resultName, resultUnit, timeStepIndex);

    //TODO: A lot of common code with function above... Can be cleaned up...?
    std::vector<size_t> fracCells = fracture->getPotentiallyFracturedCells();

    RigEclipseCaseData* eclipseCaseData = caseToApply->eclipseCaseData();
    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    RimReservoirCellResultsStorage* gridCellResults = caseToApply->results(porosityModel);
    RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(porosityModel);

    std::vector<RigFractureData> fracDataVec;

    for (size_t fracCell : fracCells)
    {

        RigFractureData fracData;
        fracData.reservoirCellIndex = fracCell;

        double upscaledAritmStimPlanValue = cvf::UNDEFINED_DOUBLE;
        double upscaledHarmStimPlanValue = cvf::UNDEFINED_DOUBLE;
        caf::AppEnum< RimDefines::UnitSystem > unitSystem = RimDefines::UNITS_METRIC;
        computeUpscaledPropertyFromStimPlanForEclipseCell(upscaledAritmStimPlanValue, upscaledHarmStimPlanValue, fracture, caseToApply, resultName, resultUnit, timeStepIndex, unitSystem, fracCell);

        if (upscaledAritmStimPlanValue != cvf::UNDEFINED_DOUBLE)
        {
            fracData.upscaledAritmStimPlanValue = upscaledAritmStimPlanValue;
            fracData.upscaledHarmStimPlanValue = upscaledHarmStimPlanValue;

            fracDataVec.push_back(fracData);
        }
    }

    fracture->setFractureData(fracDataVec);



}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFractureTransCalc::computeFlowInFracture(RimEclipseCase* caseToApply, RimFracture* fracture)
{

    //TODO: A lot of common code with function for calculating transmissibility... 

    RimStimPlanFractureTemplate* fracTemplateStimPlan;
    RimEllipseFractureTemplate* fracTemplateEllipse;
    if (dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition()))
    {
        fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracture->attachedFractureDefinition());
    }
    else if (dynamic_cast<RimEllipseFractureTemplate*>(fracture->attachedFractureDefinition()))
    {
        fracTemplateEllipse = dynamic_cast<RimEllipseFractureTemplate*>(fracture->attachedFractureDefinition());
    }
    else return;

    //Get correct unit system: 
    RigEclipseCaseData::UnitsType caseUnit = caseToApply->eclipseCaseData()->unitsType();
    RimDefines::UnitSystem unitForExport;

    if (caseUnit == RigEclipseCaseData::UNITS_METRIC)
    {
        RiaLogging::debug(QString("Calculating flow in fracture in metric units"));
        unitForExport = RimDefines::UNITS_METRIC;
    }
    else if (caseUnit == RigEclipseCaseData::UNITS_FIELD)
    {
        RiaLogging::debug(QString("Calculating flow in fracture in field units"));
        unitForExport = RimDefines::UNITS_FIELD;
    }
    else
    {
        RiaLogging::error(QString("Unit system for case not supported for fracture export."));
        return;
    }


    //TODO: A lot of common code with function above... Can be cleaned up...?
    std::vector<size_t> fracCells = fracture->getPotentiallyFracturedCells();


    RigEclipseCaseData* eclipseCaseData = caseToApply->eclipseCaseData();

    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    RimReservoirCellResultsStorage* gridCellResults = caseToApply->results(porosityModel);
    RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(porosityModel);


    std::vector<RigFractureData> fracDataVec;

    for (size_t fracCell : fracCells)
    {
        double K; //Conductivity
        double w; //Fracture width

        if (fracTemplateEllipse)
        {
            //TODO: UNit handling...
            K = fracTemplateEllipse->fractureConductivity();
            w = fracTemplateEllipse->width();
        }
        else if (fracTemplateStimPlan)
        {
            QString resultName = "CONDUCTIVITY";
            QString resultUnit = "md-m";
            size_t timeStepIndex = 0; //TODO... 
            double upscaledAritmStimPlanValue = cvf::UNDEFINED_DOUBLE;
            double upscaledHarmStimPlanValue = cvf::UNDEFINED_DOUBLE;
            caf::AppEnum< RimDefines::UnitSystem > unitSystem = RimDefines::UNITS_METRIC;
            computeUpscaledPropertyFromStimPlanForEclipseCell(upscaledAritmStimPlanValue, upscaledHarmStimPlanValue, fracture, caseToApply, resultName, resultUnit, timeStepIndex, unitSystem, fracCell);
            K = (upscaledAritmStimPlanValue + upscaledHarmStimPlanValue) / 2;

            resultName = "WIDTH";
            resultUnit = "cm"; //TODO handle mm and cm!
            computeUpscaledPropertyFromStimPlanForEclipseCell(upscaledAritmStimPlanValue, upscaledHarmStimPlanValue, fracture, caseToApply, resultName, resultUnit, timeStepIndex, unitSystem, fracCell);
            w = (upscaledAritmStimPlanValue + upscaledHarmStimPlanValue) / 2;

        }

    }

    fracture->setFractureData(fracDataVec);

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFractureTransCalc::computeFlowIntoTransverseWell(RimEclipseCase* caseToApply, RimFracture* fracture)
{

    //TODO: A lot of common code with function for calculating transmissibility... 

    if (fracture->attachedFractureDefinition()->orientation == RimFractureTemplate::ALONG_WELL_PATH) return;

    double wellRadius = cvf::UNDEFINED_DOUBLE;
    if (fracture->attachedFractureDefinition()->orientation == RimFractureTemplate::TRANSVERSE_WELL_PATH)
    {
        wellRadius = 0.0;//TODO read this value...
    }
    if (fracture->attachedFractureDefinition()->orientation == RimFractureTemplate::AZIMUTH)
    {


    }



}

