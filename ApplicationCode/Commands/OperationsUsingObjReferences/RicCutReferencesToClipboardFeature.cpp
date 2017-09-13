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

#include "RicCutReferencesToClipboardFeature.h"

#include "RimMimeData.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurveFilter.h"

#include "cafPdmObject.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>

CAF_CMD_SOURCE_INIT(RicCutReferencesToClipboardFeature, "RicCutReferencesToClipboardFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCutReferencesToClipboardFeature::isCommandEnabled()
{
    return isAnyCuttableObjectSelected();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCutReferencesToClipboardFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    if (!isAnyCuttableObjectSelected()) return;

    std::vector<QString> referenceList;

    std::vector<caf::PdmObject*> selectedFormationNamesCollObjs;
    caf::SelectionManager::instance()->objectsByType(&selectedFormationNamesCollObjs);

    for (caf::PdmObject* pdmObject : selectedFormationNamesCollObjs)
    {
        if (RicCutReferencesToClipboardFeature::isCuttingOfObjectSupported(pdmObject))
        {
            QString itemRef = caf::PdmReferenceHelper::referenceFromRootToObject(caf::SelectionManager::instance()->pdmRootObject(), pdmObject);
            
            referenceList.push_back(itemRef);
        }
    }

    MimeDataWithReferences* myObject = new MimeDataWithReferences;
    myObject->setReferences(referenceList);

    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard)
    {
        clipboard->setMimeData(myObject);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCutReferencesToClipboardFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Cut");
    actionToSetup->setIcon(QIcon(":/Clipboard.png"));
    actionToSetup->setShortcut(QKeySequence::Cut);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCutReferencesToClipboardFeature::isAnyCuttableObjectSelected()
{
    std::vector<caf::PdmObject*> selectedFormationNamesCollObjs;
    caf::SelectionManager::instance()->objectsByType(&selectedFormationNamesCollObjs);

    for (caf::PdmObject* pdmObject : selectedFormationNamesCollObjs)
    {
        if (RicCutReferencesToClipboardFeature::isCuttingOfObjectSupported(pdmObject))
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCutReferencesToClipboardFeature::isCuttingOfObjectSupported(caf::PdmObject* pdmObject)
{
    if (dynamic_cast<RimSummaryCase*>(pdmObject))
    {
        return true;
    }

    return false;
}
