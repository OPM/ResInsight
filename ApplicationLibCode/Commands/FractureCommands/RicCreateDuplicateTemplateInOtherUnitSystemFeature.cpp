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

#include "RicCreateDuplicateTemplateInOtherUnitSystemFeature.h"

#include "RiaDefines.h"

#include "RicNewEllipseFractureTemplateFeature.h"

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
#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>
#include <QFileInfo>
#include <QString>

CAF_CMD_SOURCE_INIT( RicCreateDuplicateTemplateInOtherUnitSystemFeature, "RicConvertFractureTemplateUnitFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateDuplicateTemplateInOtherUnitSystemFeature::onActionTriggered( bool isChecked )
{
    RimFractureTemplate* fractureTemplate = caf::firstAncestorOfTypeFromSelectedObject<RimFractureTemplate>();
    if ( !fractureTemplate ) return;

    auto copyOfTemplate = fractureTemplate->copyObject<RimFractureTemplate>();

    RimFractureTemplateCollection* fractureTemplateCollection = caf::firstAncestorOfTypeFromSelectedObject<RimFractureTemplateCollection>();
    fractureTemplateCollection->addFractureTemplate( copyOfTemplate );

    auto currentUnit = copyOfTemplate->fractureTemplateUnit();
    if ( currentUnit == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        copyOfTemplate->convertToUnitSystem( RiaDefines::EclipseUnitSystem::UNITS_FIELD );
    }
    else
    {
        copyOfTemplate->convertToUnitSystem( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    }

    QString name = copyOfTemplate->name();
    name += " (copy)";
    copyOfTemplate->setName( name );

    copyOfTemplate->loadDataAndUpdate();
    copyOfTemplate->updateConnectedEditors();

    RicNewEllipseFractureTemplateFeature::selectFractureTemplateAndUpdate( copyOfTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateDuplicateTemplateInOtherUnitSystemFeature::setupActionLook( QAction* actionToSetup )
{
    RimFractureTemplate* fractureTemplate = caf::firstAncestorOfTypeFromSelectedObject<RimFractureTemplate>();
    if ( !fractureTemplate ) return;

    QString destinationUnit;
    if ( fractureTemplate->fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        destinationUnit += "Field";
    }
    else if ( fractureTemplate->fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        destinationUnit += "Metric";
    }

    QString text = QString( "Create %1 Units Duplicate" ).arg( destinationUnit );

    actionToSetup->setIcon( QIcon( ":/FractureTemplate16x16.png" ) );

    actionToSetup->setText( text );
}
