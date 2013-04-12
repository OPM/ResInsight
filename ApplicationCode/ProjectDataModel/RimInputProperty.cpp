/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaStdInclude.h"

#include "RimInputProperty.h"

#include "cvfAssert.h"

#include "cafPdmUiLineEditor.h"

#include <QString>

namespace caf {
template<>
void RimInputProperty::ResolveStateEnum::setUp()
{
    addItem(RimInputProperty::UNKNOWN,              "UNKNOWN", "Unknown");
    addItem(RimInputProperty::FILE_MISSING,         "FILE_MISSING", "No data loaded, can't find the file");
    addItem(RimInputProperty::KEYWORD_NOT_IN_FILE,  "KEYWORD_NOT_IN_FILE", "No data loaded, can't find the keyword");
    addItem(RimInputProperty::RESOLVED_NOT_SAVED,   "RESOLVED_NOT_SAVED", "Ok, but not saved");
    addItem(RimInputProperty::RESOLVED,             "RESOLVED", "Loaded, Ok");
    setDefault(RimInputProperty::UNKNOWN);
}
}


CAF_PDM_SOURCE_INIT(RimInputProperty, "RimInputProperty");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimInputProperty::RimInputProperty()
{
    CAF_PDM_InitObject("Input Property", ":/EclipseInput48x48.png", "", "");
    
    CAF_PDM_InitField(&resultName, "ResultName",  QString(), "Result name", "", "" ,"");
    CAF_PDM_InitField(&eclipseKeyword, "EclipseKeyword", QString(), "Eclipse Keyword", "", "" ,"");
    CAF_PDM_InitField(&fileName, "FileName", QString(), "Filename", "", "" ,"");
    CAF_PDM_InitField(&resolvedState, "ResolvedState", (ResolveStateEnum)UNKNOWN, "Data state", "", "", "");

    resolvedState.setUiReadOnly(true);
    resolvedState.setIOReadable(false); 
    resolvedState.setIOWritable(false); 
    resolvedState.setUiEditorTypeName(caf::PdmUiLineEditor::uiEditorTypeName());

    fileName.setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimInputProperty::~RimInputProperty()
{

}

