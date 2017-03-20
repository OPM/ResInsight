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

#include "RimFracture.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RifReaderInterface.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigFracture.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigTesselatorTools.h"

#include "RimDefines.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimView.h"

#include "RivWellFracturePartMgr.h"

#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"
#include "cvfMatrix4.h"
#include "cvfPlane.h"

#include "clipper/clipper.hpp"
#include <math.h>

#include <QDebug>
#include <QString>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimFracture, "Fracture");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFracture::RimFracture(void)
{
    CAF_PDM_InitObject("Fracture", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_fractureTemplate, "FractureDef", "Fracture Template", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_anchorPosition, "anchorPosition", "Anchor Position", "", "", "");
    m_anchorPosition.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_uiAnchorPosition, "ui_positionAtWellpath", "Fracture Position", "", "", "");
    m_uiAnchorPosition.registerGetMethod(this, &RimFracture::fracturePositionForUi);
    m_uiAnchorPosition.uiCapability()->setUiReadOnly(true);
    CAF_PDM_InitField(&azimuth, "Azimuth", 0.0, "Azimuth", "", "", "");
    azimuth.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    CAF_PDM_InitField(&perforationLength, "PerforationLength", 0.0, "Perforation Length", "", "", "");
    CAF_PDM_InitField(&dip, "Dip", 0.0, "Dip", "", "", "");
    CAF_PDM_InitField(&showPolygonFractureOutline, "showPolygonFractureOutline", true, "Show Polygon Outline", "", "", "");
    CAF_PDM_InitField(&fractureUnit, "fractureUnit", caf::AppEnum<RimDefines::UnitSystem>(RimDefines::UNITS_METRIC), "Fracture Unit System", "", "", "");

    CAF_PDM_InitField(&stimPlanTimeIndexToPlot, "timeIndexToPlot", 0, "StimPlan Time Step", "", "", ""); 

    CAF_PDM_InitField(&m_i, "I", 1, "Fracture location cell I", "", "", "");
    m_i.uiCapability()->setUiHidden(true);
    
    CAF_PDM_InitField(&m_j, "J", 1, "Fracture location cell J", "", "", "");
    m_j.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_k, "K", 1, "Fracture location cell K", "", "", "");
    m_k.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_displayIJK, "Cell_IJK", "Cell IJK", "", "", "");
    m_displayIJK.registerGetMethod(this, &RimFracture::createOneBasedIJK);
    m_displayIJK.uiCapability()->setUiReadOnly(true);

    m_rigFracture = new RigFracture;
    m_recomputeGeometry = true;

    m_rivFracture = new RivWellFracturePartMgr(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFracture::~RimFracture()
{
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::uint>& RimFracture::triangleIndices() const
{
    return m_rigFracture->triangleIndices();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3f>& RimFracture::nodeCoords() const
{
    return m_rigFracture->nodeCoords();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimFracture::getPotentiallyFracturedCells()
{
    std::vector<size_t> cellindecies;

    RiaApplication* app = RiaApplication::instance();
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return cellindecies;
 
    RimEclipseView* activeRiv = dynamic_cast<RimEclipseView*>(activeView);
    if (!activeRiv) return cellindecies;

    const RigMainGrid* mainGrid = activeRiv->mainGrid();
    if (!mainGrid) return cellindecies;

    const std::vector<cvf::Vec3f>& nodeCoordVec = nodeCoords();

    if (!hasValidGeometry()) computeGeometry();

    cvf::BoundingBox polygonBBox;
    for (cvf::Vec3f nodeCoord : nodeCoordVec) polygonBBox.add(nodeCoord);

    mainGrid->findIntersectingCells(polygonBBox, &cellindecies);

    return cellindecies;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_fractureTemplate)
    {
        //perforationLength = m_fractureTemplate->perforationLength();
        //TODO: Find out if performationLength should be in RimFractureTemplate or in RimEllipseFracTemplate
        if (attachedFractureDefinition()) azimuth = m_fractureTemplate->azimuthAngle();
        else azimuth = 0.0;
        updateAzimuthFromFractureDefinition();

        RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(attachedFractureDefinition());
        if (stimPlanFracTemplate)
        {
            stimPlanTimeIndexToPlot = static_cast<int>(stimPlanFracTemplate->getStimPlanTimeValues().size() - 1);
        }
    }

    if (changedField == &azimuth || 
        changedField == &m_fractureTemplate ||
        changedField == &stimPlanTimeIndexToPlot ||
        changedField == this->objectToggleField() ||
        changedField == &showPolygonFractureOutline ||
        changedField == &fractureUnit ||
        changedField == &dip)
    {

        setRecomputeGeometryFlag();

        RimView* rimView = nullptr;
        this->firstAncestorOrThisOfType(rimView);
        if (rimView)
        {
            rimView->createDisplayModelAndRedraw();
        }
        else
        {
            // Can be triggered from well path, find active view
            RimView* activeView = RiaApplication::instance()->activeReservoirView();
            if (activeView)
            {
                activeView->createDisplayModelAndRedraw();
            }
        }

    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFracture::fracturePosition() const
{
    return m_anchorPosition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::computeGeometry()
{
    std::vector<cvf::Vec3f> nodeCoords;
    std::vector<cvf::uint>  triangleIndices;

    RimFractureTemplate* fractureDef = attachedFractureDefinition();
    if (fractureDef )
    {
        fractureDef->fractureGeometry(&nodeCoords, &triangleIndices, fractureUnit);
    }

    cvf::Mat4f m = transformMatrix();

    for (cvf::Vec3f& v : nodeCoords)
    {
        v.transformPoint(m);
    }

    m_rigFracture->setGeometry(triangleIndices, nodeCoords); 

    m_recomputeGeometry = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFracture::anchorPosition()
{
    return m_anchorPosition();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Mat4f RimFracture::transformMatrix()
{
    cvf::Vec3d center = anchorPosition();

    // Dip (in XY plane)
    cvf::Mat4f dipRotation = cvf::Mat4f::fromRotation(cvf::Vec3f::Z_AXIS, cvf::Math::toRadians(dip()));

    // Ellipsis geometry is produced in XY-plane, rotate 90 deg around X to get zero azimuth along Y
    cvf::Mat4f rotationFromTesselator = cvf::Mat4f::fromRotation(cvf::Vec3f::X_AXIS, cvf::Math::toRadians(90.0f));
    
    // Azimuth rotation
    cvf::Mat4f azimuthRotation = cvf::Mat4f::fromRotation(cvf::Vec3f::Z_AXIS, cvf::Math::toRadians(-azimuth()-90));

    cvf::Mat4f m = azimuthRotation * rotationFromTesselator * dipRotation;
    m.setTranslation(cvf::Vec3f(center));

    return m;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::computeTransmissibility(RimEclipseCase* caseToApply)
{ 
    //Get correct unit system: 
    RigEclipseCaseData::UnitsType caseUnit = caseToApply->eclipseCaseData()->unitsType();
    RimDefines::UnitSystem unitForExport;

    if (caseUnit == RigEclipseCaseData::UNITS_METRIC)
    {
        RiaLogging::debug(QString("Calculating transmissibilities for %1 in metric units").arg(name()));
        unitForExport = RimDefines::UNITS_METRIC;
    }
    else if (caseUnit == RigEclipseCaseData::UNITS_FIELD)
    {
        RiaLogging::debug(QString("Calculating transmissibilities for %1 in field units").arg(name()));
        unitForExport = RimDefines::UNITS_FIELD;
    }
    else
    {
        RiaLogging::error(QString("Unit system for case not supported for fracture export."));
        return;
    }


    //TODO: Handle case with finite conductivity in fracture
    if (attachedFractureDefinition())
    {
        if (attachedFractureDefinition()->fractureConductivity == RimFractureTemplate::FINITE_CONDUCTIVITY)
        {
            qDebug() << "Transimssibility for finite conductity in fracture not yet implemented.";
            qDebug() << "Performing calculation for infinite conductivity instead.";
        }
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

    std::vector<size_t> fracCells = getPotentiallyFracturedCells();
    
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
        if (!isPlanIntersected || planeCellPolygons.size()==0) continue;

        //Transform planCell polygon(s) and averageZdirection to x/y coordinate system (where fracturePolygon already is located)
        cvf::Mat4f invertedTransMatrix = transformMatrix().getInverted();
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

        if (attachedFractureDefinition())
        {
            std::vector<cvf::Vec3f> fracPolygon = attachedFractureDefinition()->fracturePolygon(unitForExport);

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
                cvf::Mat4f m = transformMatrix();
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

            skinfactor = attachedFractureDefinition()->skinFactor;
            
            double slDivPi = (skinfactor * fractureAreaWeightedlength) / cvf::PI_D;
            
            transmissibility_X = 8 * c * ( permY * NTG ) * Ay / (dx + slDivPi);
            transmissibility_Y = 8 * c * ( permX * NTG ) * Ax / (dy + slDivPi);
            transmissibility_Z = 8 * c * permZ * Az / (dz + slDivPi);
            
            transmissibility = sqrt(transmissibility_X * transmissibility_X
                              + transmissibility_Y * transmissibility_Y 
                              + transmissibility_Z * transmissibility_Z); 
        }
        else
        {
            transmissibility = cvf::UNDEFINED_DOUBLE;
        }

        
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

    m_rigFracture->setFractureData(fracDataVec);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::computeUpscaledPropertyFromStimPlan(RimEclipseCase* caseToApply)
{

    //TODO: A lot of common code with function for calculating transmissibility... 

    if (!attachedFractureDefinition()) return;
    
    RimStimPlanFractureTemplate* fracTemplateStimPlan;
    if (dynamic_cast<RimStimPlanFractureTemplate*>(attachedFractureDefinition()))
    {
        fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(attachedFractureDefinition());
    }
    else return;

    //Get correct unit system: 
    RigEclipseCaseData::UnitsType caseUnit = caseToApply->eclipseCaseData()->unitsType();
    RimDefines::UnitSystem unitForExport;

    if (caseUnit == RigEclipseCaseData::UNITS_METRIC)
    {
        RiaLogging::debug(QString("Calculating transmissibilities for %1 in metric units").arg(name()));
        unitForExport = RimDefines::UNITS_METRIC;
    }
    else if (caseUnit == RigEclipseCaseData::UNITS_FIELD)
    {
        RiaLogging::debug(QString("Calculating transmissibilities for %1 in field units").arg(name()));
        unitForExport = RimDefines::UNITS_FIELD;
    }
    else
    {
        RiaLogging::error(QString("Unit system for case not supported for fracture export."));
        return;
    }


    //TODO: Get these more generally: 
    QString resultName = "CONDUCTIVITY";
    QString resultUnit = "md-m";
    size_t timeStepIndex = 0;


    std::vector<std::vector<cvf::Vec3d> > stimPlanCellsAsPolygons;
    std::vector<double> stimPlanParameterValues;
    fracTemplateStimPlan->getStimPlanDataAsPolygonsAndValues(stimPlanCellsAsPolygons, stimPlanParameterValues, resultName, resultUnit, timeStepIndex);



    //TODO: A lot of common code with function above... Can be cleaned up...?
    std::vector<size_t> fracCells = getPotentiallyFracturedCells();
 

    RigEclipseCaseData* eclipseCaseData = caseToApply->eclipseCaseData();

    RifReaderInterface::PorosityModelResultType porosityModel = RifReaderInterface::MATRIX_RESULTS;
    RimReservoirCellResultsStorage* gridCellResults = caseToApply->results(porosityModel);
    RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(porosityModel);


    std::vector<RigFractureData> fracDataVec;

    for (size_t fracCell : fracCells)
    {

        if (fracCell == 168690)
        {
            qDebug() << "Test";
        }


        bool cellIsActive = activeCellInfo->isActive(fracCell);

        cvf::Vec3d localX;
        cvf::Vec3d localY;
        cvf::Vec3d localZ;
        std::vector<std::vector<cvf::Vec3d> > planeCellPolygons;
        bool isPlanIntersected = planeCellIntersectionPolygons(fracCell, planeCellPolygons, localX, localY, localZ);
        if (!isPlanIntersected || planeCellPolygons.size() == 0) continue;

        //Transform planCell polygon(s) and averageZdirection to x/y coordinate system (where fracturePolygon/stimPlan mesh already is located)
        cvf::Mat4f invertedTransMatrix = transformMatrix().getInverted();
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



        std::vector<cvf::Vec3f> fracPolygon = attachedFractureDefinition()->fracturePolygon(unitForExport);
        std::vector<std::vector<cvf::Vec3d> > polygonsDescribingFractureInCell;

        double area;
        std::vector<double> areaOfFractureParts;
        std::vector<double> areaXvalueOfFractureParts;


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
                            areaXvalueOfFractureParts.push_back(area*stimPlanParameterValue);
                        }
                    }
                }
            }


        }

        if (areaXvalueOfFractureParts.size() > 0)
        {
            double fractureCellArea = 0.0;
            for (double area : areaOfFractureParts) fractureCellArea += area;
            double fractureCellAreaXvalue = 0.0;
            for (double areaXvalue : areaXvalueOfFractureParts) fractureCellAreaXvalue += areaXvalue;
            double upscaledValueArithmetic = fractureCellAreaXvalue / fractureCellArea;

            fracData.upscaledStimPlanValue = upscaledValueArithmetic;
            fracData.cellIndex = fracCell;
            fracDataVec.push_back(fracData);
        }

        
    }

    m_rigFracture->setFractureData(fracDataVec);



}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFracture::planeCellIntersectionPolygons(size_t cellindex, std::vector<std::vector<cvf::Vec3d> > & polygons, 
                                                cvf::Vec3d & localX, cvf::Vec3d & localY, cvf::Vec3d & localZ)
{
    
    cvf::Plane fracturePlane;
    cvf::Mat4f m = transformMatrix();
    bool isCellIntersected = false;

    fracturePlane.setFromPointAndNormal(static_cast<cvf::Vec3d>(m.translation()), 
                                        static_cast<cvf::Vec3d>(m.col(2)));

    RiaApplication* app = RiaApplication::instance();
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return isCellIntersected;

    RimEclipseView* activeRiv = dynamic_cast<RimEclipseView*>(activeView);
    if (!activeRiv) return isCellIntersected;

    const RigMainGrid* mainGrid = activeRiv->mainGrid();
    if (!mainGrid) return isCellIntersected;

    RigCell cell = mainGrid->globalCellArray()[cellindex];
    if (cell.isInvalid()) return isCellIntersected;

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
void RimFracture::setRecomputeGeometryFlag()
{
    m_recomputeGeometry = true;

    m_rivFracture->clearGeometryCache();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFracture::isRecomputeGeometryFlagSet()
{
    return m_recomputeGeometry;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFracture::fracturePositionForUi() const
{
    cvf::Vec3d v = m_anchorPosition;

    v.z() = -v.z();

    return v;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimFracture::createOneBasedIJK() const
{
    return QString("Cell : [%1, %2, %3]").arg(m_i + 1).arg(m_j + 1).arg(m_k + 1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFracture::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    RimProject* proj = RiaApplication::instance()->project();
    CVF_ASSERT(proj);

    RimOilField* oilField = proj->activeOilField();
    if (oilField == nullptr) return options;

    if (fieldNeedingOptions == &m_fractureTemplate)
    {
        RimFractureTemplateCollection* fracDefColl = oilField->fractureDefinitionCollection();
        if (fracDefColl == nullptr) return options;

        for (RimFractureTemplate* fracDef : fracDefColl->fractureDefinitions())
        {
            options.push_back(caf::PdmOptionItemInfo(fracDef->name(), fracDef));
        }
    }
    else if (fieldNeedingOptions == &stimPlanTimeIndexToPlot)
    {
        if (attachedFractureDefinition())
        {
            RimFractureTemplate* fracTemplate = attachedFractureDefinition();
            if (dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate))
            {
                RimStimPlanFractureTemplate* fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate);
                std::vector<double> timeValues = fracTemplateStimPlan->getStimPlanTimeValues();
                int index = 0;
                for (double value : timeValues)
                {
                    options.push_back(caf::PdmOptionItemInfo(QString::number(value), index));
                    index++;
                }
            }

        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{

    if (attachedFractureDefinition())
    {
        if (attachedFractureDefinition()->orientation == RimFractureTemplate::ALONG_WELL_PATH
            || attachedFractureDefinition()->orientation == RimFractureTemplate::TRANSVERSE_WELL_PATH)
        {
            azimuth.uiCapability()->setUiReadOnly(true);
        }
        else if (attachedFractureDefinition()->orientation == RimFractureTemplate::AZIMUTH)
        {
            azimuth.uiCapability()->setUiReadOnly(false);
        }


        RimFractureTemplate* fracTemplate= attachedFractureDefinition();
        if (dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate))
        {
            stimPlanTimeIndexToPlot.uiCapability()->setUiHidden(false);
        }
        else
        {
            stimPlanTimeIndexToPlot.uiCapability()->setUiHidden(true);
        }
    }
    else
    {
        stimPlanTimeIndexToPlot.uiCapability()->setUiHidden(true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if (field == &azimuth)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_minimum = 0;
            myAttr->m_maximum = 360;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::setAnchorPosition(const cvf::Vec3d& pos)
{
    m_anchorPosition = pos;
    setRecomputeGeometryFlag();

    // Update ijk
    {
        std::vector<size_t> cellindecies;

        RiaApplication* app = RiaApplication::instance();
        RimView* activeView = RiaApplication::instance()->activeReservoirView();
        if (!activeView) return;

        RimEclipseView* activeRiv = dynamic_cast<RimEclipseView*>(activeView);
        if (!activeRiv) return;

        const RigMainGrid* mainGrid = activeRiv->mainGrid();
        if (!mainGrid) return;

        cvf::BoundingBox polygonBBox;
        polygonBBox.add(m_anchorPosition);

        mainGrid->findIntersectingCells(polygonBBox, &cellindecies);

        if (cellindecies.size() > 0)
        {
            size_t i = cvf::UNDEFINED_SIZE_T;
            size_t j = cvf::UNDEFINED_SIZE_T;
            size_t k = cvf::UNDEFINED_SIZE_T;

            size_t gridCellIndex = cellindecies[0];

            if (mainGrid->ijkFromCellIndex(gridCellIndex, &i, &j, &k))
            {
                m_i = static_cast<int>(i);
                m_j = static_cast<int>(j);
                m_k = static_cast<int>(k);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigFracture* RimFracture::attachedRigFracture() const
{
    CVF_ASSERT(m_rigFracture.notNull());

    return m_rigFracture.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::setFractureTemplate(RimFractureTemplate* fractureTemplate)
{
    m_fractureTemplate = fractureTemplate;

    RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(attachedFractureDefinition());
    if (stimPlanFracTemplate)
    {
        stimPlanTimeIndexToPlot = static_cast<int>(stimPlanFracTemplate->getStimPlanTimeValues().size() - 1);
    }

    this->updateAzimuthFromFractureDefinition();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate* RimFracture::attachedFractureDefinition() const
{
    return m_fractureTemplate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellFracturePartMgr* RimFracture::fracturePartManager()
{
    CVF_ASSERT(m_rivFracture.notNull());

    return m_rivFracture.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFracture::hasValidGeometry() const
{
    if (m_recomputeGeometry) return false;

    return (nodeCoords().size() > 0 && triangleIndices().size() > 0);
}

