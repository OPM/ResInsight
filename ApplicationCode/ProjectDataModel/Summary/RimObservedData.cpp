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
#include "RimTools.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT(RimObservedData, "ObservedData");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedData::RimObservedData()
{
    m_isObservedData = true;

    CAF_PDM_InitFieldNoDefault(&m_summaryCategory, "SummaryType", "Summary Type", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_identifierName, "IdentifierName", "Identifier Name", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimObservedData::summaryHeaderFilename() const
{
    return m_summaryHeaderFilename();
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
void RimObservedData::createSummaryReaderInterface()
{
    throw std::logic_error("The method or operation is not implemented.");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimObservedData::summaryReader()
{
    throw std::logic_error("The method or operation is not implemented.");

    return nullptr;
}
