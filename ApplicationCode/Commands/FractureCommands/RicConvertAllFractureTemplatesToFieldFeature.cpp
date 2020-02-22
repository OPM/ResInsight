/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicConvertAllFractureTemplatesToFieldFeature.h"

#include "RiaApplication.h"
#include "RiaEclipseUnitTools.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFracture.h"
#include "RimFractureExportSettings.h"
#include "RimFractureTemplateCollection.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <QFileInfo>
#include <QString>

CAF_CMD_SOURCE_INIT( RicConvertAllFractureTemplatesToFieldFeature, "RicConvertAllFractureTemplatesToFieldFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicConvertAllFractureTemplatesToFieldFeature::onActionTriggered( bool isChecked )
{
    auto objHandle = caf::SelectionManager::instance()->selectedItemOfType<caf::PdmObjectHandle>();
    if ( !objHandle ) return;

    RimFractureTemplateCollection* fracTempColl = nullptr;
    objHandle->firstAncestorOrThisOfType( fracTempColl );

    std::vector<RimEllipseFractureTemplate*> ellipseFracTemplates;
    fracTempColl->descendantsIncludingThisOfType( ellipseFracTemplates );

    for ( auto ellipseFracTemplate : ellipseFracTemplates )
    {
        if ( ellipseFracTemplate->fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_METRIC )
        {
            ellipseFracTemplate->changeUnits();

            ellipseFracTemplate->disconnectAllFracturesAndRedrawViews();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicConvertAllFractureTemplatesToFieldFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Convert All Ellipse Fracture Templates to Field" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicConvertAllFractureTemplatesToFieldFeature::isCommandEnabled()
{
    return true;
}
