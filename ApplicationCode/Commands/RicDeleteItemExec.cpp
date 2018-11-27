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


#include "RicDeleteItemExec.h"
#include "RicDeleteItemExecData.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimCase.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimFormationNamesCollection.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimIntersectionCollection.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimViewLinkerCollection.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiuPlotMainWindow.h"

#include "RimFractureTemplateCollection.h"


#include "cafNotificationCenter.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmDocument.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmUiFieldHandle.h"
#include "cafSelectionManager.h"
#include "Rim2dIntersectionViewCollection.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicDeleteItemExec::name()
{
    return m_commandData->classKeyword();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteItemExec::redo()
{
    caf::PdmFieldHandle* field = caf::PdmReferenceHelper::fieldFromReference(m_commandData->m_rootObject, m_commandData->m_pathToField);

    caf::PdmChildArrayFieldHandle* listField = dynamic_cast<caf::PdmChildArrayFieldHandle*>(field);
    if (listField)
    {
        std::vector<caf::PdmObjectHandle*> children;
        listField->childObjects(&children);

        caf::PdmObjectHandle* obj = children[m_commandData->m_indexToObject];
        caf::SelectionManager::instance()->removeObjectFromAllSelections(obj);

        std::vector<caf::PdmObjectHandle*> referringObjects;
        obj->objectsWithReferringPtrFields(referringObjects);

        if (m_commandData->m_deletedObjectAsXml().isEmpty())
        {
            m_commandData->m_deletedObjectAsXml = xmlObj(obj)->writeObjectToXmlString();
        }

        delete obj;

        listField->erase(m_commandData->m_indexToObject);

        caf::PdmObjectHandle* parentObj = listField->ownerObject();
        parentObj->uiCapability()->updateConnectedEditors();
        
        Rim3dView* view = nullptr;
        parentObj->firstAncestorOrThisOfType(view);

        // Range Filters

        RimCellRangeFilterCollection* rangeFilterColl;
        parentObj->firstAncestorOrThisOfType(rangeFilterColl);

        if (rangeFilterColl)
        {
            rangeFilterColl->updateDisplayModeNotifyManagedViews(nullptr);
        }

        // Prop Filter

        RimEclipsePropertyFilterCollection* eclipsePropColl;
        parentObj->firstAncestorOrThisOfType(eclipsePropColl);
        
        RimGeoMechPropertyFilterCollection* geoMechPropColl;
        parentObj->firstAncestorOrThisOfType(geoMechPropColl);

        if (view && (eclipsePropColl || geoMechPropColl))
        {
            view->scheduleGeometryRegen(PROPERTY_FILTERED);
            view->scheduleCreateDisplayModelAndRedraw();
        }

        // Intersections

        RimIntersectionCollection* crossSectionColl;
        parentObj->firstAncestorOrThisOfType(crossSectionColl);
        if (view && crossSectionColl)
        {
            crossSectionColl->syncronize2dIntersectionViews();
            view->scheduleCreateDisplayModelAndRedraw();
        }
        else
        {
            RimCase* parentCase = dynamic_cast<RimCase*>(parentObj);
            if ( parentCase ) // A view was deleted. Need to update the list of intersection views
            {
                parentCase->intersectionViewCollection()->syncFromExistingIntersections(true);
            }
        }

        // SimWell Fractures
        RimSimWellInView* simWell;
        parentObj->firstAncestorOrThisOfType(simWell);
        if (view && simWell)
        {
            view->scheduleCreateDisplayModelAndRedraw();
        }

        RimFractureTemplateCollection* fracTemplateColl;
        parentObj->firstAncestorOrThisOfType(fracTemplateColl);
        if (fracTemplateColl)
        {
            RimProject* proj = nullptr;
            parentObj->firstAncestorOrThisOfType(proj);
            if (proj)
            {
                proj->scheduleCreateDisplayModelAndRedrawAllViews();
            }

            std::vector<Rim3dView*> views;
            proj->allVisibleViews(views);
            for (Rim3dView* visibleView : views)
            {
                if (dynamic_cast<RimEclipseView*>(visibleView))
                {
                    visibleView->updateConnectedEditors();
                }
            }
        }


        // Well paths

        RimWellPath* wellPath;
        parentObj->firstAncestorOrThisOfType(wellPath);

        if (wellPath)
        {
            wellPath->updateConnectedEditors();
        }

        RimWellPathCollection* wellPathColl;
        parentObj->firstAncestorOrThisOfType(wellPathColl);

        if (wellPathColl)
        {
            wellPathColl->scheduleRedrawAffectedViews();
            wellPathColl->uiCapability()->updateConnectedEditors();
        }

        // Update due to deletion of curves (not tracks, handled separatly)

        RimWellLogPlot* wellLogPlot;
        parentObj->firstAncestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
            wellLogPlot->calculateAvailableDepthRange();
            wellLogPlot->updateDepthZoom();
            RiuPlotMainWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
            mainPlotWindow->updateWellLogPlotToolBar();
        }

        RimWellLogTrack* wellLogPlotTrack;
        parentObj->firstAncestorOrThisOfType(wellLogPlotTrack);
        if (wellLogPlotTrack)
        {
            wellLogPlotTrack->calculateXZoomRangeAndUpdateQwt();
            RiuPlotMainWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
            mainPlotWindow->updateWellLogPlotToolBar();
        }
        
        // Update due to delete plots
        // Make sure the plot collection disappears with the last plot

        RimWellLogPlotCollection* wellLogPlotCollection = dynamic_cast<RimWellLogPlotCollection*>(parentObj);
        if (wellLogPlotCollection)
        {
            if (wellLogPlotCollection->wellLogPlots.empty())
            {
                RimProject* project = nullptr;
                parentObj->firstAncestorOrThisOfType(project);
                if (project)
                {
                    project->updateConnectedEditors();
                }
            }
            RiuPlotMainWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
            mainPlotWindow->updateWellLogPlotToolBar();
        }
        
        // Linked views

        RimViewLinkerCollection* viewLinkerCollection = nullptr;
        parentObj->firstAncestorOrThisOfType(viewLinkerCollection);
        if (viewLinkerCollection)
        {
            viewLinkerCollection->uiCapability()->updateConnectedEditors();

            RimProject* project = nullptr;
            parentObj->firstAncestorOrThisOfType(project);
            if (project)
            {
                // Update visibility of top level Linked Views item in the project tree
                // Not visible if no views are linked
                project->uiCapability()->updateConnectedEditors();
            }
        }

        // Formation names

        RimFormationNamesCollection* formationNamesCollection;
        parentObj->firstAncestorOrThisOfType(formationNamesCollection);
        if (formationNamesCollection)
        {
            for(caf::PdmObjectHandle* reffingObj :referringObjects)
            {
                RimCase* aCase = dynamic_cast<RimCase*>(reffingObj);
                if (aCase) aCase->updateFormationNamesData();
            }
        }


        RimSummaryPlotCollection* summaryPlotCollection = nullptr;
        parentObj->firstAncestorOrThisOfType(summaryPlotCollection);
        if (summaryPlotCollection)
        {
            summaryPlotCollection->updateSummaryNameHasChanged();
            RiuPlotMainWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
            mainPlotWindow->updateSummaryPlotToolBar();
        }

        RimSummaryCrossPlotCollection* summaryCrossPlotCollection = nullptr;
        parentObj->firstAncestorOrThisOfType(summaryCrossPlotCollection);
        if (summaryCrossPlotCollection)
        {
            RiuPlotMainWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
            mainPlotWindow->updateSummaryPlotToolBar();
        }

        RimEnsembleCurveSetCollection* ensembleCurveSetColl = nullptr;
        parentObj->firstAncestorOrThisOfType(ensembleCurveSetColl);
        if (ensembleCurveSetColl)
        {
            RimSummaryPlot* plot = nullptr;
            ensembleCurveSetColl->firstAncestorOrThisOfType(plot);
            if (plot) plot->updateConnectedEditors();
        }

        RimEnsembleCurveFilterCollection* ensembleCurveFilterColl = nullptr;
        parentObj->firstAncestorOrThisOfType(ensembleCurveFilterColl);
        if (ensembleCurveFilterColl)
        {
            RimSummaryPlot* plot = nullptr;
            ensembleCurveFilterColl->firstAncestorOrThisOfType(plot);
            if (plot) plot->loadDataAndUpdate();
        }

        {
            RimAnnotationCollection* annotationColl = nullptr;
            parentObj->firstAncestorOrThisOfType(annotationColl);
            if (annotationColl)
            {
                annotationColl->onAnnotationDeleted();
            }
        }

        {
            RimAnnotationInViewCollection* annotationColl = nullptr;
            parentObj->firstAncestorOrThisOfType(annotationColl);
            if (annotationColl)
            {
                annotationColl->onAnnotationDeleted();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteItemExec::undo()
{
    caf::PdmFieldHandle* field = caf::PdmReferenceHelper::fieldFromReference(m_commandData->m_rootObject, m_commandData->m_pathToField);

    caf::PdmChildArrayFieldHandle* listField = dynamic_cast<caf::PdmChildArrayFieldHandle*>(field);
    if (listField)
    {
        caf::PdmObjectHandle* obj = caf::PdmXmlObjectHandle::readUnknownObjectFromXmlString(m_commandData->m_deletedObjectAsXml(), caf::PdmDefaultObjectFactory::instance());

        listField->insertAt(m_commandData->m_indexToObject, obj);

        obj->xmlCapability()->initAfterReadRecursively();

        listField->uiCapability()->updateConnectedEditors();
        listField->ownerObject()->uiCapability()->updateConnectedEditors();

        if (m_notificationCenter) m_notificationCenter->notifyObservers();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicDeleteItemExec::RicDeleteItemExec(caf::NotificationCenter* notificationCenter)
    : CmdExecuteCommand(notificationCenter)
{
    m_commandData = new RicDeleteItemExecData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicDeleteItemExecData* RicDeleteItemExec::commandData()
{
    return m_commandData;
}
