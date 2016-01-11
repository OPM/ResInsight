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

#include "RimMainPlotCollection.h"
#include "RimWellLogPlotCollection.h"

#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"

CAF_PDM_SOURCE_INIT(RimMainPlotCollection, "MainPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection::RimMainPlotCollection()
{
    CAF_PDM_InitObject("Plots", "", "", "");

    CAF_PDM_InitField(&show, "Show", true, "Show plots", "", "", "");
    show.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellLogPlotCollection, "WellLogPlotCollection", "",  "", "", "");
    m_wellLogPlotCollection.uiCapability()->setUiHidden(true);

    m_wellLogPlotCollection = new RimWellLogPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMainPlotCollection::~RimMainPlotCollection()
{
    if (m_wellLogPlotCollection()) delete m_wellLogPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMainPlotCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimMainPlotCollection::objectToggleField()
{
    return &show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection* RimMainPlotCollection::wellLogPlotCollection()
{
    return m_wellLogPlotCollection();
}
