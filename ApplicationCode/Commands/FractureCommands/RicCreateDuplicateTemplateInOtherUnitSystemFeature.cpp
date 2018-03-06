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


#include "RiaApplication.h"
#include "RiaEclipseUnitTools.h"

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

#include "cvfAssert.h"

#include <QAction>
#include <QString>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT(RicCreateDuplicateTemplateInOtherUnitSystemFeature, "RicConvertFractureTemplateUnitFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCreateDuplicateTemplateInOtherUnitSystemFeature::onActionTriggered(bool isChecked)
{
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    if (!pdmUiItem) return;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (!objHandle) return;

    RimFractureTemplate* fractureTemplate = nullptr;
    objHandle->firstAncestorOrThisOfType(fractureTemplate);

    if (!fractureTemplate) return;

    RimFractureTemplateCollection* fractureTemplateCollection = nullptr;
    fractureTemplate->firstAncestorOrThisOfType(fractureTemplateCollection);

    auto copyOfTemplate = dynamic_cast<RimFractureTemplate*>(
        fractureTemplate->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));

    fractureTemplateCollection->addFractureTemplate(copyOfTemplate);

    RicNewEllipseFractureTemplateFeature::selectFractureTemplateAndUpdate(fractureTemplateCollection, copyOfTemplate);

    auto currentUnit = copyOfTemplate->fractureTemplateUnit();
    if (currentUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
       copyOfTemplate->convertToUnitSystem(RiaEclipseUnitTools::UNITS_FIELD);
    }
    else
    {
       copyOfTemplate->convertToUnitSystem(RiaEclipseUnitTools::UNITS_METRIC);
    }

    QString name = copyOfTemplate->name();
    name += " (copy)";
    copyOfTemplate->setName(name);

    copyOfTemplate->loadDataAndUpdate();
    copyOfTemplate->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCreateDuplicateTemplateInOtherUnitSystemFeature::setupActionLook(QAction* actionToSetup)
{
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    if (!pdmUiItem) return;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (!objHandle) return;

    RimFractureTemplate* fractureTemplate = nullptr;
    objHandle->firstAncestorOrThisOfType(fractureTemplate);
    if (!fractureTemplate) return;

    QString destinationUnit;
    if (fractureTemplate->fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_METRIC)
    {
        destinationUnit += "Field";
    }
    else if (fractureTemplate->fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_FIELD)
    {
        destinationUnit += "Metric";
    }

    QString text = QString("Create %1 Units Duplicate").arg(destinationUnit);
    
    actionToSetup->setIcon(QIcon(":/FractureTemplate16x16.png"));

    actionToSetup->setText(text);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCreateDuplicateTemplateInOtherUnitSystemFeature::isCommandEnabled()
{
    return true;
}
