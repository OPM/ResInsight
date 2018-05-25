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

#include "RigEclipseToStimPlanCellTransmissibilityCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigFractureCell.h"
#include "RigFractureTransmissibilityEquations.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigHexIntersectionTools.h"

#include "RimEclipseCase.h"

#include "cvfGeometryTools.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigEclipseToStimPlanCellTransmissibilityCalculator::RigEclipseToStimPlanCellTransmissibilityCalculator(RimEclipseCase* caseToApply,
                                                                                                       cvf::Mat4d fractureTransform,
                                                                                                       double skinFactor,
                                                                                                       double cDarcy,
                                                                                                       const RigFractureCell& stimPlanCell)
    : m_case(caseToApply),
    m_fractureTransform(fractureTransform),
    m_fractureSkinFactor(skinFactor),
    m_cDarcy(cDarcy),
    m_stimPlanCell(stimPlanCell)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigEclipseToStimPlanCellTransmissibilityCalculator::globalIndiciesToContributingEclipseCells()
{
    if (m_globalIndiciesToContributingEclipseCells.size() < 1)
    {
        calculateStimPlanCellsMatrixTransmissibility();
    }

    return m_globalIndiciesToContributingEclipseCells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigEclipseToStimPlanCellTransmissibilityCalculator::contributingEclipseCellTransmissibilities()
{
    if (m_globalIndiciesToContributingEclipseCells.size() < 1)
    {
        calculateStimPlanCellsMatrixTransmissibility();
    }

    return m_contributingEclipseCellTransmissibilities;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseToStimPlanCellTransmissibilityCalculator::calculateStimPlanCellsMatrixTransmissibility()
{
    // Not calculating flow into fracture if stimPlan cell cond value is 0 (assumed to be outside the fracture):
    if (m_stimPlanCell.getConductivtyValue() < 1e-7) return;

    const RigEclipseCaseData* eclipseCaseData = m_case->eclipseCaseData();

    RiaDefines::PorosityModelType porosityModel = RiaDefines::MATRIX_MODEL;

    cvf::ref<RigResultAccessor> dataAccessObjectDx      = loadResultAndCreateResultAccessor(m_case, porosityModel, "DX");
    cvf::ref<RigResultAccessor> dataAccessObjectDy      = loadResultAndCreateResultAccessor(m_case, porosityModel, "DY");
    cvf::ref<RigResultAccessor> dataAccessObjectDz      = loadResultAndCreateResultAccessor(m_case, porosityModel, "DZ");
    cvf::ref<RigResultAccessor> dataAccessObjectPermX   = loadResultAndCreateResultAccessor(m_case, porosityModel, "PERMX");
    cvf::ref<RigResultAccessor> dataAccessObjectPermY   = loadResultAndCreateResultAccessor(m_case, porosityModel, "PERMY");
    cvf::ref<RigResultAccessor> dataAccessObjectPermZ   = loadResultAndCreateResultAccessor(m_case, porosityModel, "PERMZ");
    cvf::ref<RigResultAccessor> dataAccessObjectNTG     = loadResultAndCreateResultAccessor(m_case, porosityModel, "NTG");

    const RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(porosityModel);

    std::vector<cvf::Vec3d> stimPlanPolygonTransformed;
    for (cvf::Vec3d v : m_stimPlanCell.getPolygon())
    {
        v.transformPoint(m_fractureTransform);
        stimPlanPolygonTransformed.push_back(v);
    }

    std::vector<size_t> fracCells = getPotentiallyFracturedCellsForPolygon(stimPlanPolygonTransformed);
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

        double NTG = 1.0;
        if (dataAccessObjectNTG.notNull())
        {
            NTG = dataAccessObjectNTG->cellScalarGlobIdx(fracCell);
        }

        const RigMainGrid* mainGrid = m_case->eclipseCaseData()->mainGrid();
        std::array<cvf::Vec3d, 8> hexCorners;
        mainGrid->cellCornerVertices(fracCell, hexCorners.data());

        std::vector<std::vector<cvf::Vec3d> > planeCellPolygons;
        bool isPlanIntersected = RigHexIntersectionTools::planeHexIntersectionPolygons(hexCorners, m_fractureTransform, planeCellPolygons);
        if (!isPlanIntersected || planeCellPolygons.size() == 0) continue;

        cvf::Vec3d localX;
        cvf::Vec3d localY;
        cvf::Vec3d localZ;
        RigCellGeometryTools::findCellLocalXYZ(hexCorners, localX, localY, localZ);

        //Transform planCell polygon(s) and averageZdirection to x/y coordinate system (where fracturePolygon already is located)
        cvf::Mat4d invertedTransMatrix = m_fractureTransform.getInverted();
        for (std::vector<cvf::Vec3d> & planeCellPolygon : planeCellPolygons)
        {
            for (cvf::Vec3d& v : planeCellPolygon)
            {
                v.transformPoint(invertedTransMatrix);
            }
        }

        std::vector<std::vector<cvf::Vec3d> > polygonsForStimPlanCellInEclipseCell;
        cvf::Vec3d areaVector;
        std::vector<cvf::Vec3d> stimPlanPolygon = m_stimPlanCell.getPolygon();

        for (std::vector<cvf::Vec3d> planeCellPolygon : planeCellPolygons)
        {
            std::vector<std::vector<cvf::Vec3d> >clippedPolygons = RigCellGeometryTools::intersectPolygons(planeCellPolygon, stimPlanPolygon);
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

            length = RigCellGeometryTools::polygonLengthInLocalXdirWeightedByArea(fracturePartPolygon);
            lengthXareaOfFractureParts.push_back(length * area);

            cvf::Plane fracturePlane;
            fracturePlane.setFromPointAndNormal(static_cast<cvf::Vec3d>(m_fractureTransform.translation()),
                                                static_cast<cvf::Vec3d>(m_fractureTransform.col(2)));

            Ax += fabs(area*(fracturePlane.normal().dot(localY)));
            Ay += fabs(area*(fracturePlane.normal().dot(localX)));
            Az += fabs(area*(fracturePlane.normal().dot(localZ)));
        }

        double fractureArea = 0.0;
        for (double area : areaOfFractureParts) fractureArea += area;

        double totalAreaXLength = 0.0;
        for (double lengtXarea : lengthXareaOfFractureParts) totalAreaXLength += lengtXarea;

        double fractureAreaWeightedlength = totalAreaXLength / fractureArea;

        double transmissibility_X = RigFractureTransmissibilityEquations::matrixToFractureTrans(permY, NTG, Ay, dx, m_fractureSkinFactor, fractureAreaWeightedlength, m_cDarcy);
        double transmissibility_Y = RigFractureTransmissibilityEquations::matrixToFractureTrans(permX, NTG, Ax, dy, m_fractureSkinFactor, fractureAreaWeightedlength, m_cDarcy);
        double transmissibility_Z = RigFractureTransmissibilityEquations::matrixToFractureTrans(permZ, 1.0, Az, dz, m_fractureSkinFactor, fractureAreaWeightedlength, m_cDarcy);

        double transmissibility = sqrt(transmissibility_X * transmissibility_X
                                       + transmissibility_Y * transmissibility_Y
                                       + transmissibility_Z * transmissibility_Z);


        m_globalIndiciesToContributingEclipseCells.push_back(fracCell);
        m_contributingEclipseCellTransmissibilities.push_back(transmissibility);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigEclipseToStimPlanCellTransmissibilityCalculator::getPotentiallyFracturedCellsForPolygon(std::vector<cvf::Vec3d> polygon)
{
    std::vector<size_t> cellIndices;

    const RigMainGrid* mainGrid = m_case->eclipseCaseData()->mainGrid();
    if (!mainGrid) return cellIndices;

    cvf::BoundingBox polygonBBox;
    for (cvf::Vec3d nodeCoord : polygon) polygonBBox.add(nodeCoord);

    mainGrid->findIntersectingCells(polygonBBox, &cellIndices);

    return cellIndices;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigEclipseToStimPlanCellTransmissibilityCalculator::loadResultAndCreateResultAccessor(
    RimEclipseCase* eclipseCase,
    RiaDefines::PorosityModelType porosityModel,
    const QString& uiResultName)
{
    CVF_ASSERT(eclipseCase);

    RigCaseCellResultsData* gridCellResults = eclipseCase->results(porosityModel);

    // Calling this function will force loading of result from file
    gridCellResults->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, uiResultName);

    const RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    // Create result accessor object for main grid at time step zero (static result date is always at first time step
    return RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, uiResultName);
}
