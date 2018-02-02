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

#include "RicConvertFractureTemplateUnitFeature.h"


#include "RiaApplication.h"
#include "RiaEclipseUnitTools.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFracture.h"
#include "RimFractureExportSettings.h"
#include "Rim3dView.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <QString>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT(RicConvertFractureTemplateUnitFeature, "RicConvertFractureTemplateUnitFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicConvertFractureTemplateUnitFeature::onActionTriggered(bool isChecked)
{
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    if (!pdmUiItem) return;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (!objHandle) return;

    RimEllipseFractureTemplate* ellipseFractureTemplate = nullptr;
    objHandle->firstAncestorOrThisOfType(ellipseFractureTemplate);

   ellipseFractureTemplate->changeUnits();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicConvertFractureTemplateUnitFeature::setupActionLook(QAction* actionToSetup)
{
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    if (!pdmUiItem) return;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (!objHandle) return;

    RimEllipseFractureTemplate* ellipseFractureTemplate = nullptr;
    objHandle->firstAncestorOrThisOfType(ellipseFractureTemplate);
    if (!ellipseFractureTemplate) return;

    QString text = "Convert Values to ";
    if (ellipseFractureTemplate->fractureTemplateUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        text += "Field";
    }
    else if (ellipseFractureTemplate->fractureTemplateUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        text += "Metric";
    }
    
    actionToSetup->setIcon(QIcon(":/FractureTemplate16x16.png"));
    //TODO: Add unit to text

    actionToSetup->setText(text);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicConvertFractureTemplateUnitFeature::isCommandEnabled()
{
    return true;
}
