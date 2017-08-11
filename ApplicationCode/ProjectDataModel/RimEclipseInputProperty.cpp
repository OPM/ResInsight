/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseInputProperty.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

#include "RimEclipseInputCase.h"
#include "RimEclipseResultDefinition.h"

#include "cafPdmUiLineEditor.h"
#include "cvfAssert.h"

#include <QString>

namespace caf {
template<>
void RimEclipseInputProperty::ResolveStateEnum::setUp()
{
    addItem(RimEclipseInputProperty::UNKNOWN,              "UNKNOWN", "Unknown");
    addItem(RimEclipseInputProperty::FILE_MISSING,         "FILE_MISSING", "No data loaded, can't find the file");
    addItem(RimEclipseInputProperty::KEYWORD_NOT_IN_FILE,  "KEYWORD_NOT_IN_FILE", "No data loaded, can't find the keyword");
    addItem(RimEclipseInputProperty::RESOLVED_NOT_SAVED,   "RESOLVED_NOT_SAVED", "Ok, but not saved");
    addItem(RimEclipseInputProperty::RESOLVED,             "RESOLVED", "Loaded, Ok");
    setDefault(RimEclipseInputProperty::UNKNOWN);
}
}


CAF_PDM_SOURCE_INIT(RimEclipseInputProperty, "RimInputProperty");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputProperty::RimEclipseInputProperty()
{
    CAF_PDM_InitObject("Input Property", ":/EclipseInput48x48.png", "", "");
    
    CAF_PDM_InitField(&resultName, "ResultName",  QString(), "Result name", "", "" ,"");
    CAF_PDM_InitField(&eclipseKeyword, "EclipseKeyword", QString(), "Eclipse Keyword", "", "" ,"");
    CAF_PDM_InitField(&fileName, "FileName", QString(), "Filename", "", "" ,"");
    CAF_PDM_InitField(&resolvedState, "ResolvedState", (ResolveStateEnum)UNKNOWN, "Data state", "", "", "");

    resolvedState.uiCapability()->setUiReadOnly(true);
    resolvedState.xmlCapability()->setIOReadable(false); 
    resolvedState.xmlCapability()->setIOWritable(false); 
    resolvedState.uiCapability()->setUiEditorTypeName(caf::PdmUiLineEditor::uiEditorTypeName());

    fileName.uiCapability()->setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputProperty::~RimEclipseInputProperty()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseInputProperty::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &resultName)
    {
        RimEclipseInputCase* rimCase = nullptr;
        this->firstAncestorOrThisOfType(rimCase);
        if (rimCase)
        {
            bool anyNameUpdated = false;

            QString oldName = oldValue.toString();
            QString newName = newValue.toString();

            RigCaseCellResultsData* matrixResults = rimCase->eclipseCaseData()->results(RiaPorosityModel::MATRIX_MODEL);
            if (matrixResults)
            {
                if (matrixResults->updateResultName(RiaDefines::INPUT_PROPERTY, oldName, newName))
                {
                    anyNameUpdated = true;
                }
            }

            RigCaseCellResultsData* fracResults = rimCase->eclipseCaseData()->results(RiaPorosityModel::FRACTURE_MODEL);
            if (fracResults)
            {
                if (fracResults->updateResultName(RiaDefines::INPUT_PROPERTY, oldName, newName))
                {
                    anyNameUpdated = true;
                }
            }

            if (anyNameUpdated)
            {
                std::vector<RimEclipseResultDefinition*> resDefs;
                rimCase->descendantsIncludingThisOfType(resDefs);

                for (auto it : resDefs)             
                {
                    if (it->resultVariable() == oldName)
                    {
                        it->setResultVariable(newName);
                    }

                    it->loadDataAndUpdate();
                    it->updateAnyFieldHasChanged();
                }
            }
        }
    }
}

