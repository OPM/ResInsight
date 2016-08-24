/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RimFormationNames.h"

#include "RimTools.h"

#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT(RimFormationNames, "FormationNames");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFormationNames::RimFormationNames()
{
    CAF_PDM_InitObject("Formation Names", ":/Formations16x16.png", "", "");

    CAF_PDM_InitField(&m_formationNamesFileName, "FormationNamesFileName", QString(""), "File Name", "", "", "");

    m_formationNamesFileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFormationNames::~RimFormationNames()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFormationNames::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&m_formationNamesFileName == changedField)
    {
        updateUiTreeName();

        readFormationNamesFile(nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFormationNames::initAfterRead()
{
    updateUiTreeName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFormationNames::updateUiTreeName()
{
    this->uiCapability()->setUiName(m_formationNamesFileName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFormationNames::setFileName(const QString& fileName)
{
    m_formationNamesFileName = fileName;

    updateUiTreeName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString& RimFormationNames::fileName()
{
    return m_formationNamesFileName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFormationNames::readFormationNamesFile(QString * errorMessage)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFormationNames::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    m_formationNamesFileName = RimTools::relocateFile(m_formationNamesFileName(), newProjectPath, oldProjectPath, NULL, NULL);
}

