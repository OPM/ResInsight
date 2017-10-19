/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicSelectOrCreateViewFeatureImpl.h"

#include "FlowCommands/RicSelectViewUI.h"

#include "RiaApplication.h"

#include "RimProject.h"
#include "RimEclipseView.h"
#include "RimEclipseResultCase.h"

#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"

//==================================================================================================
/// 
//==================================================================================================
RimEclipseView* RicSelectOrCreateViewFeatureImpl::showViewSelection(RimEclipseResultCase* resultCase, const QString& lastUsedViewKey, const QString& dialogTitle)
{
    RimEclipseView* defaultSelectedView = getDefaultSelectedView(resultCase, lastUsedViewKey);

    RicSelectViewUI featureUi;
    if (defaultSelectedView)
    {
        featureUi.setView(defaultSelectedView);
    }
    else
    {
        featureUi.setCase(resultCase);
    }

    caf::PdmUiPropertyViewDialog propertyDialog(NULL, &featureUi, dialogTitle, "");
    propertyDialog.resize(QSize(400, 200));

    if (propertyDialog.exec() != QDialog::Accepted) return nullptr;

    RimEclipseView* viewToManipulate = nullptr;
    if (featureUi.createNewView())
    {
        RimEclipseView* createdView = resultCase->createAndAddReservoirView();
        createdView->name = featureUi.newViewName();

        // Must be run before buildViewItems, as wells are created in this function
        createdView->loadDataAndUpdate();
        resultCase->updateConnectedEditors();

        viewToManipulate = createdView;
    }
    else
    {
        viewToManipulate = featureUi.selectedView();
    }

    QString refFromProjectToView = caf::PdmReferenceHelper::referenceFromRootToObject(RiaApplication::instance()->project(), viewToManipulate);
    RiaApplication::instance()->setCacheDataObject(lastUsedViewKey, refFromProjectToView);

    return viewToManipulate;
}

//==================================================================================================
/// 
//==================================================================================================
void RicSelectOrCreateViewFeatureImpl::focusView(RimEclipseView* view)
{
    RiuMainWindow::instance()->setExpanded(view);
    RiuMainWindow::instance()->selectAsCurrentItem(view);
    RiuMainWindow::instance()->raise();
}

//==================================================================================================
/// 
//==================================================================================================
RimEclipseView* RicSelectOrCreateViewFeatureImpl::getDefaultSelectedView(RimEclipseResultCase* resultCase, const QString& lastUsedViewKey)
{
    RimEclipseView* defaultSelectedView = nullptr;

    QString lastUsedViewRef = RiaApplication::instance()->cacheDataObject(lastUsedViewKey).toString();
    RimEclipseView* lastUsedView = dynamic_cast<RimEclipseView*>(caf::PdmReferenceHelper::objectFromReference(RiaApplication::instance()->project(), lastUsedViewRef));
    if (lastUsedView)
    {
        RimEclipseResultCase* lastUsedViewResultCase = nullptr;
        lastUsedView->firstAncestorOrThisOfTypeAsserted(lastUsedViewResultCase);

        if (lastUsedViewResultCase == resultCase)
        {
            defaultSelectedView = lastUsedView;
        }
    }

    if (!defaultSelectedView)
    {
        RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
        if (activeView)
        {
            RimEclipseResultCase* activeViewResultCase = nullptr;
            activeView->firstAncestorOrThisOfTypeAsserted(activeViewResultCase);

            if (activeViewResultCase == resultCase)
            {
                defaultSelectedView = activeView;
            }
            else
            {
                if (resultCase->views().size() > 0)
                {
                    defaultSelectedView = dynamic_cast<RimEclipseView*>(resultCase->views()[0]);
                }
            }
        }
    }

    return defaultSelectedView;
}
