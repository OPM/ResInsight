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

#include "RimCalcScript.h"

#include "RiaApplication.h"
#include "RiaFieldHandleTools.h"
#include "RiaLogging.h"

#include "cafPdmField.h"
#include "cafPdmUiFilePathEditor.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT(RimCalcScript, "CalcScript");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCalcScript::RimCalcScript()
{
    CAF_PDM_InitObject("CalcScript", ":/OctaveScriptFile16x16.png", "Calc Script", "");

    CAF_PDM_InitField(&absoluteFileName, "AbsolutePath", QString(), "Location", "", "", "");
    CAF_PDM_InitField(&content, "Content", QString(), "Directory", "", "", "");
    RiaFieldhandleTools::disableWriteAndSetFieldHidden(&content);

    absoluteFileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCalcScript::~RimCalcScript() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimCalcScript::createCommandLineArguments(const QString& absoluteFileNameScript)
{
    QStringList arguments;

    {
        auto app = RiaApplication::instance();

        arguments = app->octaveArguments();
        arguments.append("--path");
    }

    {
        QFileInfo fi(absoluteFileNameScript);
        QString   octaveFunctionSearchPath = fi.absolutePath();
        QString   absFilePath              = fi.absoluteFilePath();

        arguments << octaveFunctionSearchPath;
        arguments << absFilePath;
    }

    bool debugPrintArgumentText = false;
    if (debugPrintArgumentText)
    {
        QString argumentString = arguments.join(" ");

        RiaLogging::info("Octave arguments : " + argumentString);
    }

    return arguments;
}
