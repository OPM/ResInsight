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
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTextEditor.h"

#include "cvfVector3.h"

#include <cmath>

// clang-format off

namespace caf
{
    template<>
    void caf::AppEnum< RimFractureTemplate::FracOrientationEnum>::setUp()
    {
        addItem(RimFractureTemplate::AZIMUTH,               "Az",                   "Azimuth");
        addItem(RimFractureTemplate::ALONG_WELL_PATH,       "AlongWellPath",        "Along Well Path");
        addItem(RimFractureTemplate::TRANSVERSE_WELL_PATH,  "TransverseWellPath",   "Transverse (normal) to Well Path");

        setDefault(RimFractureTemplate::TRANSVERSE_WELL_PATH);
    }

    template<>
    void caf::AppEnum< RimFractureTemplate::FracConductivityEnum>::setUp()
    {
        addItem(RimFractureTemplate::INFINITE_CONDUCTIVITY, "InfiniteConductivity", "Infinite Conductivity");
        addItem(RimFractureTemplate::FINITE_CONDUCTIVITY,   "FiniteConductivity",   "Finite Conductivity");

        setDefault(RimFractureTemplate::INFINITE_CONDUCTIVITY);
    }

    template<>
    void caf::AppEnum< RimFractureTemplate::PermeabilityEnum>::setUp()
    {
        addItem(RimFractureTemplate::USER_DEFINED_PERMEABILITY,     "UserDefinedPermeability",  "User Defined");
        addItem(RimFractureTemplate::CONDUCTIVITY_FROM_FRACTURE,    "FractureConductivity",     "Use Fracture Conductivity");

        setDefault(RimFractureTemplate::CONDUCTIVITY_FROM_FRACTURE);
    }

    template<>
    void caf::AppEnum<RimFractureTemplate::WidthEnum>::setUp()
    {
        addItem(RimFractureTemplate::USER_DEFINED_WIDTH,    "UserDefinedWidth", "User Defined");
        addItem(RimFractureTemplate::WIDTH_FROM_FRACTURE,   "FractureWidth",    "Use Fracture Width");

        setDefault(RimFractureTemplate::WIDTH_FROM_FRACTURE);
    }

    template<>
    void caf::AppEnum<RimFractureTemplate::NonDarcyFlowEnum>::setUp()
    {
        addItem(RimFractureTemplate::NON_DARCY_NONE,        "None",         "None");
        addItem(RimFractureTemplate::NON_DARCY_COMPUTED,    "Computed",     "Compute D-factor");
        addItem(RimFractureTemplate::NON_DARCY_USER_DEFINED,"UserDefined",  "User Defined D-factor");

        setDefault(RimFractureTemplate::NON_DARCY_NONE);
    }

