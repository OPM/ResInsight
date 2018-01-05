/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicSaveEclipseInputVisibleCellsUi.h"

#include "RiaApplication.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiOrdering.h"

#include <QDir>

namespace caf {

    template<>
    void RicSaveEclipseInputVisibleCellsUi::ExportKeywordEnum::setUp()
    {
        addItem(RicSaveEclipseInputVisibleCellsUi::FLUXNUM, "FLUXNUM", "FLUXNUM");
        addItem(RicSaveEclipseInputVisibleCellsUi::MULTNUM, "MULTNUM", "MULTNUM");
        setDefault(RicSaveEclipseInputVisibleCellsUi::FLUXNUM);
    }
}

CAF_PDM_SOURCE_INIT(RicSaveEclipseInputVisibleCellsUi, "RicSaveEclipseInputVisibleCellsUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSaveEclipseInputVisibleCellsUi::RicSaveEclipseInputVisibleCellsUi(void) : exportFilenameManuallyChanged(false)
{
    CAF_PDM_InitObject("Export Visible Cells FLUXNUM/MULTNUM", "", "", "");

    CAF_PDM_InitField(&exportFilename,          "ExportFilename",          QString(), "Export Filename", "", "", "");
    exportFilename.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    CAF_PDM_InitFieldNoDefault(&exportKeyword,  "ExportKeyword",                      "Export Keyword", "", "", "");
    CAF_PDM_InitField(&visibleActiveCellsValue, "VisibleActiveCellsValue", 1,         "Visible Active Cells Value", "", "", "");
    CAF_PDM_InitField(&hiddenActiveCellsValue,  "HiddenActiveCellsValue",  0,         "Hidden Active Cells Value", "", "", "");
    CAF_PDM_InitField(&inactiveCellsValue,      "InactiveCellsValue",      0,         "Inactive Cells Value", "", "", "");

    exportFilename = getDefaultExportPath();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSaveEclipseInputVisibleCellsUi::~RicSaveEclipseInputVisibleCellsUi()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseInputVisibleCellsUi::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if (field == &exportFilename)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectSaveFileName = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseInputVisibleCellsUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&exportFilename);
    uiOrdering.add(&exportKeyword);
    uiOrdering.add(&visibleActiveCellsValue);
    uiOrdering.add(&hiddenActiveCellsValue);
    uiOrdering.add(&inactiveCellsValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseInputVisibleCellsUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &exportFilename)
    {
        exportFilenameManuallyChanged = true;
    }
    else if (changedField == &exportKeyword)
    {
        if (!exportFilenameManuallyChanged)
        {
            exportFilename = getDefaultExportPath();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicSaveEclipseInputVisibleCellsUi::getDefaultExportPath() const
{
    QDir baseDir(RiaApplication::instance()->currentProjectPath());
    return baseDir.absoluteFilePath(QString("%1.grdecl").arg(exportKeyword().text()));
}
