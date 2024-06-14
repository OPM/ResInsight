/////////////////////////////////////////////////////////////////////////////////
//
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

#include "RicPasteStimPlanFractureFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RicNewEllipseFractureTemplateFeature.h"

#include "RimFractureTemplateCollection.h"
#include "RimStimPlanFractureTemplate.h"

#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QString>

CAF_CMD_SOURCE_INIT( RicPasteStimPlanFractureFeature, "RicPasteStimPlanFractureFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteStimPlanFractureFeature::isCommandEnabled() const
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimStimPlanFractureTemplate>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    if ( typedObjects.empty() )
    {
        return false;
    }

    if ( fractureTemplateCollection() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteStimPlanFractureFeature::onActionTriggered( bool isChecked )
{
    auto fractureTemplateColl = fractureTemplateCollection();
    if ( !fractureTemplateColl ) return;

    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimStimPlanFractureTemplate>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    for ( const auto& source : typedObjects )
    {
        auto copyOfStimPlanTemplate = source->copyObject<RimStimPlanFractureTemplate>();
        fractureTemplateColl->addFractureTemplate( copyOfStimPlanTemplate );

        RicNewEllipseFractureTemplateFeature::selectFractureTemplateAndUpdate( copyOfStimPlanTemplate );
    }

    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteStimPlanFractureFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste (StimPlan Fracture)" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection* RicPasteStimPlanFractureFeature::fractureTemplateCollection()
{
    return caf::firstAncestorOfTypeFromSelectedObject<RimFractureTemplateCollection>();
}
