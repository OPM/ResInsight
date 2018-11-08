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

#include "RicNew2dContourViewFeature.h"

#include "Rim2dEclipseView.h"
#include "Rim2dEclipseViewCollection.h"
#include "RimCellEdgeColors.h"
#include "RimEclipseView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "Rim3dView.h"

#include "Riu3DMainWindowTools.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "cafPdmDocument.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNew2dContourViewFeature, "RicNew2dContourViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNew2dContourViewFeature::isCommandEnabled()
{
    bool selectedView = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>() != nullptr;
    bool selectedCase = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseCase>() != nullptr;
    bool selectedMapCollection = caf::SelectionManager::instance()->selectedItemOfType<Rim2dEclipseViewCollection>();
    return selectedView || selectedCase || selectedMapCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNew2dContourViewFeature::onActionTriggered(bool isChecked)
{
    RimEclipseView* reservoirView = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>();
    RimEclipseCase* eclipseCase = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimEclipseCase>();
    Rim2dEclipseView* contourMap = nullptr;

    // Find case to insert into
    if (reservoirView)
    {
        contourMap = create2dContourMapFrom3dView(eclipseCase, reservoirView);
    }
    else if (eclipseCase)
    {
        contourMap = create2dContourMap(eclipseCase);
    }

    if (contourMap)
    {
        // Must be run before buildViewItems, as wells are created in this function
        contourMap->loadDataAndUpdate();

        if (eclipseCase)
        {
            eclipseCase->updateConnectedEditors();
        }
        caf::SelectionManager::instance()->setSelectedItem(contourMap);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNew2dContourViewFeature::setupActionLook(QAction* actionToSetup)
{
    Rim2dEclipseView* contourMap = caf::SelectionManager::instance()->selectedItemOfType<Rim2dEclipseView>();
    RimEclipseView* eclipseView  = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>();
    if (contourMap)
    {
        actionToSetup->setText("Duplicate Contour Map");
    }
    else if (eclipseView)
    {
        actionToSetup->setText("New Contour Map From 3d View");
    }
    else
    {
        actionToSetup->setText("New Contour Map");
    }
    actionToSetup->setIcon(QIcon(":/2DMap16x16.png"));
}    

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim2dEclipseView* RicNew2dContourViewFeature::create2dContourMapFrom3dView(RimEclipseCase* eclipseCase, const RimEclipseView* sourceView)
{
    Rim2dEclipseView* contourMap = dynamic_cast<Rim2dEclipseView*>(sourceView->xmlCapability()->copyAndCastByXmlSerialization(
        Rim2dEclipseView::classKeywordStatic(), sourceView->classKeyword(), caf::PdmDefaultObjectFactory::instance()));
    CVF_ASSERT(contourMap);

    contourMap->setEclipseCase(eclipseCase);
    contourMap->setBackgroundColor(cvf::Color3f(1.0f, 1.0f, 0.98f)); // Ignore original view background

    caf::PdmDocument::updateUiIconStateRecursively(contourMap);

    size_t i = eclipseCase->contourMapCollection()->views().size();
    contourMap->setName(QString("Contour Map %1").arg(i + 1));
    eclipseCase->contourMapCollection()->push_back(contourMap);

    // Resolve references after contour map has been inserted into Rim structures
    contourMap->resolveReferencesRecursively();
    contourMap->initAfterReadRecursively();

    return contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim2dEclipseView* RicNew2dContourViewFeature::create2dContourMap(RimEclipseCase* eclipseCase)
{
    Rim2dEclipseView* contourMap = new Rim2dEclipseView();
    contourMap->setEclipseCase(eclipseCase);

    // Set default values
    {
        contourMap->cellResult()->setResultType(RiaDefines::DYNAMIC_NATIVE);

        if (RiaApplication::instance()->preferences()->loadAndShowSoil)
        {
            contourMap->cellResult()->setResultVariable("SOIL");
        }

        contourMap->hasUserRequestedAnimation = true;
        contourMap->setBackgroundColor(cvf::Color3f(1.0f, 1.0f, 0.98f));
        contourMap->initAfterReadRecursively();
        contourMap->zoomAll();
    }

    caf::PdmDocument::updateUiIconStateRecursively(contourMap);

    size_t i = eclipseCase->contourMapCollection()->views().size();
    contourMap->setName(QString("Contour Map %1").arg(i + 1));
    eclipseCase->contourMapCollection()->push_back(contourMap);

    return contourMap;
}
