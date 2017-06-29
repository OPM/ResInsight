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
#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"

#include "RifReaderInterface.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigTesselatorTools.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFractureTemplate.h"
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
#include "RigHexIntersectionTools.h"
#include "RimFractureContainment.h"

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
    
    CAF_PDM_InitField(&perforationLength, "PerforationLength", 1.0, "Perforation Length", "", "", "");
    CAF_PDM_InitField(&perforationEfficiency, "perforationEfficiency", 1.0, "perforation Efficiency", "", "", "");
    perforationEfficiency.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&wellDiameter, "wellDiameter", 0.216, "Well Diameter at Fracture", "", "", "");
    CAF_PDM_InitField(&dip, "Dip", 0.0, "Dip", "", "", "");
    CAF_PDM_InitField(&tilt, "Tilt", 0.0, "Tilt", "", "", "");
    CAF_PDM_InitField(&showPolygonFractureOutline, "showPolygonFractureOutline", true, "Show Polygon Outline", "", "", "");
    CAF_PDM_InitField(&m_fractureUnit, "fractureUnit", caf::AppEnum<RiaEclipseUnitTools::UnitSystem>(RiaEclipseUnitTools::UNITS_METRIC), "Fracture Unit System", "", "", "");
    m_fractureUnit.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&stimPlanTimeIndexToPlot, "timeIndexToPlot", 0, "StimPlan Time Step", "", "", ""); 

    CAF_PDM_InitFieldNoDefault(&m_uiWellPathAzimuth, "WellPathAzimuth", "Well Path Azimuth", "", "", "");
    m_uiWellPathAzimuth.registerGetMethod(this, &RimFracture::wellAzimuthAtFracturePosition);
    m_uiWellPathAzimuth.uiCapability()->setUiReadOnly(true);
    CAF_PDM_InitFieldNoDefault(&m_uiWellFractureAzimuthDiff, "WellFractureAzimuthDiff", "Azimuth Difference Between Fracture and Well", "", "", "");
    m_uiWellFractureAzimuthDiff.registerGetMethod(this, &RimFracture::wellFractureAzimuthDiff);
    m_uiWellFractureAzimuthDiff.uiCapability()->setUiReadOnly(true);
    CAF_PDM_InitField(&m_wellFractureAzimuthAngleWarning, "WellFractureAzimithAngleWarning", QString("Difference is below 10 degrees. Consider longitudinal fracture"), "", "", "", "");
    m_wellFractureAzimuthAngleWarning.uiCapability()->setUiReadOnly(true);

    m_fracturePartMgr = new RivWellFracturePartMgr(this);
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
std::vector<size_t> RimFracture::getPotentiallyFracturedCells(const RigMainGrid* mainGrid)
{
    std::vector<size_t> cellindecies;
    if (!mainGrid) return cellindecies;

    cvf::BoundingBox fractureBBox = this->boundingBoxInDomainCoords();

    mainGrid->findIntersectingCells(fractureBBox, &cellindecies);

    return cellindecies;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_fractureTemplate)
    {
        setFractureTemplate(m_fractureTemplate);
    }

    if (changedField == &azimuth || 
        changedField == &m_fractureTemplate ||
        changedField == &stimPlanTimeIndexToPlot ||
        changedField == this->objectToggleField() ||
        changedField == &showPolygonFractureOutline ||
        changedField == &dip ||
        changedField == &tilt)
    {

        clearDisplayGeometryCache();

        RimView* rimView = nullptr;
        this->firstAncestorOrThisOfType(rimView);
        if (rimView)
        {
            rimView->createDisplayModelAndRedraw();
        }
        else
        {
            // Can be triggered from well path, find active view
            RimProject* proj;
            this->firstAncestorOrThisOfTypeAsserted(proj);
            proj->reloadCompletionTypeResultsInAllViews();
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
double RimFracture::wellFractureAzimuthDiff() const
{
    double wellDifference = abs(wellAzimuthAtFracturePosition() - azimuth);
    return wellDifference;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimFracture::boundingBoxInDomainCoords()
{
    std::vector<cvf::Vec3f> nodeCoordVec;
    std::vector<cvf::uint> triangleIndices;

    this->triangleGeometry(&triangleIndices, &nodeCoordVec);

    cvf::BoundingBox fractureBBox;
    for (cvf::Vec3f nodeCoord : nodeCoordVec) fractureBBox.add(nodeCoord);
  
    return fractureBBox;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFracture::wellRadius(RiaEclipseUnitTools::UnitSystem unitSystem) const
{
    if (m_fractureUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        if (unitSystem == RiaEclipseUnitTools::UNITS_FIELD)
        {
            return RiaEclipseUnitTools::meterToFeet(wellDiameter / 2);
        }
        else
        {
            return wellDiameter / 2;
        }
    }
    else if (m_fractureUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        if (unitSystem == RiaEclipseUnitTools::UNITS_METRIC)
        {
            return RiaEclipseUnitTools::inchToMeter(wellDiameter / 2);
        }
        else
        {
            return RiaEclipseUnitTools::inchToFeet(wellDiameter / 2);
        }
    }
    return cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFracture::anchorPosition() const
{
    return m_anchorPosition();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Mat4d RimFracture::transformMatrix() const
{
    cvf::Vec3d center = anchorPosition();

    // Dip (in XY plane)
    cvf::Mat4d dipRotation = cvf::Mat4d::fromRotation(cvf::Vec3d::Z_AXIS, cvf::Math::toRadians(dip()));

    // Dip (out of XY plane)
    cvf::Mat4d tiltRotation = cvf::Mat4d::fromRotation(cvf::Vec3d::X_AXIS, cvf::Math::toRadians(tilt()));


    // Ellipsis geometry is produced in XY-plane, rotate 90 deg around X to get zero azimuth along Y
    cvf::Mat4d rotationFromTesselator = cvf::Mat4d::fromRotation(cvf::Vec3d::X_AXIS, cvf::Math::toRadians(90.0f));
    
    // Azimuth rotation
    cvf::Mat4d azimuthRotation = cvf::Mat4d::fromRotation(cvf::Vec3d::Z_AXIS, cvf::Math::toRadians(-azimuth()-90));

    cvf::Mat4d m = azimuthRotation * rotationFromTesselator * dipRotation * tiltRotation;
    m.setTranslation(center);

    return m;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::clearDisplayGeometryCache()
{
    m_fracturePartMgr->clearGeometryCache();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::triangleGeometry(std::vector<cvf::uint>* triangleIndices, std::vector<cvf::Vec3f>* nodeCoords)
{
        RimFractureTemplate* fractureDef = fractureTemplate();
        if (fractureDef )
        {
            fractureDef->fractureTriangleGeometry(nodeCoords, triangleIndices, fractureUnit());
        }

        cvf::Mat4d m = transformMatrix();

        for (cvf::Vec3f& v : *nodeCoords)
        {
            cvf::Vec3d vd(v);

            vd.transformPoint(m);

            v = cvf::Vec3f(vd);
        }
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
        if (fractureTemplate())
        {
            RimFractureTemplate* fracTemplate = fractureTemplate();
            if (dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate))
            {
                RimStimPlanFractureTemplate* fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate);
                std::vector<double> timeValues = fracTemplateStimPlan->timeSteps();
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
    if (m_fractureUnit() == RiaEclipseUnitTools::UNITS_METRIC)
    {
        wellDiameter.uiCapability()->setUiName("Well Diameter [m]");
        perforationLength.uiCapability()->setUiName("Perforation Length [m]");
    }
    else if (m_fractureUnit() == RiaEclipseUnitTools::UNITS_FIELD)
    {
        wellDiameter.uiCapability()->setUiName("Well Diameter [inches]");
        perforationLength.uiCapability()->setUiName("Perforation Length [Ft]");
    }

    if (fractureTemplate())
    {
        if (fractureTemplate()->orientationType == RimFractureTemplate::ALONG_WELL_PATH
            || fractureTemplate()->orientationType == RimFractureTemplate::TRANSVERSE_WELL_PATH)
        {
            m_uiWellPathAzimuth.uiCapability()->setUiHidden(true);
            m_uiWellFractureAzimuthDiff.uiCapability()->setUiHidden(true);
            m_wellFractureAzimuthAngleWarning.uiCapability()->setUiHidden(true);
        }

        else if (fractureTemplate()->orientationType == RimFractureTemplate::AZIMUTH)
        {
            m_uiWellPathAzimuth.uiCapability()->setUiHidden(false);
            m_uiWellFractureAzimuthDiff.uiCapability()->setUiHidden(false);
            if (wellFractureAzimuthDiff() < 10
                || (wellFractureAzimuthDiff() > 170 && wellFractureAzimuthDiff() < 190 ) 
                || wellFractureAzimuthDiff() > 350)
            {
                m_wellFractureAzimuthAngleWarning.uiCapability()->setUiHidden(false);
            }
            else
            {
                m_wellFractureAzimuthAngleWarning.uiCapability()->setUiHidden(true);
            }
        }

        if (fractureTemplate()->orientationType == RimFractureTemplate::ALONG_WELL_PATH
            || fractureTemplate()->orientationType == RimFractureTemplate::TRANSVERSE_WELL_PATH)
        {
            azimuth.uiCapability()->setUiReadOnly(true);
        }
        else if (fractureTemplate()->orientationType == RimFractureTemplate::AZIMUTH)
        {
            azimuth.uiCapability()->setUiReadOnly(false);
        }

        if (fractureTemplate()->orientationType == RimFractureTemplate::ALONG_WELL_PATH)
        {
            perforationEfficiency.uiCapability()->setUiHidden(false);
            perforationLength.uiCapability()->setUiHidden(false);
        }
        else
        {
            perforationEfficiency.uiCapability()->setUiHidden(true);
            perforationLength.uiCapability()->setUiHidden(true);
        }

        if (fractureTemplate()->conductivityType == RimFractureTemplate::FINITE_CONDUCTIVITY)
        {
            wellDiameter.uiCapability()->setUiHidden(false);
        }
        else if (fractureTemplate()->conductivityType == RimFractureTemplate::INFINITE_CONDUCTIVITY)
        {
            wellDiameter.uiCapability()->setUiHidden(true);
        }

        RimFractureTemplate* fracTemplate = fractureTemplate();
        if (dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate))
        {
            stimPlanTimeIndexToPlot.uiCapability()->setUiHidden(false);
            stimPlanTimeIndexToPlot.uiCapability()->setUiReadOnly(true);

            showPolygonFractureOutline.uiCapability()->setUiHidden(false);
        }
        else
        {
            stimPlanTimeIndexToPlot.uiCapability()->setUiHidden(true);

            showPolygonFractureOutline.uiCapability()->setUiHidden(true);
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

    if (field == &perforationEfficiency)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_minimum = 0;
            myAttr->m_maximum = 1.0;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::setAnchorPosition(const cvf::Vec3d& pos)
{
    m_anchorPosition = pos;
    clearDisplayGeometryCache();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystem RimFracture::fractureUnit() const
{
    return m_fractureUnit();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::setFractureUnit(RiaEclipseUnitTools::UnitSystem unitSystem)
{
    m_fractureUnit = unitSystem;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFracture::isEclipseCellWithinContainment(const RigMainGrid* mainGrid, size_t globalCellIndex) const
{
    CVF_ASSERT(fractureTemplate());
    if (!fractureTemplate()->fractureContainment()->isEnabled()) return true;

    size_t anchorEclipseCell = findAnchorEclipseCell(mainGrid);
    return fractureTemplate()->fractureContainment()->isEclipseCellWithinContainment(mainGrid, anchorEclipseCell, globalCellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimFracture::findAnchorEclipseCell(const RigMainGrid* mainGrid ) const
{
    cvf::BoundingBox pointBBox;
    pointBBox.add(m_anchorPosition);

    std::vector<size_t> cellIndices;
    mainGrid->findIntersectingCells(pointBBox, &cellIndices);
    
    size_t cellContainingAchorPoint = cvf::UNDEFINED_SIZE_T;

    for ( size_t cellIndex : cellIndices )
    {
        auto cornerIndices =  mainGrid->globalCellArray()[cellIndex].cornerIndices();
        cvf::Vec3d vertices[8];
        vertices[0] = (mainGrid->nodes()[cornerIndices[0]]);
        vertices[1] = (mainGrid->nodes()[cornerIndices[1]]);
        vertices[2] = (mainGrid->nodes()[cornerIndices[2]]);
        vertices[3] = (mainGrid->nodes()[cornerIndices[3]]);
        vertices[4] = (mainGrid->nodes()[cornerIndices[4]]);
        vertices[5] = (mainGrid->nodes()[cornerIndices[5]]);
        vertices[6] = (mainGrid->nodes()[cornerIndices[6]]);
        vertices[7] = (mainGrid->nodes()[cornerIndices[7]]);

        if ( RigHexIntersectionTools::isPointInCell(m_anchorPosition, vertices) )
        {
            cellContainingAchorPoint = cellIndex;
            break;
        }
    }    
    
    return cellContainingAchorPoint;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::setFractureTemplate(RimFractureTemplate* fractureTemplate)
{
    m_fractureTemplate = fractureTemplate;

    RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(fractureTemplate);
    if (stimPlanFracTemplate)
    {
        stimPlanTimeIndexToPlot = stimPlanFracTemplate->activeTimeStepIndex();
    }

    this->updateAzimuthFromFractureTemplate();
    this->wellDiameter = fractureTemplate->wellDiameterInFractureUnit(m_fractureUnit());
    this->perforationLength = fractureTemplate->perforationLengthInFractureUnit(m_fractureUnit());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate* RimFracture::fractureTemplate() const
{
    return m_fractureTemplate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellFracturePartMgr* RimFracture::fracturePartManager()
{
    CVF_ASSERT(m_fracturePartMgr.notNull());

    return m_fracturePartMgr.p();
}

