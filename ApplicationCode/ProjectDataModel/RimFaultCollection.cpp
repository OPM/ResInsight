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

#include "RimCase.h"
#include "RimReservoirCellResultsCacher.h"
#include "RigCaseData.h"
#include "RivColorTableArray.h"


namespace caf
{
    template<>
    void AppEnum< RimFaultCollection::FaultFaceCullingMode >::setUp()
    {
        addItem(RimFaultCollection::FAULT_BACK_FACE_CULLING,  "FAULT_BACK_FACE_CULLING",    "Cell behind fault");
        addItem(RimFaultCollection::FAULT_FRONT_FACE_CULLING, "FAULT_FRONT_FACE_CULLING",   "Cell in front of fault");
        addItem(RimFaultCollection::FAULT_NO_FACE_CULLING,    "FAULT_NO_FACE_CULLING",      "Show both");
        setDefault(RimFaultCollection::FAULT_NO_FACE_CULLING);
    }
}



CAF_PDM_SOURCE_INIT(RimFaultCollection, "Faults");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFaultCollection::RimFaultCollection()
{
    CAF_PDM_InitObject("Faults", ":/draw_style_faults_24x24.png", "", "");

    RiaPreferences* prefs = RiaApplication::instance()->preferences();
    CAF_PDM_InitField(&showFaultCollection,     "Active",        true,   "Active", "", "", "");
    showFaultCollection.setUiHidden(true);

    CAF_PDM_InitField(&showGeometryDetectedFaults,  "ShowGeometryDetectedFaults",    false,   "Show geometry detected faults", "", "", "");
    showGeometryDetectedFaults.setUiHidden(true);

    CAF_PDM_InitField(&showFaultFaces,          "ShowFaultFaces",           true,    "Show defined faces", "", "", "");
    CAF_PDM_InitField(&showOppositeFaultFaces,  "ShowOppositeFaultFaces",   true,    "Show opposite faces", "", "", "");
    CAF_PDM_InitField(&showNNCs,                "ShowNNCs",                 false,   "Show NNCs", "", "", "");
    CAF_PDM_InitField(&showResultsOnFaults,     "ShowResultsOnFaults",      false,   "Show results on faults", "", "", "");
    CAF_PDM_InitField(&showFaultsOutsideFilters,"ShowFaultsOutsideFilters", true,    "Show faults outside filters", "", "", "");

    CAF_PDM_InitField(&faultResult,        "FaultFaceCulling", caf::AppEnum<RimFaultCollection::FaultFaceCullingMode>(RimFaultCollection::FAULT_BACK_FACE_CULLING), "Dynamic Face Selection", "", "", "");

    CAF_PDM_InitField(&showFaultLabel,          "ShowFaultLabel",    false,   "Show labels", "", "", "");
    cvf::Color3f defWellLabelColor = RiaApplication::instance()->preferences()->defaultWellLabelColor();
    CAF_PDM_InitField(&faultLabelColor,         "FaultLabelColor",   defWellLabelColor, "Label color",  "", "", "");

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
    }

    if (&faultLabelColor == changedField)
    {
        m_reservoirView->scheduleReservoirGridGeometryRegen();
    }

    if (&showGeometryDetectedFaults == changedField ||
        &showFaultFaces == changedField ||
        &showOppositeFaultFaces == changedField ||
        &showNNCs == changedField ||
        &showFaultCollection == changedField ||
        &showFaultLabel == changedField ||
        &showFaultsOutsideFilters == changedField ||
        &faultLabelColor == changedField ||
        &faultResult == changedField ||
        &showResultsOnFaults == changedField
        )
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


//--------------------------------------------------------------------------------------------------
/// A comparing function used to sort Faults in the RimFaultCollection::syncronizeFaults() method
//--------------------------------------------------------------------------------------------------
bool faultComparator(const cvf::ref<RigFault>& a, const cvf::ref<RigFault>& b)
{
    CVF_TIGHT_ASSERT(a.notNull() && b.notNull());

    int compareValue = a->name().compare(b->name(), Qt::CaseInsensitive);

    return (compareValue < 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultCollection::syncronizeFaults()
{
    if (!(m_reservoirView && m_reservoirView->eclipseCase() && m_reservoirView->eclipseCase()->reservoirData()) ) return;

    cvf::ref<cvf::Color3fArray> partColors = RivColorTableArray::colorTableArray();

    const cvf::Collection<RigFault> constRigFaults = m_reservoirView->eclipseCase()->reservoirData()->mainGrid()->faults();

    cvf::Collection<RigFault> rigFaults;
    {
        cvf::Collection<RigFault> sortedFaults(constRigFaults);

        std::sort(sortedFaults.begin(), sortedFaults.end(), faultComparator);

        cvf::ref<RigFault> undefinedFaults;
        for (size_t i = 0; i < sortedFaults.size(); i++)
        {
            if (sortedFaults[i]->name().compare(RimDefines::undefinedGridFaultName(), Qt::CaseInsensitive) == 0)
            {
                undefinedFaults = sortedFaults[i];
            }
        }

        if (undefinedFaults.notNull())
        {
            sortedFaults.erase(undefinedFaults.p());

            rigFaults.push_back(undefinedFaults.p());
        }

        for (size_t i = 0; i < sortedFaults.size(); i++)
        {
            rigFaults.push_back(sortedFaults[i].p());
        }
    }


    // Find faults with 

    std::vector<caf::PdmPointer<RimFault> > newFaults;

    // Find corresponding fault from data model, or create a new
    for (size_t fIdx = 0; fIdx < rigFaults.size(); ++fIdx)
    {
        RimFault* rimFault = this->findFaultByName(rigFaults[fIdx]->name());

        if (!rimFault)
        {
            rimFault = new RimFault();
            rimFault->faultColor = partColors->get(fIdx % partColors->size());
        }

        rimFault->setFaultGeometry(rigFaults[fIdx].p());

        newFaults.push_back(rimFault);
    }

    this->faults().clear();
    this->faults().insert(0, newFaults);

    QString toolTip = QString("Fault count (%1)").arg(newFaults.size());
    setUiToolTip(toolTip);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFaultCollection::isGridVisualizationMode() const
{
    CVF_ASSERT(m_reservoirView);

    return  m_reservoirView->isGridVisualizationMode();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    bool isGridVizMode = isGridVisualizationMode();

    faultResult.setUiReadOnly(isGridVizMode);
    showFaultFaces.setUiReadOnly(isGridVizMode);
    showOppositeFaultFaces.setUiReadOnly(isGridVizMode);

    caf::PdmUiGroup* labs = uiOrdering.addNewGroup("Fault Labels");
    labs->add(&showFaultLabel);
    labs->add(&faultLabelColor);

    caf::PdmUiGroup* adv = uiOrdering.addNewGroup("Fault Options");
    adv->add(&showFaultsOutsideFilters);
    adv->add(&showResultsOnFaults);
    adv->add(&showNNCs);

    caf::PdmUiGroup* ffviz = uiOrdering.addNewGroup("Fault Face Visibility");
    ffviz->add(&showFaultFaces);
    ffviz->add(&showOppositeFaultFaces);
    ffviz->add(&faultResult);
}

