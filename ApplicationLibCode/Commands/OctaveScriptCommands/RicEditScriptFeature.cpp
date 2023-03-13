/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicEditScriptFeature.h"

#include "ApplicationCommands/RicOpenInTextEditorFeature.h"
#include "RicScriptFeatureImpl.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RimCalcScript.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicEditScriptFeature, "RicEditScriptFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEditScriptFeature::isCommandEnabled()
{
    std::vector<RimCalcScript*> selection = RicScriptFeatureImpl::selectedScripts();
    return !selection.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditScriptFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimCalcScript*> selection = RicScriptFeatureImpl::selectedScripts();
    CVF_ASSERT( !selection.empty() );

    RimCalcScript* calcScript = selection[0];

    RicOpenInTextEditorFeature::openFileInTextEditor( calcScript->absoluteFileName, this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditScriptFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Edit" );
}
