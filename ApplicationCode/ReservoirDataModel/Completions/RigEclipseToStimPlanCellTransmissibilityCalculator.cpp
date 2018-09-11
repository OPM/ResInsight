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
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimFracture.h"

#include "RiaLogging.h"

#include "cvfGeometryTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseToStimPlanCellTransmissibilityCalculator::RigEclipseToStimPlanCellTransmissibilityCalculator(
    const RimEclipseCase*   caseToApply,
    cvf::Mat4d              fractureTransform,
    double                  skinFactor,
    double                  cDarcy,
    const RigFractureCell&  stimPlanCell,
    const std::set<size_t>& reservoirCellIndicesOpenForFlow,
    const RimFracture*      fracture)
    : m_case(caseToApply)
    , m_fractureTransform(fractureTransform)
    , m_fractureSkinFactor(skinFactor)
    , m_cDarcy(cDarcy)
    , m_stimPlanCell(stimPlanCell)
    , m_fracture(fracture)
{
    calculateStimPlanCellsMatrixTransmissibility(reservoirCellIndicesOpenForFlow);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigEclipseToStimPlanCellTransmissibilityCalculator::globalIndiciesToContributingEclipseCells() const
{
    return m_globalIndiciesToContributingEclipseCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigEclipseToStimPlanCellTransmissibilityCalculator::contributingEclipseCellTransmissibilities() const
{
    return m_contributingEclipseCellTransmissibilities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigEclipseToStimPlanCellTransmissibilityCalculator::contributingEclipseCellAreas() const
{
    return m_contributingEclipseCellAreas;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseToStimPlanCellTransmissibilityCalculator::areaOpenForFlow() const
{
    double area = 0.0;

    for (const auto& areaForOneEclipseCell : m_contributingEclipseCellAreas)
    {
        area += areaForOneEclipseCell;
    }

    return area;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseToStimPlanCellTransmissibilityCalculator::aggregatedMatrixTransmissibility() const
{
    double totalTransmissibility = 0.0;

    for (const auto& trans : m_contributingEclipseCellTransmissibilities)
    {
        totalTransmissibility += trans;
    }

    return totalTransmissibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFractureCell& RigEclipseToStimPlanCellTransmissibilityCalculator::fractureCell() const
{
    return m_stimPlanCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigEclipseToStimPlanCellTransmissibilityCalculator::requiredResultNames()
{
    std::vector<QString> resultNames;
    resultNames.push_back("PERMX");
    resultNames.push_back("PERMY");
    resultNames.push_back("PERMZ");

    resultNames.push_back("DX");
    resultNames.push_back("DY");
    resultNames.push_back("DZ");

    return resultNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigEclipseToStimPlanCellTransmissibilityCalculator::optionalResultNames()
{
    std::vector<QString> resultNames;
    resultNames.push_back("NTG");

    return resultNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseToStimPlanCellTransmissibilityCalculator::calculateStimPlanCellsMatrixTransmissibility(
    const std::set<size_t>& reservoirCellIndicesOpenForFlow)
{
    // Not calculating flow into fracture if stimPlan cell cond value is 0 (assumed to be outside the fracture):
    if (m_stimPlanCell.getConductivityValue() < 1e-7) return;

    const RigEclipseCaseData* eclipseCaseData = m_case->eclipseCaseData();

    RiaDefines::PorosityModelType porosityModel = RiaDefines::MATRIX_MODEL;

    cvf::ref<RigResultAccessor> dataAccessObjectDx = createResultAccessor(m_case, "DX");
    cvf::ref<RigResultAccessor> dataAccessObjectDy = createResultAccessor(m_case, "DY");
    cvf::ref<RigResultAccessor> dataAccessObjectDz = createResultAccessor(m_case, "DZ");
    if (dataAccessObjectDx.isNull() || dataAccessObjectDy.isNull() || dataAccessObjectDz.isNull())
    {
        RiaLogging::error("Data for DX/DY/DZ is not complete, and these values are required for export of COMPDAT. Make sure "
                          "'Preferences->Compute DEPTH Related Properties' is checked.");

        return;
    }

    cvf::ref<RigResultAccessor> dataAccessObjectPermX = createResultAccessor(m_case, "PERMX");
    cvf::ref<RigResultAccessor> dataAccessObjectPermY = createResultAccessor(m_case, "PERMY");
    cvf::ref<RigResultAccessor> dataAccessObjectPermZ = createResultAccessor(m_case, "PERMZ");
    if (dataAccessObjectPermX.isNull() || dataAccessObjectPermY.isNull() || dataAccessObjectPermZ.isNull())
    {
        RiaLogging::error("Data for PERMX/PERMY/PERMZ is not complete, and these values are required for export of COMPDAT.");

        return;
    }

    cvf::ref<RigResultAccessor> dataAccessObjectNTG = createResultAccessor(m_case, "NTG");

    const RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(porosityModel);

    std::vector<cvf::Vec3d> stimPlanPolygonTransformed;
    for (cvf::Vec3d v : m_stimPlanCell.getPolygon())
    {
        v.transformPoint(m_fractureTransform);
        stimPlanPolygonTransformed.push_back(v);
    }

    std::vector<size_t> reservoirCellIndices = getPotentiallyFracturedCellsForPolygon(stimPlanPolygonTransformed);
    for (size_t reservoirCellIndex : reservoirCellIndices)
    {
        const RigMainGrid* mainGrid = m_case->eclipseCaseData()->mainGrid();
        if (!m_fracture->isEclipseCellOpenForFlow(mainGrid, reservoirCellIndicesOpenForFlow, reservoirCellIndex)) continue;

        std::array<cvf::Vec3d, 8> hexCorners;
        mainGrid->cellCornerVertices(reservoirCellIndex, hexCorners.data());

        std::vector<std::vector<cvf::Vec3d>> planeCellPolygons;
        bool                                 isPlanIntersected =
            RigHexIntersectionTools::planeHexIntersectionPolygons(hexCorners, m_fractureTransform, planeCellPolygons);
        if (!isPlanIntersected || planeCellPolygons.empty()) continue;

        cvf::Vec3d localX;
        cvf::Vec3d localY;
        cvf::Vec3d localZ;
        RigCellGeometryTools::findCellLocalXYZ(hexCorners, localX, localY, localZ);

        // Transform planCell polygon(s) and averageZdirection to x/y coordinate system (where fracturePolygon already is located)
        cvf::Mat4d invertedTransMatrix = m_fractureTransform.getInverted();
        for (std::vector<cvf::Vec3d>& planeCellPolygon : planeCellPolygons)
        {
            for (cvf::Vec3d& v : planeCellPolygon)
            {
                v.transformPoint(invertedTransMatrix);
            }
        }

        std::vector<std::vector<cvf::Vec3d>> polygonsForStimPlanCellInEclipseCell;
        cvf::Vec3d                           areaVector;
        std::vector<cvf::Vec3d>              stimPlanPolygon = m_stimPlanCell.getPolygon();

        for (const std::vector<cvf::Vec3d>& planeCellPolygon : planeCellPolygons)
        {
            std::vector<std::vector<cvf::Vec3d>> clippedPolygons =
                RigCellGeometryTools::intersectPolygons(planeCellPolygon, stimPlanPolygon);
            for (const std::vector<cvf::Vec3d>& clippedPolygon : clippedPolygons)
            {
                polygonsForStimPlanCellInEclipseCell.push_back(clippedPolygon);
            }
        }

        if (polygonsForStimPlanCellInEclipseCell.empty()) continue;

        double              area;
        std::vector<double> areaOfFractureParts;
        double              length;
        std::vector<double> lengthXareaOfFractureParts;
        double              Ax = 0.0;
        double              Ay = 0.0;
        double              Az = 0.0;

        for (const std::vector<cvf::Vec3d>& fracturePartPolygon : polygonsForStimPlanCellInEclipseCell)
        {
            areaVector = cvf::GeometryTools::polygonAreaNormal3D(fracturePartPolygon);
            area       = areaVector.length();
            areaOfFractureParts.push_back(area);

            length = RigCellGeometryTools::polygonLengthInLocalXdirWeightedByArea(fracturePartPolygon);
            lengthXareaOfFractureParts.push_back(length * area);

            cvf::Plane fracturePlane;
            fracturePlane.setFromPointAndNormal(static_cast<cvf::Vec3d>(m_fractureTransform.translation()),
                                                static_cast<cvf::Vec3d>(m_fractureTransform.col(2)));

            Ax += fabs(area * (fracturePlane.normal().dot(localY)));
            Ay += fabs(area * (fracturePlane.normal().dot(localX)));
            Az += fabs(area * (fracturePlane.normal().dot(localZ)));
        }

        double fractureArea = 0.0;
        for (double area : areaOfFractureParts)
            fractureArea += area;

        double totalAreaXLength = 0.0;
        for (double lengtXarea : lengthXareaOfFractureParts)
            totalAreaXLength += lengtXarea;

        double fractureAreaWeightedlength = totalAreaXLength / fractureArea;

        // Transmissibility for inactive cells is set to zero
        double transmissibility = 0.0;

        if (activeCellInfo->isActive(reservoirCellIndex))
        {
            double permX = dataAccessObjectPermX->cellScalarGlobIdx(reservoirCellIndex);
            double permY = dataAccessObjectPermY->cellScalarGlobIdx(reservoirCellIndex);
            double permZ = dataAccessObjectPermZ->cellScalarGlobIdx(reservoirCellIndex);

            double dx = dataAccessObjectDx->cellScalarGlobIdx(reservoirCellIndex);
            double dy = dataAccessObjectDy->cellScalarGlobIdx(reservoirCellIndex);
            double dz = dataAccessObjectDz->cellScalarGlobIdx(reservoirCellIndex);

            double NTG = 1.0;
            if (dataAccessObjectNTG.notNull())
            {
                NTG = dataAccessObjectNTG->cellScalarGlobIdx(reservoirCellIndex);
            }

            double transmissibility_X = RigFractureTransmissibilityEquations::matrixToFractureTrans(
                permY, NTG, Ay, dx, m_fractureSkinFactor, fractureAreaWeightedlength, m_cDarcy);
            double transmissibility_Y = RigFractureTransmissibilityEquations::matrixToFractureTrans(
                permX, NTG, Ax, dy, m_fractureSkinFactor, fractureAreaWeightedlength, m_cDarcy);
            double transmissibility_Z = RigFractureTransmissibilityEquations::matrixToFractureTrans(
                permZ, 1.0, Az, dz, m_fractureSkinFactor, fractureAreaWeightedlength, m_cDarcy);

            transmissibility = sqrt(transmissibility_X * transmissibility_X + transmissibility_Y * transmissibility_Y +
                                    transmissibility_Z * transmissibility_Z);
        }

        m_globalIndiciesToContributingEclipseCells.push_back(reservoirCellIndex);
        m_contributingEclipseCellTransmissibilities.push_back(transmissibility);
        m_contributingEclipseCellAreas.push_back(fractureArea);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigEclipseToStimPlanCellTransmissibilityCalculator::getPotentiallyFracturedCellsForPolygon(
    const std::vector<cvf::Vec3d>& polygon) const
{
    std::vector<size_t> cellIndices;

    const RigMainGrid* mainGrid = m_case->eclipseCaseData()->mainGrid();
    if (!mainGrid) return cellIndices;

    cvf::BoundingBox polygonBBox;
    for (const cvf::Vec3d& nodeCoord : polygon)
    {
        polygonBBox.add(nodeCoord);
    }

    mainGrid->findIntersectingCells(polygonBBox, &cellIndices);

    return cellIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor>
    RigEclipseToStimPlanCellTransmissibilityCalculator::createResultAccessor(const RimEclipseCase* eclipseCase,
                                                                             const QString&        uiResultName)
{
    RiaDefines::PorosityModelType porosityModel   = RiaDefines::MATRIX_MODEL;
    const RigEclipseCaseData*     eclipseCaseData = eclipseCase->eclipseCaseData();

    // Create result accessor object for main grid at time step zero (static result date is always at first time step
    return RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, porosityModel, 0, uiResultName);
}
