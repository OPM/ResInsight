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

    CAF_PDM_InitFieldNoDefault(&m_importedSummaryData, "ImportedSummaryData", "Imported Summary Data", "", "", "");
    m_importedSummaryData.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    m_importedSummaryData.uiCapability()->setUiReadOnly(true);
    m_importedSummaryData.xmlCapability()->disableIO();

    CAF_PDM_InitField(&m_useCustomIdentifier, "UseCustomIdentifier", false, "Use Custom Identifier", "", "", "");
    m_useCustomIdentifier.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&m_summaryCategory, "SummaryType", caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>(RifEclipseSummaryAddress::SUMMARY_WELL), "Summary Type", "", "", "");
    m_summaryCategory.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_identifierName, "IdentifierName", "Identifier Name", "", "", "");
    m_identifierName.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimObservedData::caseName() const
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimObservedData::customWellName() const
{
    if (m_useCustomIdentifier() && m_summaryCategory() == RifEclipseSummaryAddress::SUMMARY_WELL)
    {
        return m_identifierName();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimObservedData::customWellGroupName() const
{
    if (m_useCustomIdentifier() && m_summaryCategory() == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP)
    {
        return m_identifierName();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimObservedData::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    if (fieldNeedingOptions == &m_summaryCategory)
    {
        QList<caf::PdmOptionItemInfo> options;

        using AddressAppEnum = caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>;
        options.push_back(caf::PdmOptionItemInfo(AddressAppEnum::uiText(RifEclipseSummaryAddress::SUMMARY_WELL), RifEclipseSummaryAddress::SUMMARY_WELL));
        options.push_back(caf::PdmOptionItemInfo(AddressAppEnum::uiText(RifEclipseSummaryAddress::SUMMARY_WELL_GROUP), RifEclipseSummaryAddress::SUMMARY_WELL_GROUP));

        return options;
    }
       
    return RimSummaryCase::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimObservedData::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_useCustomIdentifier ||
        changedField == &m_summaryCategory ||
        changedField == &m_identifierName)
    {
        createSummaryReaderInterface();
            
        updateMetaData();

        return;
    }

    RimSummaryCase::fieldChangedByUi(changedField, oldValue, newValue);
}
