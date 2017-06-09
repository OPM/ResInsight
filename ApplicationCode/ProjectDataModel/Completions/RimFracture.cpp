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
    CAF_PDM_InitField(&perforationLength, "PerforationLength", 1.0, "Perforation Length", "", "", "");
    CAF_PDM_InitField(&perforationEfficiency, "perforationEfficiency", 1.0, "perforation Efficiency", "", "", "");
    perforationEfficiency.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    CAF_PDM_InitField(&wellDiameter, "wellDiameter", 0.216, "Well Diameter at Fracture", "", "", "");
    CAF_PDM_InitField(&dip, "Dip", 0.0, "Dip", "", "", "");
    CAF_PDM_InitField(&tilt, "Tilt", 0.0, "Tilt", "", "", "");
    CAF_PDM_InitField(&showPolygonFractureOutline, "showPolygonFractureOutline", true, "Show Polygon Outline", "", "", "");
    CAF_PDM_InitField(&fractureUnit, "fractureUnit", caf::AppEnum<RimUnitSystem::UnitSystem>(RimUnitSystem::UNITS_METRIC), "Fracture Unit System", "", "", "");

    CAF_PDM_InitField(&stimPlanTimeIndexToPlot, "timeIndexToPlot", 0, "StimPlan Time Step", "", "", ""); 

    CAF_PDM_InitField(&m_anchorPosEclipseCellI, "I", 1, "Fracture location cell I", "", "", "");
    m_anchorPosEclipseCellI.uiCapability()->setUiHidden(true);
    
    CAF_PDM_InitField(&m_anchorPosEclipseCellJ, "J", 1, "Fracture location cell J", "", "", "");
    m_anchorPosEclipseCellJ.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_anchorPosEclipseCellK, "K", 1, "Fracture location cell K", "", "", "");
    m_anchorPosEclipseCellK.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_displayIJK, "Cell_IJK", "Cell IJK", "", "", "");
    m_displayIJK.registerGetMethod(this, &RimFracture::createOneBasedIJKText);
    m_displayIJK.uiCapability()->setUiReadOnly(true);

    m_rigFracture = new RigFracture;
    m_recomputeGeometry = true;

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
std::vector<size_t> RimFracture::getPotentiallyFracturedCells(const RigMainGrid* mainGrid)
{
    std::vector<size_t> cellindecies;
    if (!mainGrid) return cellindecies;

    if (!hasValidGeometry()) computeGeometry();

    const std::vector<cvf::Vec3f>& nodeCoordVec = nodeCoords();

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

    if (changedField == &fractureUnit)
    {
        if (fractureUnit == RimUnitSystem::UNITS_METRIC)
        {
            wellDiameter = RimUnitSystem::inchToMeter(wellDiameter);
            perforationLength = RimUnitSystem::feetToMeter(perforationLength);
        }
        else if (fractureUnit == RimUnitSystem::UNITS_FIELD)
        {
            wellDiameter = RimUnitSystem::meterToInch(wellDiameter);
            perforationLength = RimUnitSystem::meterToFeet(perforationLength);
        }
        this->updateConnectedEditors();
    }


    if (changedField == &m_fractureTemplate)
    {
        //perforationLength = m_fractureTemplate->perforationLength();
        //TODO: Find out if performationLength should be in RimFractureTemplate or in RimEllipseFracTemplate
        if (fractureTemplate()) azimuth = m_fractureTemplate->azimuthAngle();
        else azimuth = 0.0;
        updateAzimuthFromFractureTemplate();

        RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(fractureTemplate());
        if (stimPlanFracTemplate)
        {
            stimPlanTimeIndexToPlot = stimPlanFracTemplate->activeTimeStepIndex;
        }
    }

    if (changedField == &azimuth || 
        changedField == &m_fractureTemplate ||
        changedField == &stimPlanTimeIndexToPlot ||
        changedField == this->objectToggleField() ||
        changedField == &showPolygonFractureOutline ||
        changedField == &fractureUnit ||
        changedField == &dip ||
        changedField == &tilt)
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
cvf::BoundingBox RimFracture::boundingBoxInDomainCoords()
{
    cvf::BoundingBox bb;

    for (auto coord : nodeCoords())
    {
        bb.add(coord);
    }

    return bb;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::computeGeometry()
{
    std::vector<cvf::Vec3f> nodeCoords;
    std::vector<cvf::uint>  triangleIndices;

    RimFractureTemplate* fractureDef = fractureTemplate();
    if (fractureDef )
    {
        fractureDef->fractureTriangleGeometry(&nodeCoords, &triangleIndices, fractureUnit());
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
double RimFracture::wellRadius() const
{
    if (fractureUnit == RimUnitSystem::UNITS_METRIC)
    {
        return wellDiameter / 2;
    }
    else if (fractureUnit == RimUnitSystem::UNITS_FIELD)
    {
        return RimUnitSystem::inchToFeet(wellDiameter / 2);
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
cvf::Mat4f RimFracture::transformMatrix() const
{
    cvf::Vec3d center = anchorPosition();

    // Dip (in XY plane)
    cvf::Mat4f dipRotation = cvf::Mat4f::fromRotation(cvf::Vec3f::Z_AXIS, cvf::Math::toRadians(dip()));

    // Dip (out of XY plane)
    cvf::Mat4f tiltRotation = cvf::Mat4f::fromRotation(cvf::Vec3f::X_AXIS, cvf::Math::toRadians(tilt()));


    // Ellipsis geometry is produced in XY-plane, rotate 90 deg around X to get zero azimuth along Y
    cvf::Mat4f rotationFromTesselator = cvf::Mat4f::fromRotation(cvf::Vec3f::X_AXIS, cvf::Math::toRadians(90.0f));
    
    // Azimuth rotation
    cvf::Mat4f azimuthRotation = cvf::Mat4f::fromRotation(cvf::Vec3f::Z_AXIS, cvf::Math::toRadians(-azimuth()-90));

    cvf::Mat4f m = azimuthRotation * rotationFromTesselator * dipRotation * tiltRotation;
    m.setTranslation(cvf::Vec3f(center));

    return m;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::setRecomputeGeometryFlag()
{
    m_recomputeGeometry = true;

    m_fracturePartMgr->clearGeometryCache();
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
QString RimFracture::createOneBasedIJKText() const
{
    return QString("Cell : [%1, %2, %3]").arg(m_anchorPosEclipseCellI + 1).arg(m_anchorPosEclipseCellJ + 1).arg(m_anchorPosEclipseCellK + 1);
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
    if (fractureUnit == RimUnitSystem::UNITS_METRIC)
    {
        wellDiameter.uiCapability()->setUiName("Well Diameter [m]");
        perforationLength.uiCapability()->setUiName("Perforation Length [m]");
    }
    else if (fractureUnit == RimUnitSystem::UNITS_FIELD)
    {
        wellDiameter.uiCapability()->setUiName("Well Diameter [inches]");
        perforationLength.uiCapability()->setUiName("Perforation Length [Ft]");
    }

    if (fractureTemplate())
    {
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

        cvf::BoundingBox pointBBox;
        pointBBox.add(m_anchorPosition);

        mainGrid->findIntersectingCells(pointBBox, &cellindecies);

        if (cellindecies.size() > 0)
        {
            size_t i = cvf::UNDEFINED_SIZE_T;
            size_t j = cvf::UNDEFINED_SIZE_T;
            size_t k = cvf::UNDEFINED_SIZE_T;

            size_t gridCellIndex = cellindecies[0];

            if (mainGrid->ijkFromCellIndex(gridCellIndex, &i, &j, &k))
            {
                m_anchorPosEclipseCellI = static_cast<int>(i);
                m_anchorPosEclipseCellJ = static_cast<int>(j);
                m_anchorPosEclipseCellK = static_cast<int>(k);
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

    RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(fractureTemplate);
    if (stimPlanFracTemplate)
    {
        stimPlanTimeIndexToPlot = stimPlanFracTemplate->activeTimeStepIndex;
    }

    this->updateAzimuthFromFractureTemplate();
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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFracture::hasValidGeometry() const
{
    if (m_recomputeGeometry) return false;

    return (nodeCoords().size() > 0 && triangleIndices().size() > 0);
}

