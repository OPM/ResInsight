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
#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"

#include "RigMainGrid.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFractureContainment.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanFractureTemplate.h"

#include "RivWellFracturePartMgr.h"

#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"
#include "cvfMatrix4.h"
#include "cvfPlane.h"

#include <QMessageBox>
#include <QString>

#include <math.h>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimFracture, "Fracture");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void setDefaultFractureColorResult()
{
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();

    for (RimEclipseCase* const eclCase : proj->eclipseCases())
    {
        for (Rim3dView* const view : eclCase->views())
        {
            std::vector<RimStimPlanColors*> fractureColors;
            view->descendantsIncludingThisOfType(fractureColors);

            for (RimStimPlanColors* const stimPlanColors : fractureColors)
            {
                stimPlanColors->setDefaultResultName();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFracture::RimFracture()
{
    // clang-format off

    CAF_PDM_InitObject("Fracture", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_fractureTemplate, "FractureDef", "Fracture Template", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_anchorPosition, "AnchorPosition", "Anchor Position", "", "", "");
    m_anchorPosition.uiCapability()->setUiHidden(true);
    m_anchorPosition.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_uiAnchorPosition, "ui_positionAtWellpath", "Fracture Position", "", "", "");
    m_uiAnchorPosition.registerGetMethod(this, &RimFracture::fracturePositionForUi);
    m_uiAnchorPosition.uiCapability()->setUiReadOnly(true);
    m_uiAnchorPosition.xmlCapability()->disableIO();
    
    CAF_PDM_InitField(&m_azimuth, "Azimuth", 0.0, "Azimuth", "", "", "");
    m_azimuth.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&m_perforationLength, "PerforationLength", 1.0, "Perforation Length", "", "", "");
    CAF_PDM_InitField(&m_perforationEfficiency, "PerforationEfficiency", 1.0, "Perforation Efficiency", "", "", "");
    m_perforationEfficiency.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&m_wellDiameter, "WellDiameter", 0.216, "Well Diameter at Fracture", "", "", "");
    CAF_PDM_InitField(&m_dip, "Dip", 0.0, "Dip", "", "", "");
    CAF_PDM_InitField(&m_tilt, "Tilt", 0.0, "Tilt", "", "", "");
    
    CAF_PDM_InitField(&m_fractureUnit, "FractureUnit", caf::AppEnum<RiaEclipseUnitTools::UnitSystem>(RiaEclipseUnitTools::UNITS_METRIC), "Fracture Unit System", "", "", "");
    m_fractureUnit.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_stimPlanTimeIndexToPlot, "TimeIndexToPlot", 0, "StimPlan Time Step", "", "", ""); 

    CAF_PDM_InitFieldNoDefault(&m_uiWellPathAzimuth, "WellPathAzimuth", "Well Path Azimuth", "", "", "");
    m_uiWellPathAzimuth.registerGetMethod(this, &RimFracture::wellAzimuthAtFracturePositionText);
    m_uiWellPathAzimuth.uiCapability()->setUiReadOnly(true);
    m_uiWellPathAzimuth.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_uiWellFractureAzimuthDiff, "WellFractureAzimuthDiff", "Azimuth Difference Between\nFracture and Well", "", "", "");
    m_uiWellFractureAzimuthDiff.registerGetMethod(this, &RimFracture::wellFractureAzimuthDiffText);
    m_uiWellFractureAzimuthDiff.uiCapability()->setUiReadOnly(true);
    m_uiWellFractureAzimuthDiff.xmlCapability()->disableIO();
    
    CAF_PDM_InitField(&m_wellFractureAzimuthAngleWarning, "WellFractureAzimithAngleWarning", QString("Difference is below 10 degrees. Consider longitudinal fracture"), "", "", "", "");
    m_wellFractureAzimuthAngleWarning.uiCapability()->setUiReadOnly(true);
    m_wellFractureAzimuthAngleWarning.xmlCapability()->disableIO();

    m_fracturePartMgr = new RivWellFracturePartMgr(this);

    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFracture::~RimFracture() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFracture::perforationLength() const
{
    return m_perforationLength();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFracture::perforationEfficiency() const
{
    return m_perforationEfficiency();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFracture::setStimPlanTimeIndexToPlot(int timeIndex)
{
    m_stimPlanTimeIndexToPlot = timeIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimFracture::getPotentiallyFracturedCells(const RigMainGrid* mainGrid) const
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
        if (fractureUnit() != m_fractureTemplate->fractureTemplateUnit())
        {
            QString fractureUnitText = RiaEclipseUnitTools::UnitSystemType::uiText(fractureUnit());

            QString warningText =
                QString("Using a fracture template defined in a different unit is not supported.\n\nPlease select a "
                        "fracture template of unit '%1'")
                    .arg(fractureUnitText);

            QMessageBox::warning(nullptr, "Fracture Template Selection", warningText);

            PdmObjectHandle* prevValue    = oldValue.value<caf::PdmPointer<PdmObjectHandle>>().rawPtr();
            auto             prevTemplate = dynamic_cast<RimFractureTemplate*>(prevValue);

            m_fractureTemplate = prevTemplate;
        }

        setFractureTemplate(m_fractureTemplate);
        setDefaultFractureColorResult();
    }

    if (changedField == &m_azimuth || changedField == &m_fractureTemplate || changedField == &m_stimPlanTimeIndexToPlot ||
        changedField == this->objectToggleField() || changedField == &m_dip || changedField == &m_tilt ||
        changedField == &m_perforationLength)
    {
        clearCachedNonDarcyProperties();

        RimEclipseView* rimView = nullptr;
        this->firstAncestorOrThisOfType(rimView);
        if (rimView)
        {
            RimEclipseCase* eclipseCase = nullptr;
            rimView->firstAncestorOrThisOfType(eclipseCase);
            if (eclipseCase)
            {
                RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews(
                    eclipseCase);
            }
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
const NonDarcyData& RimFracture::nonDarcyProperties() const
{
    CVF_ASSERT(!m_cachedFractureProperties.isDirty());

    return m_cachedFractureProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFracture::ensureValidNonDarcyProperties()
{
    if (m_cachedFractureProperties.isDirty())
    {
        NonDarcyData props;

        if (m_fractureTemplate)
        {
            props.width                 = m_fractureTemplate->computeFractureWidth(this);
            props.conductivity          = m_fractureTemplate->computeKh(this);
            props.dFactor               = m_fractureTemplate->computeDFactor(this);
            props.effectivePermeability = m_fractureTemplate->computeEffectivePermeability(this);

            props.isDataDirty = false;
        }
        m_cachedFractureProperties = props;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFracture::clearCachedNonDarcyProperties()
{
    m_cachedFractureProperties = NonDarcyData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFracture::wellFractureAzimuthDiff() const
{
    double wellDifference = fabs(wellAzimuthAtFracturePosition() - m_azimuth);
    return wellDifference;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFracture::wellFractureAzimuthDiffText() const
{
    double wellDifference = wellFractureAzimuthDiff();
    return QString::number(wellDifference, 'f', 2);
}

QString RimFracture::wellAzimuthAtFracturePositionText() const
{
    double wellAzimuth = wellAzimuthAtFracturePosition();
    return QString::number(wellAzimuth, 'f', 2);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimFracture::boundingBoxInDomainCoords() const
{
    std::vector<cvf::Vec3f> nodeCoordVec;
    std::vector<cvf::uint>  triangleIndices;

    this->triangleGeometry(&triangleIndices, &nodeCoordVec);

    cvf::BoundingBox fractureBBox;
    for (const auto& nodeCoord : nodeCoordVec)
        fractureBBox.add(nodeCoord);

    return fractureBBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFracture::wellRadius() const
{
    if (m_fractureUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        return m_wellDiameter / 2.0;
    }
    else if (m_fractureUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        return RiaEclipseUnitTools::inchToFeet(m_wellDiameter / 2.0);
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
    cvf::Mat4d dipRotation = cvf::Mat4d::fromRotation(cvf::Vec3d::Z_AXIS, cvf::Math::toRadians(m_dip()));

    // Dip (out of XY plane)
    cvf::Mat4d tiltRotation = cvf::Mat4d::fromRotation(cvf::Vec3d::X_AXIS, cvf::Math::toRadians(m_tilt()));

    // Ellipsis geometry is produced in XY-plane, rotate 90 deg around X to get zero azimuth along Y
    cvf::Mat4d rotationFromTesselator = cvf::Mat4d::fromRotation(cvf::Vec3d::X_AXIS, cvf::Math::toRadians(90.0f));

    // Azimuth rotation
    cvf::Mat4d azimuthRotation = cvf::Mat4d::fromRotation(cvf::Vec3d::Z_AXIS, cvf::Math::toRadians(-m_azimuth() - 90));

    cvf::Mat4d m = azimuthRotation * rotationFromTesselator * dipRotation * tiltRotation;
    m.setTranslation(center);

    return m;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFracture::dip() const
{
    return m_dip();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFracture::tilt() const
{
    return m_tilt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFracture::setFractureTemplateNoUpdate(RimFractureTemplate* fractureTemplate)
{
    if (fractureTemplate && fractureTemplate->fractureTemplateUnit() != fractureUnit())
    {
        QString fractureUnitText = RiaEclipseUnitTools::UnitSystemType::uiText(fractureUnit());

        QString warningText =
            QString("Using a fracture template defined in a different unit is not supported.\n\nPlease select a "
                    "fracture template of unit '%1'")
                .arg(fractureUnitText);

        QMessageBox::warning(nullptr, "Fracture Template Selection", warningText);

        return;
    }

    m_fractureTemplate = fractureTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFracture::triangleGeometry(std::vector<cvf::uint>* triangleIndices, std::vector<cvf::Vec3f>* nodeCoords) const
{
    RimFractureTemplate* fractureDef = fractureTemplate();
    if (fractureDef)
    {
        fractureDef->fractureTriangleGeometry(nodeCoords, triangleIndices);
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
QList<caf::PdmOptionItemInfo> RimFracture::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    RimProject* proj = RiaApplication::instance()->project();
    CVF_ASSERT(proj);

    if (fieldNeedingOptions == &m_fractureTemplate)
    {
        RimOilField* oilField = proj->activeOilField();
        if (oilField && oilField->fractureDefinitionCollection)
        {
            RimFractureTemplateCollection* fracDefColl = oilField->fractureDefinitionCollection();

            for (RimFractureTemplate* fracDef : fracDefColl->fractureTemplates())
            {
                QString displayText = fracDef->nameAndUnit();
                if (fracDef->fractureTemplateUnit() != fractureUnit())
                {
                    displayText += " (non-matching unit)";
                }

                options.push_back(caf::PdmOptionItemInfo(displayText, fracDef));
            }
        }
    }
    else if (fieldNeedingOptions == &m_stimPlanTimeIndexToPlot)
    {
        if (fractureTemplate())
        {
            RimFractureTemplate* fracTemplate = fractureTemplate();
            if (dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate))
            {
                RimStimPlanFractureTemplate* fracTemplateStimPlan = dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate);
                std::vector<double>          timeValues           = fracTemplateStimPlan->timeSteps();
                int                          index                = 0;
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
        m_wellDiameter.uiCapability()->setUiName("Well Diameter [m]");
        m_perforationLength.uiCapability()->setUiName("Perforation Length [m]");
    }
    else if (m_fractureUnit() == RiaEclipseUnitTools::UNITS_FIELD)
    {
        m_wellDiameter.uiCapability()->setUiName("Well Diameter [inches]");
        m_perforationLength.uiCapability()->setUiName("Perforation Length [ft]");
    }

    if (fractureTemplate())
    {
        if (fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH ||
            fractureTemplate()->orientationType() == RimFractureTemplate::TRANSVERSE_WELL_PATH)
        {
            m_uiWellPathAzimuth.uiCapability()->setUiHidden(true);
            m_uiWellFractureAzimuthDiff.uiCapability()->setUiHidden(true);
            m_wellFractureAzimuthAngleWarning.uiCapability()->setUiHidden(true);
        }

        else if (fractureTemplate()->orientationType() == RimFractureTemplate::AZIMUTH)
        {
            m_uiWellPathAzimuth.uiCapability()->setUiHidden(false);
            m_uiWellFractureAzimuthDiff.uiCapability()->setUiHidden(false);
            if (wellFractureAzimuthDiff() < 10 || (wellFractureAzimuthDiff() > 170 && wellFractureAzimuthDiff() < 190) ||
                wellFractureAzimuthDiff() > 350)
            {
                m_wellFractureAzimuthAngleWarning.uiCapability()->setUiHidden(false);
            }
            else
            {
                m_wellFractureAzimuthAngleWarning.uiCapability()->setUiHidden(true);
            }
        }

        if (fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH ||
            fractureTemplate()->orientationType() == RimFractureTemplate::TRANSVERSE_WELL_PATH)
        {
            m_azimuth.uiCapability()->setUiReadOnly(true);
        }
        else if (fractureTemplate()->orientationType() == RimFractureTemplate::AZIMUTH)
        {
            m_azimuth.uiCapability()->setUiReadOnly(false);
        }

        if (fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH)
        {
            m_perforationEfficiency.uiCapability()->setUiHidden(false);
            m_perforationLength.uiCapability()->setUiHidden(false);
        }
        else
        {
            m_perforationEfficiency.uiCapability()->setUiHidden(true);
            m_perforationLength.uiCapability()->setUiHidden(true);
        }

        if (fractureTemplate()->conductivityType() == RimFractureTemplate::FINITE_CONDUCTIVITY)
        {
            m_wellDiameter.uiCapability()->setUiHidden(false);
        }
        else if (fractureTemplate()->conductivityType() == RimFractureTemplate::INFINITE_CONDUCTIVITY)
        {
            m_wellDiameter.uiCapability()->setUiHidden(true);
        }

        RimFractureTemplate* fracTemplate = fractureTemplate();
        if (dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate))
        {
            m_stimPlanTimeIndexToPlot.uiCapability()->setUiHidden(false);

            m_stimPlanTimeIndexToPlot.uiCapability()->setUiReadOnly(true);
        }
        else
        {
            m_stimPlanTimeIndexToPlot.uiCapability()->setUiHidden(true);
        }
    }
    else
    {
        m_stimPlanTimeIndexToPlot.uiCapability()->setUiHidden(true);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFracture::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                        QString                    uiConfigName,
                                        caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_azimuth)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_minimum = 0;
            myAttr->m_maximum = 360;
        }
    }

    if (field == &m_perforationEfficiency)
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
bool RimFracture::isEclipseCellWithinContainment(const RigMainGrid*      mainGrid,
                                                 const std::set<size_t>& containmentCells,
                                                 size_t                  globalCellIndex) const
{
    CVF_ASSERT(fractureTemplate());
    if (!fractureTemplate()->fractureContainment()->isEnabled()) return true;

    size_t anchorEclipseCell = mainGrid->findReservoirCellIndexFromPoint(m_anchorPosition);

    return fractureTemplate()->fractureContainment()->isEclipseCellWithinContainment(
        mainGrid, anchorEclipseCell, globalCellIndex, containmentCells);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFracture::setFractureTemplate(RimFractureTemplate* fractureTemplate)
{
    setFractureTemplateNoUpdate(fractureTemplate);

    if (!fractureTemplate)
    {
        return;
    }

    RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(fractureTemplate);
    if (stimPlanFracTemplate)
    {
        m_stimPlanTimeIndexToPlot = stimPlanFracTemplate->activeTimeStepIndex();
    }

    if (fractureTemplate->orientationType() == RimFractureTemplate::AZIMUTH)
    {
        m_azimuth = fractureTemplate->azimuthAngle();
    }
    else
    {
        this->updateAzimuthBasedOnWellAzimuthAngle();
    }
    this->m_wellDiameter      = fractureTemplate->wellDiameter();
    this->m_perforationLength = fractureTemplate->perforationLength();

    clearCachedNonDarcyProperties();
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