    template<>
    void caf::AppEnum< RimFractureTemplate::BetaFactorEnum>::setUp()
    {
        addItem(RimFractureTemplate::USER_DEFINED_BETA_FACTOR, "UserDefinedBetaFactor", "User Defined");
        addItem(RimFractureTemplate::BETA_FACTOR_FROM_FRACTURE, "FractureBetaFactor", "Use Fracture Beta Factor");

        setDefault(RimFractureTemplate::USER_DEFINED_BETA_FACTOR);
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

    CAF_PDM_InitField(&m_id, "Id", -1, "ID", "", "", "");
    m_id.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_name,                "UserDescription",  QString("Fracture Template"), "Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_nameAndUnit, "NameAndUnit", "NameAndUnit", "", "", "");
    m_nameAndUnit.registerGetMethod(this, &RimFractureTemplate::nameAndUnit);
    m_nameAndUnit.uiCapability()->setUiHidden(true);
    m_nameAndUnit.xmlCapability()->disableIO();

    CAF_PDM_InitField(&m_fractureTemplateUnit, "UnitSystem", caf::AppEnum<RiaEclipseUnitTools::UnitSystem>(RiaEclipseUnitTools::UNITS_UNKNOWN), "Units System", "", "", "");
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
    CAF_PDM_InitField_Basic(&m_fractureWidth,           "FractureWidth",  0.01,    "Fracture Width (h)");

    CAF_PDM_InitFieldNoDefault(&m_betaFactorType,       "BetaFactorType", "Type", "", "", "");
    CAF_PDM_InitField_Basic(&m_inertialCoefficient,     "InertialCoefficient",  0.006083236,    "<html>Inertial Coefficient (&beta;)</html> [Forch. unit]");

    CAF_PDM_InitFieldNoDefault(&m_permeabilityType,         "PermeabilityType",     "Type", "", "", "");
    CAF_PDM_InitField_Basic(&m_relativePermeability,        "RelativePermeability", 1.0,    "Relative Permeability");
    CAF_PDM_InitField(&m_userDefinedEffectivePermeability,  "EffectivePermeability",0.0,    "Effective Permeability (Ke) [mD]", "", "", "");

    CAF_PDM_InitField(&m_relativeGasDensity,            "RelativeGasDensity",   0.8,    "<html>Relative Gas Density (&gamma;)</html>", "", "Relative density of gas at surface conditions with respect to air at STP", "");
    CAF_PDM_InitField(&m_gasViscosity,                  "GasViscosity",         0.02,   "<html>Gas Viscosity (&mu;)</html> [cP]", "", "Gas viscosity at bottom hole pressure", "");

    CAF_PDM_InitFieldNoDefault(&m_dFactorDisplayField, "dFactorDisplayField", "D Factor", "", "", "");
    m_dFactorDisplayField.registerGetMethod(this, &RimFractureTemplate::dFactorForTemplate);
    m_dFactorDisplayField.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleValueEditor::uiEditorTypeName());
    m_dFactorDisplayField.uiCapability()->setUiReadOnly(true);
    m_dFactorDisplayField.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_dFactorSummaryText, "dFactorSummaryText", "D Factor Summary", "", "", "");
    m_dFactorSummaryText.registerGetMethod(this, &RimFractureTemplate::dFactorSummary);
    m_dFactorSummaryText.uiCapability()->setUiReadOnly(true);
    m_dFactorSummaryText.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    m_dFactorSummaryText.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::LabelPosType::TOP);
    m_dFactorSummaryText.xmlCapability()->disableIO();

    CAF_PDM_InitField(&m_heightScaleFactor, "HeightScaleFactor", 1.0, "Height", "", "", "");
    CAF_PDM_InitField(&m_halfLengthScaleFactor,  "WidthScaleFactor",  1.0, "Half Length", "", "", "");
    CAF_PDM_InitField(&m_dFactorScaleFactor, "DFactorScaleFactor", 1.0, "D-factor", "", "", "");
    CAF_PDM_InitField(&m_conductivityScaleFactor, "ConductivityFactor", 1.0, "Conductivity", "", "The conductivity values read from file will be scaled with this parameters", "");
    CAF_PDM_InitField(&m_scaleApplyButton, "ScaleApplyButton", false, "Apply", "", "", "");

    m_scaleApplyButton.xmlCapability()->disableIO();
    m_scaleApplyButton.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_scaleApplyButton.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
}

