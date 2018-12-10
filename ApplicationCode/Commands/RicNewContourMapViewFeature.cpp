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

#include "RicNewContourMapViewFeature.h"

#include "RimContourMapView.h"
#include "RimContourMapViewCollection.h"
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

CAF_CMD_SOURCE_INIT(RicNewContourMapViewFeature, "RicNewContourMapViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewContourMapViewFeature::isCommandEnabled()
{
    bool selectedView = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>() != nullptr;
    bool selectedCase = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseCase>() != nullptr;
    bool selectedMapCollection = caf::SelectionManager::instance()->selectedItemOfType<RimContourMapViewCollection>();
    return selectedView || selectedCase || selectedMapCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewContourMapViewFeature::onActionTriggered(bool isChecked)
{
    RimEclipseView* reservoirView = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>();
    RimContourMapView* existingContourMap = caf::SelectionManager::instance()->selectedItemOfType<RimContourMapView>();
    RimEclipseCase* eclipseCase = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimEclipseCase>();
    RimContourMapView* contourMap = nullptr;

    // Find case to insert into
    if (existingContourMap)
    {
        contourMap = create2dContourMapFromExistingContourMap(eclipseCase, existingContourMap);
    }
    else if (reservoirView)
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

        contourMap->createDisplayModelAndRedraw();
        contourMap->zoomAll();

        Riu3DMainWindowTools::setExpanded(contourMap);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewContourMapViewFeature::setupActionLook(QAction* actionToSetup)
{
    RimContourMapView* contourMap = caf::SelectionManager::instance()->selectedItemOfType<RimContourMapView>();
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
RimContourMapView* RicNewContourMapViewFeature::create2dContourMapFromExistingContourMap(RimEclipseCase* eclipseCase, RimContourMapView* existingContourMap)
{
    RimContourMapView* contourMap =
        dynamic_cast<RimContourMapView*>(existingContourMap->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
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
RimContourMapView* RicNewContourMapViewFeature::create2dContourMapFrom3dView(RimEclipseCase* eclipseCase, const RimEclipseView* sourceView)
{
    RimContourMapView* contourMap = dynamic_cast<RimContourMapView*>(sourceView->xmlCapability()->copyAndCastByXmlSerialization(
        RimContourMapView::classKeywordStatic(), sourceView->classKeyword(), caf::PdmDefaultObjectFactory::instance()));
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
RimContourMapView* RicNewContourMapViewFeature::create2dContourMap(RimEclipseCase* eclipseCase)
{
    RimContourMapView* contourMap = new RimContourMapView();
    contourMap->setEclipseCase(eclipseCase);

    // Set default values
    {
        contourMap->cellResult()->setResultType(RiaDefines::DYNAMIC_NATIVE);

        if (RiaApplication::instance()->preferences()->loadAndShowSoil)
        {
            contourMap->cellResult()->setResultVariable("SOIL");
        }       
    }

    caf::PdmDocument::updateUiIconStateRecursively(contourMap);

    size_t i = eclipseCase->contourMapCollection()->views().size();
    contourMap->setName(QString("Contour Map %1").arg(i + 1));
    eclipseCase->contourMapCollection()->push_back(contourMap);

    contourMap->hasUserRequestedAnimation = true;
    contourMap->setBackgroundColor(cvf::Color3f(1.0f, 1.0f, 0.98f));
    contourMap->initAfterReadRecursively();

    return contourMap;
}
