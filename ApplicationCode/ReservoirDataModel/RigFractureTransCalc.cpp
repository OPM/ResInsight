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
#include "RigFractureCell.h"
#include <cmath> //Used for log 
#include "RiaApplication.h"
#include "RimEclipseView.h"
#include "RigFractureTransmissibilityEquations.h"

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
std::vector<RigFracturedEclipseCellExportData>  RigFractureTransCalc::computeTransmissibilityFromPolygonWithInfiniteConductivityInFracture()
{
    if (m_fracture->attachedFractureDefinition()->fractureConductivity == RimFractureTemplate::FINITE_CONDUCTIVITY)
    {
        RiaLogging::warning(QString("Transimssibility for finite conductity in fracture not yet implemented."));
        RiaLogging::warning(QString("Performing calculation for infinite conductivity instead."));
    }

    RigEclipseCaseData* eclipseCaseData = m_case->eclipseCaseData();
    double cDarchy = eclipseCaseData->darchysValue();

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

    std::vector<RigFracturedEclipseCellExportData> fracDataVec;
    std::vector<size_t> fracCells = m_fracture->getPotentiallyFracturedCells(eclipseCaseData->mainGrid());

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

        const RigMainGrid* mainGrid = m_case->eclipseCaseData()->mainGrid();
        cvf::Vec3d hexCorners[8];
        mainGrid->cellCornerVertices(fracCell, hexCorners);

        std::vector<std::vector<cvf::Vec3d> > planeCellPolygons;
        bool isPlanIntersected = planeCellIntersectionPolygons(hexCorners, m_fracture->transformMatrix(), planeCellPolygons);
        if (!isPlanIntersected || planeCellPolygons.size() == 0) continue;

        cvf::Vec3d localX;
        cvf::Vec3d localY;
        cvf::Vec3d localZ;
        RigCellGeometryTools::findCellLocalXYZ(hexCorners, localX, localY, localZ);

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

        RigFracturedEclipseCellExportData fracData;
        fracData.reservoirCellIndex = fracCell;

        std::vector<cvf::Vec3f> fracPolygon = m_fracture->attachedFractureDefinition()->fracturePolygon(m_unitForCalculation);

        std::vector<cvf::Vec3d> fracPolygonDouble;
        for (auto v : fracPolygon) fracPolygonDouble.push_back(static_cast<cvf::Vec3d>(v));

        std::vector<std::vector<cvf::Vec3d> > polygonsDescribingFractureInCell;
        cvf::Vec3d areaVector;

        for (std::vector<cvf::Vec3d> planeCellPolygon : planeCellPolygons)
        {
            std::vector<std::vector<cvf::Vec3d> >clippedPolygons = RigCellGeometryTools::intersectPolygons(planeCellPolygon, fracPolygonDouble);
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

        double transmissibility_X = RigFractureTransmissibilityEquations::matrixToFractureTrans(permY, NTG, Ay, dx, skinfactor, fractureAreaWeightedlength, cDarchy);
        double transmissibility_Y = RigFractureTransmissibilityEquations::matrixToFractureTrans(permX, NTG, Ax, dy, skinfactor, fractureAreaWeightedlength, cDarchy);
        double transmissibility_Z = RigFractureTransmissibilityEquations::matrixToFractureTrans(permZ, 1.0, Az, dz, skinfactor, fractureAreaWeightedlength, cDarchy);

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

    return fracDataVec;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigFractureTransCalc::planeCellIntersectionPolygons(cvf::Vec3d hexCorners[8],
                                                         cvf::Mat4f transformMatrixForPlane, 
                                                         std::vector<std::vector<cvf::Vec3d> > & polygons)
{

    //Lage static func - input: transform-matrix for plan, hexcorners for celle

    bool isCellIntersected = false;

    cvf::Plane fracturePlane;
    fracturePlane.setFromPointAndNormal(static_cast<cvf::Vec3d>(transformMatrixForPlane.translation()),
        static_cast<cvf::Vec3d>(transformMatrixForPlane.col(2)));

    //Find line-segments where cell and fracture plane intersects
    std::list<std::pair<cvf::Vec3d, cvf::Vec3d > > intersectionLineSegments;

    isCellIntersected = RigCellGeometryTools::planeHexCellIntersection(hexCorners, fracturePlane, intersectionLineSegments);

    RigCellGeometryTools::createPolygonFromLineSegments(intersectionLineSegments, polygons);

    return isCellIntersected;
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

