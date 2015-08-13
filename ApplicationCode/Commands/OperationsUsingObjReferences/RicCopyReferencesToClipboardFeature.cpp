//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

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
    actionToSetup->setText("RicCopyReferencesToClipboardFeature");
}

} // end namespace caf
