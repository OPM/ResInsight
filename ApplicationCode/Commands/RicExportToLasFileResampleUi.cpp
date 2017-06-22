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

#include "RicExportToLasFileResampleUi.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiOrdering.h"

#include <cmath>


CAF_PDM_SOURCE_INIT(RicExportToLasFileObj, "RicExportToLasFileObj");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportToLasFileObj::RicExportToLasFileObj(void)
{
    CAF_PDM_InitObject("RicExportToLasFileObj", "", "", "");

    CAF_PDM_InitField(&tvdrkbOffset, "tvdrkbOffset", QString(""), "TVDRKB offset (RKB - MSL) [m]", "", "", "");
}

CAF_PDM_SOURCE_INIT(RicExportToLasFileResampleUi, "RicExportToLasFileResampleUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportToLasFileResampleUi::RicExportToLasFileResampleUi(void)
{
    CAF_PDM_InitObject("Resample LAS curves for export", "", "", "");

    CAF_PDM_InitField(&exportFolder,     "ExportFolder",     QString(), "Export Folder", "", "", "");
    exportFolder.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitField(&activateResample, "ActivateResample", false,     "Resample Curve Data", "", "", "");
    activateResample.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&resampleInterval, "ResampleInterval", 1.0,       "Resample Interval [m]", "", "", "");

    CAF_PDM_InitField(&exportTvdrkb,    "ExportTvdrkb",     false,      "Export TVDRKB", "", "", "");
    exportTvdrkb.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    
    CAF_PDM_InitFieldNoDefault(&m_tvdrkbOffsets, "tvdrkbOffsets", "", "", "", "");

    updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportToLasFileResampleUi::~RicExportToLasFileResampleUi()
{
    m_tvdrkbOffsets.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileResampleUi::tvdrkbDiffForWellPaths(std::vector<double>* rkbDiffs)
{
    for (size_t i = 0; i < m_tvdrkbOffsets.size(); i++)
    {
        double value = HUGE_VAL;
        if (!m_tvdrkbOffsets()[i]->tvdrkbOffset().isEmpty())
        {
            value = m_tvdrkbOffsets()[i]->tvdrkbOffset().toDouble();
        }
        rkbDiffs->push_back(value);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileResampleUi::setRkbDiffs(const std::vector<QString>& wellNames, const std::vector<double>& rkbDiffs)
{
    for (size_t i = 0; i < wellNames.size(); i++)
    {
        RicExportToLasFileObj* obj = new RicExportToLasFileObj;
        if (rkbDiffs[i] != HUGE_VAL)
        {
            obj->tvdrkbOffset = QString::number(rkbDiffs[i]);
        }
        obj->tvdrkbOffset.uiCapability()->setUiName(wellNames[i]);

        m_tvdrkbOffsets.push_back(obj);
    }

    updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileResampleUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileResampleUi::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &exportFolder)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectDirectory = true;
        }
    }

    if (field == &exportTvdrkb || field == &activateResample)
    {
        caf::PdmUiCheckBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiCheckBoxEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_useNativeCheckBoxLabel = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileResampleUi::updateFieldVisibility()
{
    if (activateResample)
    {
        resampleInterval.uiCapability()->setUiReadOnly(false);
    }
    else
    {
        resampleInterval.uiCapability()->setUiReadOnly(true);
    }

    for (RicExportToLasFileObj* obj : m_tvdrkbOffsets)
    {
        obj->tvdrkbOffset.uiCapability()->setUiReadOnly(!exportTvdrkb);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileResampleUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&exportFolder);
    
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Resampling");

        group->add(&activateResample);
        group->add(&resampleInterval);
    }

    
    caf::PdmUiGroup* tvdrkbGroup = uiOrdering.addNewGroup("TVDRKB");
    tvdrkbGroup->add(&exportTvdrkb);

    caf::PdmUiGroup* group = tvdrkbGroup->addNewGroup("Difference between TVDRKB and TVDMSL, clear diff for no export");
    for (auto& obj : m_tvdrkbOffsets)
    {
        group->add(&obj->tvdrkbOffset);
    }
}

