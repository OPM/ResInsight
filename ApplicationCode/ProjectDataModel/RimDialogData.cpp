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

#include "RimDialogData.h"

#include "ExportCommands/RicExportCarfinUi.h"
#include "CompletionExportCommands/RicExportCompletionDataSettingsUi.h"

CAF_PDM_SOURCE_INIT(RimDialogData, "RimDialogData");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDialogData::RimDialogData()
{
    CAF_PDM_InitObject("Dialog Data", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_exportCarfin, "ExportCarfin", "Export Carfin", "", "", "");
    m_exportCarfin = new RicExportCarfinUi;

    CAF_PDM_InitFieldNoDefault(&m_exportCompletionData, "ExportCompletionData", "Export Completion Data", "", "", "");
    m_exportCompletionData = new RicExportCompletionDataSettingsUi();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportCarfinUi* RimDialogData::exportCarfin() const
{
    return m_exportCarfin;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimDialogData::exportCarfinDataAsString() const
{
    return m_exportCarfin->writeObjectToXmlString();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimDialogData::setExportCarfinDataFromString(const QString& data)
{
    m_exportCarfin->readObjectFromXmlString(data, caf::PdmDefaultObjectFactory::instance());
    m_exportCarfin->resolveReferencesRecursively();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportCompletionDataSettingsUi* RimDialogData::exportCompletionData() const
{
    return m_exportCompletionData;
}

