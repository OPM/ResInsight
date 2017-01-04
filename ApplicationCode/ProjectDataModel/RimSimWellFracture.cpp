/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimSimWellFracture.h"

#include "RiaApplication.h"

#include "RimFractureDefinition.h"
#include "RimFractureDefinitionCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmUiItem.h"

#include "QToolBox"
#include "QList"
#include "cvfVector3.h"
#include "RigTesselatorTools.h"


CAF_PDM_SOURCE_INIT(RimSimWellFracture, "SimWellFracture");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellFracture::RimSimWellFracture(void)
{
    CAF_PDM_InitObject("SimWellFracture", "", "", "");

    CAF_PDM_InitField(&name,    "UserDescription", QString("Fracture Name"), "Name", "", "", "");
    
    CAF_PDM_InitField(&m_i,               "I",                1,      "Fracture location cell I", "", "", "");
    CAF_PDM_InitField(&m_j,               "J",                1,      "Fracture location cell J", "", "", "");
    CAF_PDM_InitField(&m_k,               "K",                1,      "Fracture location cell K", "", "", "");

    CAF_PDM_InitFieldNoDefault(&fractureDefinition, "FractureDef", "FractureDef", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellFracture::~RimSimWellFracture()
{
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSimWellFracture::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{

    QList<caf::PdmOptionItemInfo> options;

    RimProject* proj = RiaApplication::instance()->project();
    CVF_ASSERT(proj);

    RimOilField* oilField = proj->activeOilField();
    if (oilField == nullptr) return options;

    if (fieldNeedingOptions == &fractureDefinition)
    {

        RimFractureDefinitionCollection* fracDefColl = oilField->fractureDefinitionCollection();
        if (fracDefColl == nullptr) return options;

        for (RimFractureDefinition* fracDef : fracDefColl->fractureDefinitions())
        {
            options.push_back(caf::PdmOptionItemInfo(fracDef->name(), fracDef));
        }
    }

    return options;



}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimSimWellFracture::centerPointForFracture()
{
    // TODO: Find center point of cell
    return cvf::Vec3d::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureDefinition* RimSimWellFracture::attachedFractureDefinition()
{
    return fractureDefinition();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::setijk(size_t i, size_t j, size_t k)
{
    m_i = static_cast<int>(i + 1);
    m_j = static_cast<int>(j + 1);
    m_k = static_cast<int>(k + 1);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Fractures");
    geometryGroup->add(&fractureDefinition);
   
    geometryGroup->add(&m_i);
    geometryGroup->add(&m_j);
    geometryGroup->add(&m_k);

    uiOrdering.setForgetRemainingFields(true);

}
