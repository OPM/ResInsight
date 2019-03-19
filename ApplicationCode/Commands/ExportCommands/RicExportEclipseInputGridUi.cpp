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

#include "RicExportEclipseInputGridUi.h"

#include "RiaApplication.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

#include "RimEclipseResultDefinition.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiOrdering.h"

#include <QDir>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT(RicExportEclipseInputGridUi, "RicExportEclipseInputGridUi");

namespace caf
{
template<>
void RicExportEclipseInputGridUi::ResultExportOptionsEnum::setUp()
{
    addItem(RicExportEclipseInputGridUi::EXPORT_NO_RESULTS, "NO_RESULTS", "Do not export results");
    addItem(RicExportEclipseInputGridUi::EXPORT_TO_GRID_FILE, "TO_GRID_FILE", "Export to grid file");
    addItem(RicExportEclipseInputGridUi::EXPORT_TO_SINGLE_SEPARATE_FILE, "TO_SINGLE_RESULT_FILE", "Export to single results file");
    addItem(RicExportEclipseInputGridUi::EXPORT_TO_SEPARATE_FILE_PER_RESULT, "TO_SEPARATE_RESULT_FILES", "Export to a separate file per result");

    setDefault(RicExportEclipseInputGridUi::EXPORT_TO_GRID_FILE);
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportEclipseInputGridUi::RicExportEclipseInputGridUi(RigEclipseCaseData* caseData)
    : m_caseData(caseData)
{
    CAF_PDM_InitObject("Export Visible Cells as Eclipse Input Grid", "", "", "");

    CAF_PDM_InitField(&exportGrid, "ExportGrid", true, "Export Grid", "", "Includes COORD, ZCORN and ACTNUM", "");
    CAF_PDM_InitFieldNoDefault(&exportResults, "ExportResults", "Export Results", "", "", "");
    CAF_PDM_InitField(&exportResultsFilename, "ExportResultsFilename", QString(), "Results File Name", "", "", "");
    CAF_PDM_InitField(&exportGridFilename, "ExportGridFilename", QString(), "Grid File Name", "", "", "");
    exportGridFilename.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    exportResultsFilename.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    CAF_PDM_InitFieldNoDefault(&exportMainKeywords, "ExportMainKeywords", "Main Keywords", "", "", "");
    CAF_PDM_InitFieldNoDefault(&exportAdditionalKeywords, "ExportAdditionalKeywords", "Additional Keywords", "", "", "");

    QString ijkLabel = "Cell Count I, J, K";
    CAF_PDM_InitField(&cellCountI, "CellCountI", 1, ijkLabel, "", "", "");
    CAF_PDM_InitField(&cellCountJ, "CellCountJ", 1, "", "", "", "");
    CAF_PDM_InitField(&cellCountK, "CellCountK", 1, "", "", "", "");

    exportGridFilename    = defaultGridFileName();
    exportResultsFilename = defaultResultsFileName();

    for (QString keyword : mainKeywords())
    {
        exportMainKeywords.v().push_back(keyword);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportEclipseInputGridUi::~RicExportEclipseInputGridUi()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RicExportEclipseInputGridUi::allSelectedKeywords() const
{
    std::vector<QString> additionalResults = exportAdditionalKeywords();
    std::vector<QString> allRes = exportMainKeywords();
    allRes.insert(allRes.end(), additionalResults.begin(), additionalResults.end());
    return allRes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportEclipseInputGridUi::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if (field == &exportResultsFilename || field == &exportGridFilename)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_selectSaveFileName = true;
        }
    }
    else if (field == &exportMainKeywords || field == &exportAdditionalKeywords)
    {
        caf::PdmUiListEditorAttribute* myAttr = dynamic_cast<caf::PdmUiListEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_heightHint = 200;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportEclipseInputGridUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup("Grid Export");
    gridGroup->add(&exportGrid);
    gridGroup->add(&exportGridFilename);
    exportGridFilename.uiCapability()->setUiReadOnly(!exportGrid());        

    caf::PdmUiGroup* resultsGroup = uiOrdering.addNewGroup("Results Export");
    resultsGroup->add(&exportResults);
    if (exportResults() != EXPORT_NO_RESULTS)
    {
        if (exportResults() == EXPORT_TO_SINGLE_SEPARATE_FILE)
            resultsGroup->add(&exportResultsFilename);

        resultsGroup->add(&exportMainKeywords);
        resultsGroup->add(&exportAdditionalKeywords);
    }

    caf::PdmUiGroup* gridRefinement = uiOrdering.addNewGroup("Grid Refinement");
    gridRefinement->add(&cellCountI, { true, 2, 1 });
    gridRefinement->add(&cellCountJ, { false });
    gridRefinement->add(&cellCountK, { false });

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportEclipseInputGridUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &exportGridFilename)
    {
        if (exportResultsFilename() == defaultResultsFileName())
        {
            QFileInfo info(exportGridFilename());
            QDir gridDirPath = info.absoluteDir();
            exportResultsFilename = gridDirPath.absoluteFilePath("results.grdecl");
        }
    }
    else if (changedField == &exportResultsFilename)
    {
        if (exportGridFilename() == defaultGridFileName())
        {
            QFileInfo info(exportResultsFilename());
            QDir      resultsDirPath = info.absoluteDir();
            exportGridFilename = resultsDirPath.absoluteFilePath("grid.grdecl");
        }
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RicExportEclipseInputGridUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    if (fieldNeedingOptions == &exportMainKeywords)
    {
        RigCaseCellResultsData* resultData = m_caseData->results(RiaDefines::MATRIX_MODEL);
        QList<caf::PdmOptionItemInfo> allOptions = RimEclipseResultDefinition::calcOptionsForVariableUiFieldStandard(RiaDefines::STATIC_NATIVE, resultData);        

        std::set<QString> mainKeywords = this->mainKeywords();
        for (caf::PdmOptionItemInfo option : allOptions)
        { 
            if (mainKeywords.count(option.optionUiText()))
            {
                options.push_back(option);
            }
        }
    }
    else if (fieldNeedingOptions == &exportAdditionalKeywords)
    {
        RigCaseCellResultsData*       resultData = m_caseData->results(RiaDefines::MATRIX_MODEL);
        QList<caf::PdmOptionItemInfo> allOptions =
            RimEclipseResultDefinition::calcOptionsForVariableUiFieldStandard(RiaDefines::STATIC_NATIVE, resultData);

        std::set<QString> mainKeywords = this->mainKeywords();
        for (caf::PdmOptionItemInfo option : allOptions)
        {
            if (!mainKeywords.count(option.optionUiText()) && option.optionUiText() != "None")
            {
                if (option.optionUiText() == "ACTNUM" && exportGrid())
                {
                    if (exportResults() != EXPORT_TO_GRID_FILE)
                        options.push_back(caf::PdmOptionItemInfo("ACTNUM (included in Grid File)", "ACTNUM"));
                }
                else
                {
                    options.push_back(option);
                }
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RicExportEclipseInputGridUi::mainKeywords()
{
    return { "EQLNUM", "FIPNUM", "NTG", "PERMX", "PERMY", "PERMZ", "PORO", "PVTNUM", "SATNUM", "SWATINIT" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportEclipseInputGridUi::defaultGridFileName() const
{
    QDir    baseDir(RiaApplication::instance()->currentProjectPath());
    return baseDir.absoluteFilePath("GRID.GRDECL");    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportEclipseInputGridUi::defaultResultsFileName() const
{
    QDir    baseDir(RiaApplication::instance()->currentProjectPath());
    return baseDir.absoluteFilePath("RESULTS.GRDECL");
}
