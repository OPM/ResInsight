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

#include "RimWellPathFracture.h"

#include "RiaApplication.h"

#include "RigTesselatorTools.h"

#include "RimFractureEllipseDefinition.h"
#include "RimFractureDefinitionCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "RivWellPathPartMgr.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmUiItem.h"

#include "cvfVector3.h"

#include <QToolBox>
#include <QList>



CAF_PDM_SOURCE_INIT(RimWellPathFracture, "WellPathFracture");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFracture::RimWellPathFracture(void)
{
    CAF_PDM_InitObject("Fracture", ":/FractureSymbol16x16.png", "", "");

    CAF_PDM_InitField(&name,    "UserDescription", QString("Fracture Name"), "Name", "", "", "");

    CAF_PDM_InitField(         &measuredDepth,          "MeasuredDepth",        0.0f, "Measured Depth Location (if along well path)", "", "", "");
    CAF_PDM_InitField(         &positionAtWellpath,     "PositionAtWellpath",   cvf::Vec3d::ZERO, "Fracture Position along Well Path", "", "", "");

    CAF_PDM_InitFieldNoDefault(&ui_positionAtWellpath, "ui_positionAtWellpath", "Fracture Position at Well Path", "", "", "");
    ui_positionAtWellpath.registerGetMethod(this, &RimWellPathFracture::fracturePositionForUi);
    ui_positionAtWellpath.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&i,               "I",                1,      "Fracture location cell I", "", "", "");
    CAF_PDM_InitField(&j,               "J",                1,      "Fracture location cell J", "", "", "");
    CAF_PDM_InitField(&k,               "K",                1,      "Fracture location cell K", "", "", "");

    CAF_PDM_InitFieldNoDefault(&fractureDefinition, "FractureDef", "Fracture Template", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFracture::~RimWellPathFracture()
{
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPathFracture::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
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

        for (RimFractureEllipseDefinition* fracDef : fracDefColl->fractureDefinitions())
        {
            options.push_back(caf::PdmOptionItemInfo(fracDef->name(), fracDef));
        }
    }
  
    return options;



}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathFracture::centerPointForFracture()
{
    //return cvf::Vec3d::UNDEFINED;
    return positionAtWellpath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureEllipseDefinition* RimWellPathFracture::attachedFractureDefinition()
{
    return fractureDefinition();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &measuredDepth)
    {
        positionAtWellpath = cvf::Vec3d::ZERO;
        
        caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(this);
        if (!objHandle) return;

        RimWellPath* wellPath = nullptr;
        objHandle->firstAncestorOrThisOfType(wellPath);
        if (!wellPath) return;

        RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();
        positionAtWellpath = wellPathGeometry->interpolatedPointAlongWellPath(measuredDepth);
    }

    setRecomputeGeometryFlag();

    RimProject* proj;
    this->firstAncestorOrThisOfType(proj);
    if (proj) proj->createDisplayModelAndRedrawAllViews();
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellPathFracture::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Fractures");
    geometryGroup->add(&fractureDefinition);

    geometryGroup->add(&measuredDepth);
    geometryGroup->add(&ui_positionAtWellpath);

    uiOrdering.setForgetRemainingFields(true);

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathFracture::fracturePositionForUi() const
{
    cvf::Vec3d v = positionAtWellpath;

    v.z() = -v.z();

    return v;
}
