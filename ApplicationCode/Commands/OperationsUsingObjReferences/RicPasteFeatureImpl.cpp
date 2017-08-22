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

#include "RicPasteFeatureImpl.h"

#include "RiaApplication.h"

#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMimeData.h"
#include "RimProject.h"

#include "cafPdmObjectGroup.h"
#include "cafPdmObjectHandle.h"

#include <QClipboard>
#include <QString>
#include <QAction>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteFeatureImpl::populateObjectGroupFromReferences(const std::vector<QString>& referenceList, caf::PdmObjectGroup* objectGroup)
{
    caf::PdmObjectHandle* referenceRoot = RiaApplication::instance()->project();

    for (size_t i = 0; i < referenceList.size(); i++)
    {
        QString reference = referenceList[i];

        caf::PdmObjectHandle* pdmObj = caf::PdmReferenceHelper::objectFromReference(referenceRoot, reference);
        if (pdmObj)
        {
            objectGroup->objects.push_back(pdmObj);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteFeatureImpl::referencesFromClipboard(std::vector<QString>& referenceList)
{
    QClipboard* clipboard = QApplication::clipboard();
    if (!clipboard) return;

    const MimeDataWithReferences* mimeDataReferences = dynamic_cast<const MimeDataWithReferences*>(clipboard->mimeData());
    if (!mimeDataReferences) return;

    referenceList = mimeDataReferences->references();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteFeatureImpl::findObjectsFromClipboardRefs(caf::PdmObjectGroup* objectGroup)
{
    std::vector<QString> referenceList;
    RicPasteFeatureImpl::referencesFromClipboard(referenceList);

    RicPasteFeatureImpl::populateObjectGroupFromReferences(referenceList, objectGroup);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RicPasteFeatureImpl::findGridCaseGroup(caf::PdmObjectHandle* objectHandle)
{
    if (dynamic_cast<RimIdenticalGridCaseGroup*>(objectHandle))
    {
        return dynamic_cast<RimIdenticalGridCaseGroup*>(objectHandle);
    }
    else if (dynamic_cast<RimCaseCollection*>(objectHandle) ||
             dynamic_cast<RimEclipseCase*>(objectHandle))
    {
        RimIdenticalGridCaseGroup* gridCaseGroup = NULL;
        objectHandle->firstAncestorOrThisOfType(gridCaseGroup);

        return gridCaseGroup;
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicPasteFeatureImpl::findEclipseCase(caf::PdmObjectHandle* objectHandle)
{
    if (dynamic_cast<RimEclipseCase*>(objectHandle))
    {
        return dynamic_cast<RimEclipseCase*>(objectHandle);
    }
    else if (dynamic_cast<RimEclipseView*>(objectHandle))
    {
        RimEclipseView* reservoirView = dynamic_cast<RimEclipseView*>(objectHandle);

        return reservoirView->eclipseCase();
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RicPasteFeatureImpl::findGeoMechCase(caf::PdmObjectHandle* objectHandle)
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(objectHandle);
    if (!geomCase)
    {
        RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(objectHandle);
        if (geomView) geomCase = geomView->geoMechCase();
    }

    return geomCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteFeatureImpl::setIconAndShortcuts(QAction* action)
{
    if (action)
    {
        action->setIcon(QIcon(":/clipboard.png"));
        action->setShortcuts(QKeySequence::Paste);
    }
}
