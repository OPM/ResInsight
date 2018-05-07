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

#include "cafPdmField.h"
#include "RimCalcScript.h"
#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT(RimCalcScript, "CalcScript");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalcScript::RimCalcScript()
{
    CAF_PDM_InitObject("CalcScript", ":/OctaveScriptFile16x16.png", "Calc Script", "");

    CAF_PDM_InitField(&absolutePath, "AbsolutePath", QString(), "Location", "", "" ,"");
    CAF_PDM_InitField(&content, "Content", QString(), "Directory", "", "" ,"");
    content.uiCapability()->setUiHidden(true);
    content.xmlCapability()->setIOWritable(false);

    absolutePath.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalcScript::~RimCalcScript()
{
}

