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

#include "RicExportCarfinUi.h"

#include "RicCellRangeUi.h"

#include "RimEclipseCase.h"
#include "RimTools.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafVecIjk.h"


CAF_PDM_SOURCE_INIT(RicExportCarfinUi, "RicExportCarfinUi");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportCarfinUi::RicExportCarfinUi()
{
    CAF_PDM_InitObject("Export CARFIN", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_cellRange, "CellRange", "CellRange", "", "", "");
    m_cellRange = new RicCellRangeUi;

    CAF_PDM_InitFieldNoDefault(&m_exportFileName, "ExportFileName", "Export FileName", "", "", "");
    m_exportFileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_caseToApply, "CaseToApply", "Source Case", "", "", "");

    CAF_PDM_InitField(&m_cellCountI,    "CellCountI",   2, "Cell Count I", "", "", "");
    CAF_PDM_InitField(&m_cellCountJ,    "CellCountJ",   2, "Cell Count J", "", "", "");
    CAF_PDM_InitField(&m_cellCountK,    "CellCountK",   2, "Cell Count K", "", "", "");
    CAF_PDM_InitField(&m_maxWellCount,  "MaxWellCount", 8, "Max Well Count", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCarfinUi::setCase(RimEclipseCase* rimCase)
{
    m_caseToApply = rimCase;
    m_cellRange->setCase(rimCase);

    if (rimCase)
    {
        QString caseFolder = rimCase->locationOnDisc();
        m_exportFileName = caseFolder + "/carfin.data";
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RicExportCarfinUi::maxWellCount() const
{
    return m_maxWellCount();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::VecIjk RicExportCarfinUi::lgrCellCount() const
{
    return caf::VecIjk{m_cellCountI, m_cellCountJ, m_cellCountK};
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RicCellRangeUi* RicExportCarfinUi::cellRange() const
{
    return m_cellRange();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicExportCarfinUi::exportFileName() const
{
    return m_exportFileName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicExportCarfinUi::caseToApply() const
{
    return m_caseToApply();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicExportCarfinUi::gridName() const
{
    return m_cellRange->gridName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicExportCarfinUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
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
void RicExportCarfinUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_caseToApply)
    {
        setCase(m_caseToApply);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCarfinUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_exportFileName);
    uiOrdering.add(&m_caseToApply);
    
    caf::PdmUiGroup* sourceGridBox = uiOrdering.addNewGroup("Source Grid Box");
    m_cellRange->uiOrdering(uiConfigName, *sourceGridBox);

    caf::PdmUiGroup* gridRefinement = uiOrdering.addNewGroup("Grid Refinement");
    gridRefinement->add(&m_cellCountI);
    gridRefinement->add(&m_cellCountJ);
    gridRefinement->add(&m_cellCountK);
    gridRefinement->add(&m_maxWellCount);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCarfinUi::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_exportFileName)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectSaveFileName = true;
        }
    }
}

