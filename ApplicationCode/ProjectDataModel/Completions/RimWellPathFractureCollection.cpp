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

#include "RimWellPathFractureCollection.h"

#include "RimProject.h"
#include "RimWellPathFracture.h"

#include "cafPdmObject.h"


namespace caf {
    template<>
    void RimWellPathFractureCollection::ReferenceMDEnum::setUp()
    {
        addItem(RimWellPathFractureCollection::AUTO_REFERENCE_MD, "GridIntersectionRefMD", "Use depth where the well path meets grid");
        addItem(RimWellPathFractureCollection::MANUAL_REFERENCE_MD, "ManualRefMD", "Set Manually");
        setDefault(RimWellPathFractureCollection::AUTO_REFERENCE_MD);
    }
}


CAF_PDM_SOURCE_INIT(RimWellPathFractureCollection, "WellPathFractureCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection::RimWellPathFractureCollection(void)
{
    CAF_PDM_InitObject("Fractures", ":/FractureLayout16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_fractures, "Fractures", "", "", "", "");
    m_fractures.uiCapability()->setUiHidden(true);

    setName("Fractures");
    nameField()->uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_refMDType, "RefMDType", "Reference Depth", "", "", "");
    CAF_PDM_InitField(&m_refMD, "RefMD", 0.0, "Reference MD", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_mswParameters, "MswParameters", "Multi Segment Well Parameters", "", "", "");
    m_mswParameters = new RimMswCompletionParameters;
    m_mswParameters.uiCapability()->setUiTreeHidden(true);
    m_mswParameters.uiCapability()->setUiTreeChildrenHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection::~RimWellPathFractureCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimMswCompletionParameters* RimWellPathFractureCollection::mswParameters() const
{
    return m_mswParameters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFractureCollection::addFracture(RimWellPathFracture* fracture)
{
    m_fractures.push_back(fracture);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFractureCollection::deleteFractures()
{
    m_fractures.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFractureCollection::setUnitSystemSpecificDefaults()
{
    m_mswParameters->setUnitSystemSpecificDefaults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection::ReferenceMDType RimWellPathFractureCollection::referenceMDType() const
{
    return m_refMDType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathFractureCollection::manualReferenceMD() const
{
    if (m_refMDType == AUTO_REFERENCE_MD)
    {
        return std::numeric_limits<double>::infinity();
    }
    return m_refMD;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathFracture*> RimWellPathFractureCollection::fractures() const
{
    return m_fractures.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathFracture*> RimWellPathFractureCollection::activeFractures() const
{
    std::vector<RimWellPathFracture*> active;

    if (isChecked())
    {
        for (const auto& f : fractures())
        {
            if (f->isChecked())
            {
                active.push_back(f);
            }
        }
    }

    return active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFractureCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* mswGroup = uiOrdering.addNewGroup("Multi Segment Well Properties");

    mswGroup->add(&m_refMDType);
    mswGroup->add(&m_refMD);
    m_refMD.uiCapability()->setUiHidden(m_refMDType == AUTO_REFERENCE_MD);

    m_mswParameters->uiOrdering(uiConfigName, *mswGroup);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFractureCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    if (changedField == &m_isChecked)
    {
        proj->reloadCompletionTypeResultsInAllViews();
    }
    else
    {
        proj->createDisplayModelAndRedrawAllViews();
    }
}
