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

#include "RicCopyReferencesToClipboardFeature.h"

#include "RimMimeData.h"

#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>


namespace caf
{

CAF_CMD_SOURCE_INIT(RicCopyReferencesToClipboardFeature, "RicCopyReferencesToClipboardFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCopyReferencesToClipboardFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCopyReferencesToClipboardFeature::onActionTriggered(bool isChecked)
{
    std::vector<QString> referenceList;
    SelectionManager::instance()->selectionAsReferences(referenceList);

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
void RicCopyReferencesToClipboardFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Copy");
}

} // end namespace caf
