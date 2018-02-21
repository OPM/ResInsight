/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RimFractureTemplate.h"

#include "RiaFractureDefines.h"
#include "RigTesselatorTools.h"

#include "RimFracture.h"
#include "RimFractureContainment.h"
#include "RimProject.h"

#include "cafPdmObject.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiTextEditor.h"

#include "cvfVector3.h"

#include <cmath>

namespace caf
{
    template<>
    void caf::AppEnum< RimFractureTemplate::FracOrientationEnum>::setUp()
    {
        addItem(RimFractureTemplate::AZIMUTH, "Az", "Azimuth");
        addItem(RimFractureTemplate::ALONG_WELL_PATH, "AlongWellPath", "Along Well Path");
        addItem(RimFractureTemplate::TRANSVERSE_WELL_PATH, "TransverseWellPath", "Transverse (normal) to Well Path");

        setDefault(RimFractureTemplate::TRANSVERSE_WELL_PATH);
    }

    template<>
    void caf::AppEnum< RimFractureTemplate::FracConductivityEnum>::setUp()
    {
        addItem(RimFractureTemplate::INFINITE_CONDUCTIVITY, "InfiniteConductivity", "Infinite Conductivity");
        addItem(RimFractureTemplate::FINITE_CONDUCTIVITY, "FiniteConductivity", "Finite Conductivity");

        setDefault(RimFractureTemplate::INFINITE_CONDUCTIVITY);
    }

    template<>
    void caf::AppEnum< RimFractureTemplate::PermeabilityEnum>::setUp()
    {
        addItem(RimFractureTemplate::USER_DEFINED_PERMEABILITY, "UserDefinedPermeability",  "User Defined");
        addItem(RimFractureTemplate::CONDUCTIVITY_FROM_FRACTURE, "FractureConductivity",    "Use Fracture Conductivity");

        setDefault(RimFractureTemplate::CONDUCTIVITY_FROM_FRACTURE);
    }

    template<>
    void caf::AppEnum<RimFractureTemplate::WidthEnum>::setUp()
    {
        addItem(RimFractureTemplate::USER_DEFINED_WIDTH, "UserDefinedWidth", "User Defined");
        addItem(RimFractureTemplate::WIDTH_FROM_FRACTURE, "FractureWidth", "Use Fracture Width");

        setDefault(RimFractureTemplate::WIDTH_FROM_FRACTURE);
    }

    template<>
    void caf::AppEnum<RimFractureTemplate::NonDarcyFlowEnum>::setUp()
    {
        addItem(RimFractureTemplate::NON_DARCY_NONE, "None", "None");
        addItem(RimFractureTemplate::NON_DARCY_COMPUTED, "Computed", "Compute D-factor from Parameters");
        addItem(RimFractureTemplate::NON_DARCY_USER_DEFINED, "UserDefined", "By User Defined D-factor");

        setDefault(RimFractureTemplate::NON_DARCY_NONE);
    }
}

// TODO Move to cafPdmObject.h
#define CAF_PDM_InitField_Basic(field, keyword, default, uiName) CAF_PDM_InitField(field, keyword, default, uiName, "", "", "")


CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimFractureTemplate, "RimFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate::RimFractureTemplate()
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&m_name,                "UserDescription",  QString("Fracture Template"), "Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_nameAndUnit, "NameAndUnit", "NameAndUnit", "", "", "");
    m_nameAndUnit.registerGetMethod(this, &RimFractureTemplate::nameAndUnit);
    m_nameAndUnit.uiCapability()->setUiHidden(true);
    m_nameAndUnit.xmlCapability()->disableIO();

    CAF_PDM_InitField(&m_fractureTemplateUnit,"UnitSystem", caf::AppEnum<RiaEclipseUnitTools::UnitSystem>(RiaEclipseUnitTools::UNITS_METRIC), "Units System", "", "", "");
    m_fractureTemplateUnit.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_orientationType,     "Orientation",  caf::AppEnum<FracOrientationEnum>(TRANSVERSE_WELL_PATH), "Fracture Orientation", "", "", "");
    CAF_PDM_InitField(&m_azimuthAngle,        "AzimuthAngle", 0.0f, "Azimuth Angle", "", "", ""); //Is this correct description?
    CAF_PDM_InitField(&m_skinFactor,          "SkinFactor",   0.0f, "Skin Factor", "", "", "");

    CAF_PDM_InitField(&m_perforationLength,     "PerforationLength",      1.0, "Perforation Length", "", "", "");
    CAF_PDM_InitField(&m_perforationEfficiency, "PerforationEfficiency",  1.0, "Perforation Efficiency", "", "", "");
    m_perforationEfficiency.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    CAF_PDM_InitField(&m_wellDiameter,        "WellDiameter", 0.216, "Well Diameter at Fracture", "", "", "");

    CAF_PDM_InitField(&m_conductivityType,    "ConductivityType", caf::AppEnum<FracConductivityEnum>(FINITE_CONDUCTIVITY), "Conductivity in Fracture", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_fractureContainment, "FractureContainmentField", "Fracture Containment", "", "", "");
    m_fractureContainment = new RimFractureContainment();
    m_fractureContainment.uiCapability()->setUiTreeHidden(true);
    m_fractureContainment.uiCapability()->setUiTreeChildrenHidden(true);

    // Non-Darcy Flow options
    CAF_PDM_InitFieldNoDefault(&m_nonDarcyFlowType,     "NonDarcyFlowType",      "Non-Darcy Flow", "", "", "");

    CAF_PDM_InitField(&m_userDefinedDFactor,            "UserDefinedDFactor",      1.0, "D Factor", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_fractureWidthType,    "FractureWidthType",     "Type", "", "", "");
    CAF_PDM_InitField_Basic(&m_fractureWidth,           "FractureWidth",  0.1,    "Fracture Width (h)");

    CAF_PDM_InitField_Basic(&m_inertialCoefficient,     "InertialCoefficient",  0.006083236,    "<html>Inertial Coefficient (&beta;)</html>");

    CAF_PDM_InitFieldNoDefault(&m_permeabilityType,         "PermeabilityType",     "Type", "", "", "");
    CAF_PDM_InitField_Basic(&m_relativePermeability,        "RelativePermeability", 1.0,    "Relative Permeability");
    CAF_PDM_InitField(&m_userDefinedEffectivePermeability,  "EffectivePermeability",0.0,    "Effective Permeability (Ke) [mD]", "", "", "");

    CAF_PDM_InitField(&m_relativeGasDensity,            "RelativeGasDensity",   0.8,    "<html>Relative Gas Density (&gamma;)</html>", "", "Relative density of gas at surface conditions with respect to air at STP", "");
    CAF_PDM_InitField(&m_gasViscosity,                  "GasViscosity",         0.02,   "<html>Gas Viscosity (&mu;)</html>", "", "Gas viscosity at bottom hole pressure", "");

    CAF_PDM_InitFieldNoDefault(&m_dFactorDisplayField, "dFactorDisplayField", "D Factor", "", "", "");
    m_dFactorDisplayField.registerGetMethod(this, &RimFractureTemplate::dFactor);
    m_dFactorDisplayField.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleValueEditor::uiEditorTypeName());
    m_dFactorDisplayField.uiCapability()->setUiReadOnly(true);
    m_dFactorDisplayField.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_dFactorSummaryText, "dFactorSummaryText", "D Factor Summary", "", "", "");
    m_dFactorSummaryText.registerGetMethod(this, &RimFractureTemplate::dFactorSummary);
    m_dFactorSummaryText.uiCapability()->setUiReadOnly(true);
    m_dFactorSummaryText.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    m_dFactorSummaryText.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::LabelPosType::TOP);
    m_dFactorSummaryText.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate::~RimFractureTemplate()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::setName(const QString& name)
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::setFractureTemplateUnit(RiaEclipseUnitTools::UnitSystemType unitSystem)
{
    m_fractureTemplateUnit = unitSystem;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimFractureTemplate::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate::FracOrientationEnum RimFractureTemplate::orientationType() const
{
    return m_orientationType();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystemType RimFractureTemplate::fractureTemplateUnit() const
{
    return m_fractureTemplateUnit();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFractureTemplate::userDescriptionField()
{
    return &m_nameAndUnit;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_azimuthAngle || changedField == &m_orientationType)
    {
        //Changes to one of these parameters should change all fractures with this fracture template attached. 
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            //Regenerate geometry
            std::vector<RimFracture*> fractures;
            proj->descendantsIncludingThisOfType(fractures);

            for (RimFracture* fracture : fractures)
            {
                if (fracture->fractureTemplate() == this)
                {
                    if (changedField == &m_azimuthAngle && (fabs(oldValue.toDouble() - fracture->m_azimuth()) < 1e-5))
                    {
                        fracture->m_azimuth = m_azimuthAngle;
                    }

                    if (changedField == &m_orientationType)
                    {
                        if (newValue == AZIMUTH)
                        {
                            fracture->m_azimuth = m_azimuthAngle;
                        }
                        else fracture->updateAzimuthBasedOnWellAzimuthAngle();
                    }
                }
            }

            proj->createDisplayModelAndRedrawAllViews();
        }
    }

    if (changedField == &m_perforationLength || changedField == &m_perforationEfficiency || changedField == &m_wellDiameter)
    {
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (!proj) return;
        std::vector<RimFracture*> fractures;
        proj->descendantsIncludingThisOfType(fractures);

        for (RimFracture* fracture : fractures)
        {
            if (fracture->fractureTemplate() == this)
            {
                if (changedField == &m_perforationLength && (fabs(oldValue.toDouble() - fracture->m_perforationLength()) < 1e-5))
                {
                    fracture->m_perforationLength = m_perforationLength;
                }
                if (changedField == &m_perforationEfficiency && (fabs(oldValue.toDouble() - fracture->m_perforationEfficiency()) < 1e-5))
                {
                    fracture->m_perforationEfficiency = m_perforationEfficiency;
                }
                if (changedField == &m_wellDiameter && (fabs(oldValue.toDouble() - fracture->m_wellDiameter()) < 1e-5))
                {
                    fracture->m_wellDiameter = m_wellDiameter;
                }
            }
        }
    }

    if (changedField == &m_perforationLength)
    {
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            proj->createDisplayModelAndRedrawAllViews();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    prepareFieldsForUiDisplay();

    auto nonDarcyFlowGroup = uiOrdering.addNewGroup("Non-Darcy Flow");
    nonDarcyFlowGroup->add(&m_nonDarcyFlowType);
    
    if (m_nonDarcyFlowType == RimFractureTemplate::NON_DARCY_USER_DEFINED)
    {
        nonDarcyFlowGroup->add(&m_userDefinedDFactor);
    }

    if (m_nonDarcyFlowType == RimFractureTemplate::NON_DARCY_COMPUTED)
    {
        nonDarcyFlowGroup->add(&m_inertialCoefficient);

        {
            auto group = nonDarcyFlowGroup->addNewGroup("Effective Permeability");
            group->add(&m_permeabilityType);
            group->add(&m_relativePermeability);
            group->add(&m_userDefinedEffectivePermeability);
        }

        {
            auto group = nonDarcyFlowGroup->addNewGroup("Width");
            group->add(&m_fractureWidthType);
            group->add(&m_fractureWidth);
        }

        nonDarcyFlowGroup->add(&m_relativeGasDensity);
        nonDarcyFlowGroup->add(&m_gasViscosity);
        nonDarcyFlowGroup->add(&m_dFactorDisplayField);

        {
            auto group  = nonDarcyFlowGroup->addNewGroup("D Factor Details");
            group->setCollapsedByDefault(true);
            group->add(&m_dFactorSummaryText);
        }
    }

    uiOrdering.add(&m_fractureTemplateUnit);
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_perforationEfficiency)
    {
        auto myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_minimum = 0;
            myAttr->m_maximum = 1.0;
        }
    }

    if (field == &m_dFactorSummaryText)
    {
        auto myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
            
            QFont font("Monospace", 10);
            myAttr->font = font;
            myAttr->textMode = caf::PdmUiTextEditorAttribute::HTML;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFractureTemplate::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                         bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_fractureWidthType)
    {
        options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<WidthEnum>::uiText(USER_DEFINED_WIDTH), USER_DEFINED_WIDTH));

        auto widthAndCond = widthAndConductivityAtWellPathIntersection();
        if (widthAndCond.isValid())
        {
            options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<WidthEnum>::uiText(WIDTH_FROM_FRACTURE), WIDTH_FROM_FRACTURE));
        }
        else
        {
            m_fractureWidthType = USER_DEFINED_WIDTH;
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::prepareFieldsForUiDisplay()
{
    if (m_fractureTemplateUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        m_wellDiameter.uiCapability()->setUiName("Well Diameter [m]");
        m_perforationLength.uiCapability()->setUiName("Perforation Length [m]");
    }
    else if (m_fractureTemplateUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        m_wellDiameter.uiCapability()->setUiName("Well Diameter [inches]");
        m_perforationLength.uiCapability()->setUiName("Perforation Length [Ft]");
    }

    if (m_orientationType == RimFractureTemplate::ALONG_WELL_PATH
        || m_orientationType == RimFractureTemplate::TRANSVERSE_WELL_PATH)
    {
        m_azimuthAngle.uiCapability()->setUiHidden(true);
    }
    else if (m_orientationType == RimFractureTemplate::AZIMUTH)
    {
        m_azimuthAngle.uiCapability()->setUiHidden(false);
    }

    if (m_orientationType == RimFractureTemplate::ALONG_WELL_PATH)
    {
        m_perforationEfficiency.uiCapability()->setUiHidden(false);
        m_perforationLength.uiCapability()->setUiHidden(false);
    }
    else 
    {
        m_perforationEfficiency.uiCapability()->setUiHidden(true);
        m_perforationLength.uiCapability()->setUiHidden(true);
    }

    if (m_conductivityType == FINITE_CONDUCTIVITY)
    {
        m_wellDiameter.uiCapability()->setUiHidden(false);
    }
    else if (m_conductivityType == INFINITE_CONDUCTIVITY)
    {
        m_wellDiameter.uiCapability()->setUiHidden(true);
    }

    // Non Darcy Flow

    auto values = widthAndConductivityAtWellPathIntersection();
    if (!values.isValid())
    {
        m_fractureWidthType = RimFractureTemplate::USER_DEFINED_WIDTH;
    }

    if (m_fractureWidthType == RimFractureTemplate::USER_DEFINED_WIDTH)
    {
        m_fractureWidth.uiCapability()->setUiReadOnly(false);
    }
    else
    {
        m_fractureWidth = values.m_width;

        m_fractureWidth.uiCapability()->setUiReadOnly(true);
    }

    if (m_permeabilityType == RimFractureTemplate::USER_DEFINED_PERMEABILITY)
    {
        m_relativePermeability.uiCapability()->setUiHidden(true);
        m_userDefinedEffectivePermeability.uiCapability()->setUiHidden(false);
    }
    else
    {
        m_relativePermeability.uiCapability()->setUiHidden(false);
        m_userDefinedEffectivePermeability.uiCapability()->setUiHidden(true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimFractureTemplate::dFactorSummary() const
{
    QString text;
    
    auto val = dFactor();
    text += QString("D-factor : %1").arg(val);

    text += "<br>";
    text += "<br>";
    auto alpha = RiaDefines::nonDarcyFlowAlpha(m_fractureTemplateUnit());
    text += QString("&alpha;  : %1").arg(alpha);

    text += "<br>";
    auto beta = m_inertialCoefficient;
    text += QString("&beta;  : %1").arg(beta);

    text += "<br>";
    double effPerm = effectivePermeability();
    text += QString("Ke : %1").arg(effPerm);

    text += "<br>";
    double gamma = m_relativeGasDensity;
    text += QString("&gamma;  : %1").arg(gamma);

    text += "<br>";
    auto h = fractureWidth();
    text += QString("h  : %1").arg(h);

    text += "<br>";
    auto wellRadius = m_wellDiameter / 2.0;
    text += QString("rw : %1").arg(wellRadius);

    text += "<br>";
    auto mu = m_gasViscosity;
    text += QString("&mu;  : %1").arg(mu);

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::effectivePermeability() const
{
    if (m_permeabilityType() == RimFractureTemplate::USER_DEFINED_PERMEABILITY)
    {
        return m_userDefinedEffectivePermeability;
    }
    else
    {
        auto values = widthAndConductivityAtWellPathIntersection();

        auto fracPermeability = values.m_permeability;

        return fracPermeability * m_relativePermeability;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::dFactor() const
{
    if (m_nonDarcyFlowType == RimFractureTemplate::NON_DARCY_USER_DEFINED)
    {
        return m_userDefinedDFactor;
    }

    auto alpha   = RiaDefines::nonDarcyFlowAlpha(m_fractureTemplateUnit());
    auto beta    = m_inertialCoefficient;
    auto effPerm = effectivePermeability();
    auto gamma   = m_relativeGasDensity;
    
    auto radius  = m_wellDiameter / 2.0;
    auto mu      = m_gasViscosity;
    auto h       = fractureWidth();

    double numerator   = alpha * beta * effPerm * gamma;
    double denumerator = h * radius * mu;

    if (denumerator < 1e-10) return HUGE_VAL;

    return numerator / denumerator;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::kh() const
{
    return effectivePermeability() * fractureWidth();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::convertToUnitSystem(RiaEclipseUnitTools::UnitSystem neededUnit)
{
    if (neededUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        m_perforationLength = RiaEclipseUnitTools::feetToMeter(m_perforationLength);
        m_wellDiameter      = RiaEclipseUnitTools::inchToMeter(m_wellDiameter);
    }
    else if (neededUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        m_perforationLength = RiaEclipseUnitTools::meterToFeet(m_perforationLength);
        m_wellDiameter      = RiaEclipseUnitTools::meterToInch(m_wellDiameter);
    }

    // TODO : Convert NON-darcy values
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::disconnectAllFracturesAndRedrawViews() const
{
    // The unit has changed. Disconnect all fractures referencing this fracture template to avoid mix of units between fracture
    // and template

    std::vector<caf::PdmObjectHandle*> referringObjects;
    this->objectsWithReferringPtrFields(referringObjects);

    for (auto objHandle : referringObjects)
    {
        RimFracture* fracture = dynamic_cast<RimFracture*>(objHandle);
        if (fracture)
        {
            fracture->setFractureTemplate(nullptr);
        }
    }

    RimProject* proj;
    this->firstAncestorOrThisOfType(proj);
    if (proj)
    {
        proj->createDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::fractureWidth() const
{
    if (m_fractureWidthType == RimFractureTemplate::WIDTH_FROM_FRACTURE)
    {
        auto values = widthAndConductivityAtWellPathIntersection();
    
        return values.m_width;
    }

    return m_fractureWidth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimFractureTemplate::nameAndUnit() const
{
    QString decoratedName;

    if (m_fractureTemplateUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        decoratedName += "[M] - ";
    }
    else if (m_fractureTemplateUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        decoratedName += "[F] - ";
    }

    decoratedName += m_name;

    return decoratedName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::wellDiameter()
{
    return m_wellDiameter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::perforationLength()
{
    return m_perforationLength;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimFractureContainment * RimFractureTemplate::fractureContainment()
{
    return m_fractureContainment();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate::FracConductivityEnum RimFractureTemplate::conductivityType() const
{
    return m_conductivityType();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RimFractureTemplate::azimuthAngle() const
{
    return m_azimuthAngle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RimFractureTemplate::skinFactor() const
{
    return m_skinFactor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::setDefaultWellDiameterFromUnit()
{
    if (m_fractureTemplateUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        m_wellDiameter = 8.5;
    }
    else if (m_fractureTemplateUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        m_wellDiameter = 0.216;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFractureTemplate::isNonDarcyFlowEnabled() const
{
    return m_nonDarcyFlowType() != RimFractureTemplate::NON_DARCY_NONE;
}