// clang-format on

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplate::~RimFractureTemplate() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFractureTemplate::id() const
{
    return m_id;
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
void RimFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue)
{
    bool createDisplayModelAndRedraw = false;
    if (changedField == &m_azimuthAngle || changedField == &m_orientationType)
    {
        for (RimFracture* fracture : fracturesUsingThisTemplate())
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
                else
                {
                    fracture->updateAzimuthBasedOnWellAzimuthAngle();
                }
            }

            createDisplayModelAndRedraw = true;
        }
    }

    if (changedField == &m_perforationLength || changedField == &m_perforationEfficiency || changedField == &m_wellDiameter)
    {
        for (RimFracture* fracture : fracturesUsingThisTemplate())
        {
            if (changedField == &m_perforationLength && (fabs(oldValue.toDouble() - fracture->m_perforationLength()) < 1e-5))
            {
                fracture->m_perforationLength = m_perforationLength;
            }
            if (changedField == &m_perforationEfficiency &&
                (fabs(oldValue.toDouble() - fracture->m_perforationEfficiency()) < 1e-5))
            {
                fracture->m_perforationEfficiency = m_perforationEfficiency;
            }
            if (changedField == &m_wellDiameter && (fabs(oldValue.toDouble() - fracture->m_wellDiameter()) < 1e-5))
            {
                fracture->m_wellDiameter = m_wellDiameter;
            }
        }
    }

    for (RimFracture* fracture : fracturesUsingThisTemplate())
    {
        fracture->clearCachedNonDarcyProperties();
    }

    if (changedField == &m_perforationLength)
    {
        createDisplayModelAndRedraw = true;
    }

    if (createDisplayModelAndRedraw)
    {
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            proj->reloadCompletionTypeResultsInAllViews();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    prepareFieldsForUiDisplay();

    {
        auto group = uiOrdering.addNewGroup("Sensitivity Scale Factors");
        group->setCollapsedByDefault(true);
        group->add(&m_heightScaleFactor);
        group->add(&m_halfLengthScaleFactor);
        group->add(&m_dFactorScaleFactor);
        group->add(&m_conductivityScaleFactor);

        group->add(&m_scaleApplyButton);
    }

    auto nonDarcyFlowGroup = uiOrdering.addNewGroup("Non-Darcy Flow");
    nonDarcyFlowGroup->add(&m_nonDarcyFlowType);

    if (m_nonDarcyFlowType == RimFractureTemplate::NON_DARCY_USER_DEFINED)
    {
        nonDarcyFlowGroup->add(&m_userDefinedDFactor);
    }

    if (m_nonDarcyFlowType == RimFractureTemplate::NON_DARCY_COMPUTED)
    {
        {
            auto group = nonDarcyFlowGroup->addNewGroup("<html>Inertial Coefficient(&beta;-factor)</html>");
            group->add(&m_betaFactorType);
            group->add(&m_inertialCoefficient);
        }

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

        if (orientationType() != ALONG_WELL_PATH)
        {
            nonDarcyFlowGroup->add(&m_dFactorDisplayField);
        }

        {
            auto group = nonDarcyFlowGroup->addNewGroup("D Factor Details");
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
void RimFractureTemplate::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute)
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
            myAttr->font     = font;
            myAttr->textMode = caf::PdmUiTextEditorAttribute::HTML;
        }
    }

    if (field == &m_scaleApplyButton)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Apply";
        }
    }
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
        m_fractureWidth.uiCapability()->setUiName("Fracture Width [m]");
    }
    else if (m_fractureTemplateUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        m_wellDiameter.uiCapability()->setUiName("Well Diameter [inches]");
        m_perforationLength.uiCapability()->setUiName("Perforation Length [ft]");
        m_fractureWidth.uiCapability()->setUiName("Fracture Width [ft]");
    }

    if (m_orientationType == RimFractureTemplate::ALONG_WELL_PATH ||
        m_orientationType == RimFractureTemplate::TRANSVERSE_WELL_PATH)
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

    {
        if (m_fractureWidthType == RimFractureTemplate::USER_DEFINED_WIDTH)
        {
            m_fractureWidth.uiCapability()->setUiReadOnly(false);
        }
        else
        {
            m_fractureWidth.uiCapability()->setUiReadOnly(true);
        }

        if (m_betaFactorType == RimFractureTemplate::USER_DEFINED_BETA_FACTOR)
        {
            m_inertialCoefficient.uiCapability()->setUiReadOnly(false);
        }
        else
        {
            m_inertialCoefficient.uiCapability()->setUiReadOnly(true);
        }
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

QString indentedText(const QString& text)
{
    return QString("  %1\n").arg(text);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureTemplate::dFactorSummary() const
{
    QString text;

    std::vector<RimFracture*> fracturesToDisplay;
    {
        auto candidateFractures = fracturesUsingThisTemplate();

        if (orientationType() != ALONG_WELL_PATH)
        {
            // D-factor values are identical for all fractures, only show summary for the first fracture
            if (!candidateFractures.empty())
            {
                fracturesToDisplay.push_back(candidateFractures.front());
            }
        }
        else
        {
            fracturesToDisplay = candidateFractures;
        }
    }

    for (auto f : fracturesToDisplay)
    {
        f->ensureValidNonDarcyProperties();

        if (orientationType() == ALONG_WELL_PATH)
        {
            text += QString("Fracture name : %1").arg(f->name());
        }

        text += "<pre>";
        {
            auto val = f->nonDarcyProperties().dFactor;
            text += indentedText(QString("D-factor : %1").arg(val));

            auto alpha = RiaDefines::nonDarcyFlowAlpha(m_fractureTemplateUnit());
            text += indentedText(QString("&alpha;  : %1").arg(alpha));

            auto beta = getOrComputeBetaFactor(f);
            text += indentedText(QString("&beta;  : %1").arg(beta));

            double effPerm = f->nonDarcyProperties().effectivePermeability;
            text += indentedText(QString("Ke : %1").arg(effPerm));

            double gamma = m_relativeGasDensity;
            text += indentedText(QString("&gamma;  : %1").arg(gamma));

            auto h = f->nonDarcyProperties().width;
            text += indentedText(QString("h  : %1").arg(h));

            auto wellRadius = f->nonDarcyProperties().eqWellRadius;
            text += indentedText(QString("rw : %1").arg(wellRadius));

            auto mu = m_gasViscosity;
            text += indentedText(QString("&mu;  : %1").arg(mu));
        }
        text += "</pre>";

        text += "<br>";
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::dFactorForTemplate() const
{
    if (orientationType() == ALONG_WELL_PATH)
    {
        return std::numeric_limits<double>::infinity();
    }

    return computeDFactor(nullptr);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::computeEffectivePermeability(const RimFracture* fractureInstance) const
{
    if (m_permeabilityType() == RimFractureTemplate::USER_DEFINED_PERMEABILITY)
    {
        return m_userDefinedEffectivePermeability;
    }
    else
    {
        double fracPermeability = 0.0;
        auto   values           = wellFractureIntersectionData(fractureInstance);
        if (values.isWidthAndPermeabilityDefined())
        {
            fracPermeability = values.m_permeability;
        }
        else
        {
            auto conductivity = values.m_conductivity;
            auto width        = computeFractureWidth(fractureInstance);

            if (fabs(width) < 1e-10) return std::numeric_limits<double>::infinity();

            fracPermeability = conductivity / width;
        }

        return fracPermeability * m_relativePermeability;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::computeWellRadiusForDFactorCalculation(const RimFracture* fractureInstance) const
{
    double radius = 0.0;

    if (m_orientationType == ALONG_WELL_PATH && fractureInstance)
    {
        auto perforationLength = fractureInstance->perforationLength();

        radius = perforationLength / cvf::PI_D;
    }
    else
    {
        radius = m_wellDiameter / 2.0;
    }

    return radius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::computeDFactor(const RimFracture* fractureInstance) const
{
    double d;

    if (m_nonDarcyFlowType == RimFractureTemplate::NON_DARCY_USER_DEFINED)
    {
        d = m_userDefinedDFactor;
    }
    else
    {
        double radius  = computeWellRadiusForDFactorCalculation(fractureInstance);
        double alpha   = RiaDefines::nonDarcyFlowAlpha(m_fractureTemplateUnit());
        double beta    = getOrComputeBetaFactor(fractureInstance);
        double effPerm = computeEffectivePermeability(fractureInstance);
        double gamma   = m_relativeGasDensity;

        double mu = m_gasViscosity;
        double h  = computeFractureWidth(fractureInstance);

        double numerator   = alpha * beta * effPerm * gamma;
        double denumerator = h * radius * mu;

        if (denumerator < 1e-10) return std::numeric_limits<double>::infinity();

        d = numerator / denumerator;

        if (m_orientationType == ALONG_WELL_PATH)
        {
            // Correction for linear inflow into the well
            // Dlinear = cgeometric * Dradial
            // Dlinear = 1.2 * Dradial
            d *= 1.2;
        }
    }

    return d * m_dFactorScaleFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::computeKh(const RimFracture* fractureInstance) const
{
    // kh           = permeability * h
    // conductivity = permeability * h

    auto values = wellFractureIntersectionData(fractureInstance);
    if (values.isConductivityDefined())
    {
        // If conductivity is found in stim plan file, use this directly
        return values.m_conductivity;
    }

    return computeEffectivePermeability(fractureInstance) * computeFractureWidth(fractureInstance);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::convertToUnitSystem(RiaEclipseUnitTools::UnitSystem neededUnit)
{
    if (neededUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        m_perforationLength = RiaEclipseUnitTools::feetToMeter(m_perforationLength);
        m_wellDiameter      = RiaEclipseUnitTools::inchToMeter(m_wellDiameter);
        m_fractureWidth     = RiaEclipseUnitTools::feetToMeter(m_fractureWidth);
    }
    else if (neededUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        m_perforationLength = RiaEclipseUnitTools::meterToFeet(m_perforationLength);
        m_wellDiameter      = RiaEclipseUnitTools::meterToInch(m_wellDiameter);
        m_fractureWidth     = RiaEclipseUnitTools::meterToFeet(m_fractureWidth);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::disconnectAllFracturesAndRedrawViews() const
{
    // The unit has changed. Disconnect all fractures referencing this fracture template to avoid mix of units between fracture
    // and template

    for (auto fracture : fracturesUsingThisTemplate())
    {
        if (fracture)
        {
            fracture->setFractureTemplate(nullptr);
        }
    }

    RimProject* proj;
    this->firstAncestorOrThisOfType(proj);
    if (proj)
    {
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::setId(int id)
{
    m_id = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::setScaleFactors(double halfLengthScale,
                                          double heightScale,
                                          double dFactorScale,
                                          double conductivityScale)
{
    m_halfLengthScaleFactor   = halfLengthScale;
    m_heightScaleFactor       = heightScale;
    m_dFactorScaleFactor      = dFactorScale;
    m_conductivityScaleFactor = conductivityScale;

    for (RimFracture* fracture : fracturesUsingThisTemplate())
    {
        fracture->clearCachedNonDarcyProperties();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::scaleFactors(double* halfLengthScale,
                                       double* heightScale,
                                       double* dFactorScale,
                                       double* conductivityScale) const
{
    CVF_ASSERT(halfLengthScale && heightScale && dFactorScale && conductivityScale);

    *halfLengthScale   = m_halfLengthScaleFactor;
    *heightScale       = m_heightScaleFactor;
    *dFactorScale      = m_dFactorScaleFactor;
    *conductivityScale = m_conductivityScaleFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::setContainmentTopKLayer(int topKLayer)
{
    m_fractureContainment->setTopKLayer(topKLayer);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::setContainmentBaseKLayer(int baseKLayer)
{
    m_fractureContainment->setBaseKLayer(baseKLayer);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::computeFractureWidth(const RimFracture* fractureInstance) const
{
    if (m_fractureWidthType == RimFractureTemplate::WIDTH_FROM_FRACTURE)
    {
        auto values = wellFractureIntersectionData(fractureInstance);

        return values.m_width;
    }

    return m_fractureWidth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::getOrComputeBetaFactor(const RimFracture* fractureInstance) const
{
    if (m_betaFactorType == RimFractureTemplate::BETA_FACTOR_FROM_FRACTURE)
    {
        auto values = wellFractureIntersectionData(fractureInstance);

        return values.m_betaFactorInForcheimerUnits;
    }

    return m_inertialCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::loadDataAndUpdateGeometryHasChanged()
{
    onLoadDataAndUpdateGeometryHasChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFracture*> RimFractureTemplate::fracturesUsingThisTemplate() const
{
    std::vector<RimFracture*> fractures;
    this->objectsWithReferringPtrFieldsOfType(fractures);

    return fractures;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureTemplate::isBetaFactorAvailableOnFile() const
{
    return false;
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
double RimFractureTemplate::wellDiameter() const
{
    return m_wellDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::perforationLength() const
{
    return m_perforationLength;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimFractureContainment* RimFractureTemplate::fractureContainment() const
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
