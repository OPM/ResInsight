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

CAF_PDM_SOURCE_INIT( RimCalcScript, "CalcScript" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCalcScript::RimCalcScript()
{
    CAF_PDM_InitObject( "CalcScript", ":/OctaveScriptFile16x16.png", "Calc Script", "" );

    CAF_PDM_InitField( &absoluteFileName, "AbsolutePath", QString(), "Location" );

    absoluteFileName.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCalcScript::~RimCalcScript()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCalcScript::ScriptType RimCalcScript::scriptType( const QString& absoluteFileNameScript )
{
    QFileInfo fileInfo( absoluteFileNameScript );
    if ( fileInfo.suffix() == "py" )
    {
        return PYTHON;
    }
    else if ( fileInfo.suffix() == "m" )
    {
        return OCTAVE;
    }
    return UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCalcScript::ScriptType RimCalcScript::scriptType() const
{
    return scriptType( absoluteFileName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimCalcScript::createCommandLineArguments( const QString& absoluteFileNameScript )
{
    QStringList arguments;

    if ( scriptType( absoluteFileNameScript ) == PYTHON )
    {
        arguments.append( absoluteFileNameScript );
    }
    else if ( scriptType( absoluteFileNameScript ) == OCTAVE )
    {
        {
            auto app = RiaApplication::instance();

            arguments = app->octaveArguments();
            arguments.append( "--path" );
        }

        {
            QFileInfo fi( absoluteFileNameScript );
            QString   octaveFunctionSearchPath = fi.absolutePath();
            QString   absFilePath              = fi.absoluteFilePath();

            arguments << octaveFunctionSearchPath;
            arguments << absFilePath;
        }
    }

    bool debugPrintArgumentText = false;
    if ( debugPrintArgumentText )
    {
        QString argumentString = arguments.join( " " );

        RiaLogging::info( "Scriptarguments : " + argumentString );
    }

    return arguments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCalcScript::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( scriptType() == PYTHON )
    {
        uiCapability()->setUiIconFromResourceString( ":/PythonScriptFile16x16.png" );
    }
}
