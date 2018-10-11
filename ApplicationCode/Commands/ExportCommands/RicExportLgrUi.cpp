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

#include "RicExportLgrUi.h"

#include "RicCellRangeUi.h"

#include "RimEclipseCase.h"
#include "RimTools.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafVecIjk.h"


CAF_PDM_SOURCE_INIT(RicExportLgrUi, "RicExportLgrUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
namespace caf
{
    template<>
    void RicExportLgrUi::LgrSplitTypeEnum::setUp()
    {
        addItem(RicExportLgrUi::PER_CELL_LGR, "PER_CELL_LGR", "LGR Per Cell");
        addItem(RicExportLgrUi::SINGLE_LGR, "SINGLE_LGR", "Single LGR");

        setDefault(RicExportLgrUi::PER_CELL_LGR);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportLgrUi::RicExportLgrUi()
{
    CAF_PDM_InitObject("Export CARFIN", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_exportFolder, "ExportFolder", "Export Folder", "", "", "");
    m_exportFolder.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_caseToApply, "CaseToApply", "Source Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_timeStep, "TimeStepIndex", "Time Step", "", "", "");

    CAF_PDM_InitField(&m_cellCountI,    "CellCountI",   2, "Cell Count I", "", "", "");
    CAF_PDM_InitField(&m_cellCountJ,    "CellCountJ",   2, "Cell Count J", "", "", "");
    CAF_PDM_InitField(&m_cellCountK,    "CellCountK",   2, "Cell Count K", "", "", "");

    CAF_PDM_InitField(&m_splitType,     "SplitType", LgrSplitTypeEnum(), "Split Type", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::setCase(RimEclipseCase* rimCase)
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
void RicExportLgrUi::setTimeStep(int timeStep)
{
    bool isDifferent = timeStep != m_timeStep;

    if (isDifferent)
    {
        m_timeStep = timeStep;
        setDefaultValuesFromCase();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::VecIjk RicExportLgrUi::lgrCellCount() const
{
    return caf::VecIjk (m_cellCountI, m_cellCountJ, m_cellCountK);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicExportLgrUi::exportFolder() const
{
    return m_exportFolder();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicExportLgrUi::caseToApply() const
{
    return m_caseToApply();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RicExportLgrUi::timeStep() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportLgrUi::singleLgrSplit() const
{
    return m_splitType == SINGLE_LGR;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::setExportFolder(const QString& folder)
{
    m_exportFolder = folder;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::setDefaultValuesFromCase()
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
QList<caf::PdmOptionItemInfo> RicExportLgrUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_caseToApply)
    {
        RimTools::caseOptionItems(&options);
    }
    else if (fieldNeedingOptions == &m_timeStep)
    {
        QStringList timeStepNames;

        if (m_caseToApply)
        {
            timeStepNames = m_caseToApply->timeStepStrings();
        }
        for (int i = 0; i < timeStepNames.size(); i++)
        {
            options.push_back(caf::PdmOptionItemInfo(timeStepNames[i], i));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_caseToApply)
    {
        setDefaultValuesFromCase();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_caseToApply);
    uiOrdering.add(&m_timeStep);
    uiOrdering.add(&m_exportFolder);
    
    caf::PdmUiGroup* gridRefinement = uiOrdering.addNewGroup("Grid Refinement");
    gridRefinement->add(&m_cellCountI);
    gridRefinement->add(&m_cellCountJ);
    gridRefinement->add(&m_cellCountK);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportLgrUi::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
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

