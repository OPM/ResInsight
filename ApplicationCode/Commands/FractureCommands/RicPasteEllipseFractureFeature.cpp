/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RicPasteEllipseFractureFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RicNewEllipseFractureTemplateFeature.h"

#include "RimEllipseFractureTemplate.h"
#include "RimFractureTemplateCollection.h"

#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QString>

CAF_CMD_SOURCE_INIT(RicPasteEllipseFractureFeature, "RicPasteEllipseFractureFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteEllipseFractureFeature::isCommandEnabled()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimEllipseFractureTemplate>> typedObjects;
    objectGroup.objectsByType(&typedObjects);

    if (typedObjects.size() == 0)
    {
        return false;
    }

    if (fractureTemplateCollection())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteEllipseFractureFeature::onActionTriggered(bool isChecked)
{
    auto fractureTemplateColl = fractureTemplateCollection();
    if (!fractureTemplateColl) return;

    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimEllipseFractureTemplate>> typedObjects;
    objectGroup.objectsByType(&typedObjects);

    for (const auto& source : typedObjects)
    {
        auto templ = dynamic_cast<RimEllipseFractureTemplate*>(
            source->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));

        fractureTemplateColl->addFractureTemplate(templ);

        RicNewEllipseFractureTemplateFeature::selectFractureTemplateAndUpdate(fractureTemplateColl, templ);
    }

    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteEllipseFractureFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste (Ellipse Fracture)");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection* RicPasteEllipseFractureFeature::fractureTemplateCollection()
{
    RimFractureTemplateCollection* fractureTemplateColl = nullptr;

    auto destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());
    if (destinationObject)
    {
        destinationObject->firstAncestorOrThisOfType(fractureTemplateColl);
    }

    return fractureTemplateColl;
}
