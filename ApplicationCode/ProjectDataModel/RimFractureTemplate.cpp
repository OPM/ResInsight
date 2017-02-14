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

#include "cvfVector3.h"



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
        addItem(RimFractureTemplate::INFINITE_CONDUCTIVITY, "InfiniteConductivity", "Infinite conductivity in fracture");
        addItem(RimFractureTemplate::FINITE_CONDUCTIVITY, "FiniteConductivity", "Finite conductivity in fracture");

        setDefault(RimFractureTemplate::INFINITE_CONDUCTIVITY);
    }


    template<>

    void caf::AppEnum< RimFractureTemplate::FracUnitEnum>::setUp()
    {
        addItem(RimFractureTemplate::UNITS_METRIC, "MetricUnits", "Metric");
        addItem(RimFractureTemplate::UNITS_FIELD, "FieldsUnits", "Field units");

        setDefault(RimFractureTemplate::UNITS_METRIC);
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
    CAF_PDM_InitField(&fractureTemplateUnit, "fractureTemplateUnit", caf::AppEnum<FracUnitEnum>(UNITS_METRIC), "Units used in frac template", "", "", "");
    fractureTemplateUnit.uiCapability()->setUiReadOnly(true);


    CAF_PDM_InitField(&orientation, "Orientation",      caf::AppEnum<FracOrientationEnum>(TRANSVERSE_WELL_PATH), "Fracture orientation", "", "", "");
    CAF_PDM_InitField(&azimuthAngle, "AzimuthAngle",    0.0f, "Azimuth Angle", "", "", ""); //Is this correct description?
    CAF_PDM_InitField(&skinFactor, "SkinFactor", 1.0f, "Skin Factor", "", "", "");

    CAF_PDM_InitField(&fractureConductivity, "FractureCondictivity", caf::AppEnum<FracConductivityEnum>(INFINITE_CONDUCTIVITY), "Fracture conductivity", "", "", "");

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

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{

    if (orientation == RimFractureTemplate::ALONG_WELL_PATH
        || orientation == RimFractureTemplate::TRANSVERSE_WELL_PATH)
    {
        azimuthAngle.uiCapability()->setUiHidden(true);
    }
    else if (orientation == RimFractureTemplate::AZIMUTH)
    {
        azimuthAngle.uiCapability()->setUiHidden(false);
    }
}
