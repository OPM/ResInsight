/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimPerforationCollection.h"

#include "RimEclipseWell.h"
#include "RimPerforationInterval.h"
#include "RimView.h"
#include "RimProject.h"

#include "RigWellPath.h"

#include "RifWellPathImporter.h"

#include "RiuMainWindow.h"


CAF_PDM_SOURCE_INIT(RimPerforationCollection, "PerforationCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPerforationCollection::RimPerforationCollection()
{
    CAF_PDM_InitObject("Perforations", ":/PerforationIntervals16x16.png", "", "");

    nameField()->uiCapability()->setUiHidden(true);
    this->setName("Perforations");

    CAF_PDM_InitFieldNoDefault(&m_perforations, "Perforations", "Perforations", "", "", "");
    m_perforations.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPerforationCollection::~RimPerforationCollection()
{
    m_perforations.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPerforationCollection::appendPerforation(RimPerforationInterval* perforation)
{
    m_perforations.push_back(perforation);

    perforation->setUnitSystemSpecificDefaults();

    updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(perforation);

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<const RimPerforationInterval*> RimPerforationCollection::perforations() const
{
    std::vector<const RimPerforationInterval*> myPerforations;

    for (auto perforation : m_perforations)
    {
        myPerforations.push_back(perforation);
    }

    return myPerforations;
}

