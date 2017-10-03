/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimObservedData.h"

#include "RifSummaryReaderInterface.h"
#include "RimTools.h"

#include "cafPdmUiTextEditor.h"

#include <QFileInfo>

CAF_PDM_ABSTRACT_SOURCE_INIT(RimObservedData, "ObservedData");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedData::RimObservedData()
{
    m_isObservedData = true;

    CAF_PDM_InitFieldNoDefault(&m_summaryCategory, "SummaryType", "Summary Type", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_identifierName, "IdentifierName", "Identifier Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_importedSummaryData, "ImportedSummaryData", "Imported Summary Data", "", "", "");
    m_importedSummaryData.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    m_importedSummaryData.uiCapability()->setUiReadOnly(true);
    m_importedSummaryData.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimObservedData::caseName()
{
    QFileInfo caseFileName(this->summaryHeaderFilename());

    return caseFileName.completeBaseName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimObservedData::updateFilePathsFromProjectPath(const QString & newProjectPath, const QString & oldProjectPath)
{
    m_summaryHeaderFilename = RimTools::relocateFile(m_summaryHeaderFilename(), newProjectPath, oldProjectPath, nullptr, nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimObservedData::identifierName() const
{
    return m_identifierName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::SummaryVarCategory RimObservedData::summaryCategory() const
{
    return m_summaryCategory();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimObservedData::updateMetaData()
{
    std::string metaDataString;

    RifSummaryReaderInterface* readerInterface = summaryReader();
    if (readerInterface)
    {
        for (const auto& a : readerInterface->allResultAddresses())
        {
            metaDataString += a.uiText();
            metaDataString += "\n";
        }
    }
    
    m_importedSummaryData = QString::fromStdString(metaDataString);
}
