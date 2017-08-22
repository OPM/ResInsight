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

#include "RigTesselatorTools.h"

#include "RimFracture.h"
#include "RimProject.h"

#include "cafPdmObject.h"
#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfVector3.h"
#include "RimFractureContainment.h"


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


}


CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimFractureTemplate, "RimFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate::RimFractureTemplate(void)
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&name,        "UserDescription",  QString("Fracture Template"), "Name", "", "", "");
    CAF_PDM_InitField(&fractureTemplateUnit, "fractureTemplateUnit", caf::AppEnum<RiaEclipseUnitTools::UnitSystem>(RiaEclipseUnitTools::UNITS_METRIC), "Units System", "", "", "");
    fractureTemplateUnit.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&orientationType, "Orientation",      caf::AppEnum<FracOrientationEnum>(TRANSVERSE_WELL_PATH), "Fracture Orientation", "", "", "");
    CAF_PDM_InitField(&azimuthAngle, "AzimuthAngle",    0.0f, "Azimuth Angle", "", "", ""); //Is this correct description?
    CAF_PDM_InitField(&skinFactor, "SkinFactor", 0.0f, "Skin Factor", "", "", "");

    CAF_PDM_InitField(&perforationLength, "PerforationLength", 1.0, "Perforation Length", "", "", "");
    CAF_PDM_InitField(&perforationEfficiency, "perforationEfficiency", 1.0, "perforation Efficiency", "", "", "");
    perforationEfficiency.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    CAF_PDM_InitField(&wellDiameter, "wellDiameter", 0.216, "Well Diameter at Fracture", "", "", "");

    CAF_PDM_InitField(&conductivityType, "FractureCondictivity", caf::AppEnum<FracConductivityEnum>(FINITE_CONDUCTIVITY), "Conductivity in Fracture", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_fractureContainment, "fractureContainmentField", "Fracture Containment", "", "", "");
    m_fractureContainment = new RimFractureContainment();
    m_fractureContainment.uiCapability()->setUiTreeHidden(true);
    m_fractureContainment.uiCapability()->setUiTreeChildrenHidden(true);
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
caf::PdmFieldHandle* RimFractureTemplate::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &azimuthAngle || changedField == &orientationType)
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
                    if (changedField == &azimuthAngle && (abs(oldValue.toDouble() - fracture->azimuth()) < 1e-5))
                    {
                        fracture->azimuth = azimuthAngle;
                        fracture->clearDisplayGeometryCache();
                    }

                    if (changedField == &orientationType)
                    {
                        if (newValue == AZIMUTH)
                        {
                            fracture->azimuth = azimuthAngle;
                        }
                        else fracture->updateAzimuthBasedOnWellAzimuthAngle();

                        fracture->clearDisplayGeometryCache();
                    }
                }
            }

            proj->createDisplayModelAndRedrawAllViews();
        }
    }

    if (changedField == &perforationLength || changedField == &perforationEfficiency || changedField == &wellDiameter)
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
                if (changedField == &perforationLength && (abs(oldValue.toDouble() - fracture->perforationLength()) < 1e-5))
                {
                    fracture->perforationLength = perforationLength;
                }
                if (changedField == &perforationEfficiency && (abs(oldValue.toDouble() - fracture->perforationEfficiency()) < 1e-5))
                {
                    fracture->perforationEfficiency = perforationEfficiency;
                }
                if (changedField == &wellDiameter && (abs(oldValue.toDouble() - fracture->wellDiameter()) < 1e-5))
                {
                    fracture->wellDiameter = wellDiameter;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{

    if (fractureTemplateUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        wellDiameter.uiCapability()->setUiName("Well Diameter [m]");
        perforationLength.uiCapability()->setUiName("Perforation Length [m]");
    }
    else if (fractureTemplateUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        wellDiameter.uiCapability()->setUiName("Well Diameter [inches]");
        perforationLength.uiCapability()->setUiName("Perforation Length [Ft]");
    }

    if (orientationType == RimFractureTemplate::ALONG_WELL_PATH
        || orientationType == RimFractureTemplate::TRANSVERSE_WELL_PATH)
    {
        azimuthAngle.uiCapability()->setUiHidden(true);
    }
    else if (orientationType == RimFractureTemplate::AZIMUTH)
    {
        azimuthAngle.uiCapability()->setUiHidden(false);
    }

    if (orientationType == RimFractureTemplate::ALONG_WELL_PATH)
    {
        perforationEfficiency.uiCapability()->setUiHidden(false);
        perforationLength.uiCapability()->setUiHidden(false);
    }
    else 
    {
        perforationEfficiency.uiCapability()->setUiHidden(true);
        perforationLength.uiCapability()->setUiHidden(true);
    }

    if (conductivityType == FINITE_CONDUCTIVITY)
    {
        wellDiameter.uiCapability()->setUiHidden(false);
    }
    else if (conductivityType == INFINITE_CONDUCTIVITY)
    {
        wellDiameter.uiCapability()->setUiHidden(true);
    }
 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
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
double RimFractureTemplate::wellDiameterInFractureUnit(RiaEclipseUnitTools::UnitSystemType fractureUnit)
{
    if (fractureUnit == fractureTemplateUnit())
    {
        return wellDiameter;
    }
    else if (fractureTemplateUnit == RiaEclipseUnitTools::UNITS_METRIC
             && fractureUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        return RiaEclipseUnitTools::meterToInch(wellDiameter);
    }
    else if (fractureTemplateUnit == RiaEclipseUnitTools::UNITS_FIELD
             && fractureUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        return RiaEclipseUnitTools::inchToMeter(wellDiameter);
    }

    return cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimFractureTemplate::perforationLengthInFractureUnit(RiaEclipseUnitTools::UnitSystemType fractureUnit)
{
    if (fractureUnit == fractureTemplateUnit())
    {
        return perforationLength;
    }
    else if (fractureTemplateUnit == RiaEclipseUnitTools::UNITS_METRIC
             && fractureUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        return RiaEclipseUnitTools::meterToFeet(perforationLength);
    }
    else if (fractureTemplateUnit == RiaEclipseUnitTools::UNITS_FIELD
             && fractureUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        return RiaEclipseUnitTools::feetToMeter(perforationLength);
    }

    return cvf::UNDEFINED_DOUBLE;
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
void RimFractureTemplate::setDefaultWellDiameterFromUnit()
{
    if (fractureTemplateUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        wellDiameter = 8.5;
    }
    else if (fractureTemplateUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        wellDiameter = 0.216;
    }
}