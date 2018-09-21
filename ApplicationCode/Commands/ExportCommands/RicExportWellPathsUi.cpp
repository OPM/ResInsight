/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RicExportWellPathsUi.h"

#include "RiaApplication.h"
#include "RiaOptionItemFactory.h"

#include "RimCase.h"
#include "RimGridView.h"
#include "RimProject.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT(RicExportWellPathsUi, "RicExportWellPathsUi");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportWellPathsUi::RicExportWellPathsUi()
{
    CAF_PDM_InitObject("Resample LAS curves for export", "", "", "");

    CAF_PDM_InitField(&m_exportFolder, "ExportFolder", QString(), "Export Folder", "", "", "");
    m_exportFolder.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_mdStepSize, "MdStepSize", 5.0, "MD Step Size", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportWellPathsUi::setMdStepSize(double mdStepSize)
{
    m_mdStepSize = mdStepSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportWellPathsUi::exportFolder() const
{
    return m_exportFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicExportWellPathsUi::mdStepSize() const
{
    return m_mdStepSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
//QList<caf::PdmOptionItemInfo> RicExportWellPathsUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
//                                                                                 bool*                      useOptionsOnly)
//{
//    QList<caf::PdmOptionItemInfo> options;
//
//    if (fieldNeedingOptions == &m_mdStepSize)
//    {
//        std::vector<RimGridView*> visibleViews;
//        RiaApplication::instance()->project()->allVisibleGridViews(visibleViews);
//
//        for (RimGridView* v : visibleViews)
//        {
//            RiaOptionItemFactory::appendOptionItemFromViewNameAndCaseName(v, &options);
//        }
//    }
//
//    return options;
//}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportWellPathsUi::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                        QString                    uiConfigName,
                                                        caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_exportFolder)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectDirectory = true;
        }
    }
}
