/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicExportCarfinForCompletionsUi.h"

#include "RicCellRangeUi.h"

#include "RimEclipseCase.h"
#include "RimTools.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafVecIjk.h"


CAF_PDM_SOURCE_INIT(RicExportCarfinForCompletionsUi, "RicExportCarfinForCompletionsUi");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportCarfinForCompletionsUi::RicExportCarfinForCompletionsUi()
{
    CAF_PDM_InitObject("Export CARFIN", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_exportFolder, "ExportFolder", "Export Folder", "", "", "");
    m_exportFolder.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_caseToApply, "CaseToApply", "Source Case", "", "", "");

    CAF_PDM_InitField(&m_cellCountI,    "CellCountI",   2, "Cell Count I", "", "", "");
    CAF_PDM_InitField(&m_cellCountJ,    "CellCountJ",   2, "Cell Count J", "", "", "");
    CAF_PDM_InitField(&m_cellCountK,    "CellCountK",   2, "Cell Count K", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCarfinForCompletionsUi::setCase(RimEclipseCase* rimCase)
{
    bool isDifferent = (rimCase != m_caseToApply);

    if (isDifferent)
    {
        m_caseToApply = rimCase;
        setDefaultValuesFromCase();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::VecIjk RicExportCarfinForCompletionsUi::lgrCellCount() const
{
    return caf::VecIjk (m_cellCountI, m_cellCountJ, m_cellCountK);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicExportCarfinForCompletionsUi::exportFolder() const
{
    return m_exportFolder();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicExportCarfinForCompletionsUi::caseToApply() const
{
    return m_caseToApply();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCarfinForCompletionsUi::setExportFolder(const QString& folder)
{
    m_exportFolder = folder;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCarfinForCompletionsUi::setDefaultValuesFromCase()
{
    if (m_caseToApply)
    {
        QString caseFolder = m_caseToApply->locationOnDisc();
        m_exportFolder = caseFolder;
    }

    m_cellCountI = 2;
    m_cellCountJ = 2;
    m_cellCountK = 2;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicExportCarfinForCompletionsUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_caseToApply)
    {
        RimTools::caseOptionItems(&options);
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCarfinForCompletionsUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_caseToApply)
    {
        setDefaultValuesFromCase();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCarfinForCompletionsUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_caseToApply);
    uiOrdering.add(&m_exportFolder);
    
    caf::PdmUiGroup* gridRefinement = uiOrdering.addNewGroup("Grid Refinement");
    gridRefinement->add(&m_cellCountI);
    gridRefinement->add(&m_cellCountJ);
    gridRefinement->add(&m_cellCountK);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCarfinForCompletionsUi::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_exportFolder)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectSaveFileName = true;
        }
    }
}

