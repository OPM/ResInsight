/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RicNewThermalFractureTemplateFeature.h"

#include "RicMeshFractureTemplateHelper.h"

#include "RimThermalFractureTemplate.h"

#include <vector>

CAF_CMD_SOURCE_INIT( RicNewThermalFractureTemplateFeature, "RicNewThermalFractureTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewThermalFractureTemplateFeature::createNewTemplateForFractureAndUpdate( RimFracture* fracture )
{
    RicMeshFractureTemplateHelper<RimThermalFractureTemplate>::createNewTemplateForFractureAndUpdate( fracture,
                                                                                                      title(),
                                                                                                      lastUsedDialogFallback(),
                                                                                                      fileFilter(),
                                                                                                      defaultTemplateName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewThermalFractureTemplateFeature::selectFractureTemplateAndUpdate( RimFractureTemplate* fractureTemplate )
{
    RicMeshFractureTemplateHelper<RimThermalFractureTemplate>::selectFractureTemplateAndUpdate( fractureTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimThermalFractureTemplate*> RicNewThermalFractureTemplateFeature::createNewTemplates()
{
    return RicMeshFractureTemplateHelper<RimThermalFractureTemplate>::createNewTemplates( title(),
                                                                                          lastUsedDialogFallback(),
                                                                                          fileFilter(),
                                                                                          defaultTemplateName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimThermalFractureTemplate*>
    RicNewThermalFractureTemplateFeature::createNewTemplatesFromFiles( const std::vector<QString>& fileNames,
                                                                       bool reuseExistingTemplatesWithMatchingNames )
{
    return RicMeshFractureTemplateHelper<RimThermalFractureTemplate>::createNewTemplatesFromFiles( fileNames,
                                                                                                   defaultTemplateName(),
                                                                                                   reuseExistingTemplatesWithMatchingNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewThermalFractureTemplateFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimThermalFractureTemplate*> newFractures = createNewTemplates();
    if ( !newFractures.empty() )
    {
        selectFractureTemplateAndUpdate( newFractures.back() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewThermalFractureTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureTemplate16x16.png" ) );
    actionToSetup->setText( "New Thermal Fracture Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewThermalFractureTemplateFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewThermalFractureTemplateFeature::fileFilter()
{
    return "Reveal Open-Server Files (*.csv);;All files (*.*)";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewThermalFractureTemplateFeature::title()
{
    return "Open Thermal Fracture File";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewThermalFractureTemplateFeature::lastUsedDialogFallback()
{
    return "REVEAL_CSV_DIR";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewThermalFractureTemplateFeature::defaultTemplateName()
{
    return "Thermal Fracture Template";
}
