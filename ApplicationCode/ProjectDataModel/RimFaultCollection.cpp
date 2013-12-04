/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "RimFaultCollection.h"

#include "cafAppEnum.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "RimReservoirView.h"

#include "RimResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellEdgeResultSlot.h"
#include "RiaApplication.h"
#include "RiaPreferences.h"



CAF_PDM_SOURCE_INIT(RimFaultCollection, "Faults");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFaultCollection::RimFaultCollection()
{
    CAF_PDM_InitObject("Faults", ":/draw_style_faults_24x24.png", "", "");

    CAF_PDM_InitField(&showFaultCollection,              "Active",        true,   "Active", "", "", "");
    showFaultCollection.setUiHidden(true);

    CAF_PDM_InitField(&showGeometryDetectedFaults,       "ShowGeometryDetectedFaults",    true,   "Show geometry detected faults", "", "", "");

    CAF_PDM_InitField(&showFaultLabel,       "ShowFaultLabel",    true,   "Show fault labels", "", "", "");
    cvf::Color3f defWellLabelColor = RiaApplication::instance()->preferences()->defaultWellLabelColor();
    CAF_PDM_InitField(&faultLabelColor,      "FaultLabelColor",   defWellLabelColor, "Fault label color",  "", "", "");

    CAF_PDM_InitFieldNoDefault(&faults, "Faults", "Faults",  "", "", "");

    m_reservoirView = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFaultCollection::~RimFaultCollection()
{
   faults.deleteAllChildObjects();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&showFaultCollection == changedField)
    {
        this->updateUiIconFromState(showFaultCollection);

        if (m_reservoirView) 
        {
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }

    if (&showGeometryDetectedFaults == changedField)
    {
        if (m_reservoirView) 
        {
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultCollection::setReservoirView(RimReservoirView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFaultCollection::objectToggleField()
{
    return &showFaultCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFault* RimFaultCollection::findFaultByName(QString name)
{
    for (size_t i = 0; i < this->faults().size(); ++i)
    {
        if (this->faults()[i]->name() == name)
        {
            return this->faults()[i];
        }
    }
    return NULL;
}

