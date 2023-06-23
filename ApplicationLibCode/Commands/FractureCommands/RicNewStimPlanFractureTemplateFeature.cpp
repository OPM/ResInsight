/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-2018 Statoil ASA
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicNewStimPlanFractureTemplateFeature.h"

#include "RicMeshFractureTemplateHelper.h"

#include "RimStimPlanFractureTemplate.h"

#include <vector>

CAF_CMD_SOURCE_INIT( RicNewStimPlanFractureTemplateFeature, "RicNewStimPlanFractureTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::createNewTemplateForFractureAndUpdate( RimFracture* fracture )
{
    RicMeshFractureTemplateHelper<RimStimPlanFractureTemplate>::createNewTemplateForFractureAndUpdate( fracture,
                                                                                                       title(),
                                                                                                       lastUsedDialogFallback(),
                                                                                                       fileFilter(),
                                                                                                       defaultTemplateName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::selectFractureTemplateAndUpdate( RimFractureTemplate* fractureTemplate )
{
    RicMeshFractureTemplateHelper<RimStimPlanFractureTemplate>::selectFractureTemplateAndUpdate( fractureTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimStimPlanFractureTemplate*> RicNewStimPlanFractureTemplateFeature::createNewTemplates()
{
    return RicMeshFractureTemplateHelper<RimStimPlanFractureTemplate>::createNewTemplates( title(),
                                                                                           lastUsedDialogFallback(),
                                                                                           fileFilter(),
                                                                                           defaultTemplateName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimStimPlanFractureTemplate*>
    RicNewStimPlanFractureTemplateFeature::createNewTemplatesFromFiles( const std::vector<QString>& fileNames,
                                                                        bool reuseExistingTemplatesWithMatchingNames )
{
    return RicMeshFractureTemplateHelper<RimStimPlanFractureTemplate>::createNewTemplatesFromFiles( fileNames,
                                                                                                    defaultTemplateName(),
                                                                                                    reuseExistingTemplatesWithMatchingNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimStimPlanFractureTemplate*> newFractures = createNewTemplates();
    if ( !newFractures.empty() )
    {
        selectFractureTemplateAndUpdate( newFractures.back() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureTemplate16x16.png" ) );
    actionToSetup->setText( "New StimPlan Fracture Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewStimPlanFractureTemplateFeature::fileFilter()
{
    return "StimPlan XML File (*.xml);;All files(*.*)";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewStimPlanFractureTemplateFeature::title()
{
    return "Open StimPlan XML File";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewStimPlanFractureTemplateFeature::lastUsedDialogFallback()
{
    return "STIMPLAN_XML_DIR";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewStimPlanFractureTemplateFeature::defaultTemplateName()
{
    return "StimPlan Fracture Template";
}
