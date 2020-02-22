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

#include "RicCopyIntersectionsToAllViewsInCaseFeature.h"

#include "RimBoxIntersection.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"

#include "cafCmdExecCommandManager.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCopyIntersectionsToAllViewsInCaseFeature, "RicCopyIntersectionsToAllViewsInCaseFeature" );

//--------------------------------------------------------------------------------------------------
/// Internal definitions
//--------------------------------------------------------------------------------------------------
enum SelectionComposition
{
    SEL_INVALID,
    SEL_COLLECTION,
    SEL_INTERSECTIONS,
    SEL_INTERSECTION_BOXES,
    SEL_BOTH_INTERSECTION_TYPES
};

static RimIntersectionCollection*                 selectedIntersectionCollection();
static std::vector<RimExtrudedCurveIntersection*> selectedIntersections();
static std::vector<RimBoxIntersection*>           selectedIntersectionBoxes();
static SelectionComposition                       selectionComposition();
static RimCase*                                   commonGridCase( std::vector<caf::PdmUiItem*> selectedItems );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCopyIntersectionsToAllViewsInCaseFeature::isCommandEnabled()
{
    return selectionComposition() != SEL_INVALID;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyIntersectionsToAllViewsInCaseFeature::onActionTriggered( bool isChecked )
{
    RimCase*                     gridCase = nullptr;
    std::vector<caf::PdmUiItem*> selItems;
    caf::SelectionManager::instance()->selectedItems( selItems );
    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>( selItems.front() );
    if ( objHandle ) objHandle->firstAncestorOrThisOfType( gridCase );

    if ( gridCase )
    {
        SelectionComposition compostion = selectionComposition();
        if ( compostion == SEL_COLLECTION )
        {
            RimIntersectionCollection* coll = selectedIntersectionCollection();
            copyIntersectionsToOtherViews( *gridCase, coll->intersections() );
            copyIntersectionBoxesToOtherViews( *gridCase, coll->intersectionBoxes() );
        }

        std::vector<RimExtrudedCurveIntersection*> selIntersections     = selectedIntersections();
        std::vector<RimBoxIntersection*>           selIntersectionBoxes = selectedIntersectionBoxes();

        if ( compostion == SEL_INTERSECTIONS || compostion == SEL_BOTH_INTERSECTION_TYPES )
        {
            copyIntersectionsToOtherViews( *gridCase, selIntersections );
        }
        if ( compostion == SEL_INTERSECTION_BOXES || compostion == SEL_BOTH_INTERSECTION_TYPES )
        {
            copyIntersectionBoxesToOtherViews( *gridCase, selIntersectionBoxes );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyIntersectionsToAllViewsInCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Copy.png" ) );
    actionToSetup->setText( "Copy intersections to all views in case" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyIntersectionsToAllViewsInCaseFeature::copyIntersectionsToOtherViews(
    RimCase&                                   gridCase,
    std::vector<RimExtrudedCurveIntersection*> intersections )
{
    for ( RimExtrudedCurveIntersection* intersection : intersections )
    {
        for ( Rim3dView* const view : gridCase.views() )
        {
            RimGridView* currGridView = dynamic_cast<RimGridView*>( view );
            RimGridView* parentView   = nullptr;
            intersection->firstAncestorOrThisOfType( parentView );

            if ( currGridView && parentView != nullptr && parentView != currGridView )
            {
                RimIntersectionCollection* destCollection = currGridView->intersectionCollection();

                RimExtrudedCurveIntersection* copy = dynamic_cast<RimExtrudedCurveIntersection*>(
                    intersection->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
                CVF_ASSERT( copy );

                destCollection->appendIntersectionAndUpdate( copy, false );

                // Resolve references after object has been inserted into the project data model
                copy->resolveReferencesRecursively();
                copy->updateConnectedEditors();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyIntersectionsToAllViewsInCaseFeature::copyIntersectionBoxesToOtherViews(
    RimCase&                         gridCase,
    std::vector<RimBoxIntersection*> intersectionBoxes )
{
    for ( RimBoxIntersection* intersectionBox : intersectionBoxes )
    {
        for ( Rim3dView* const view : gridCase.views() )
        {
            RimGridView* currGridView = dynamic_cast<RimGridView*>( view );
            RimGridView* parentView   = nullptr;
            intersectionBox->firstAncestorOrThisOfType( parentView );

            if ( currGridView && parentView != nullptr && parentView != currGridView )
            {
                RimIntersectionCollection* destCollection = currGridView->intersectionCollection();

                RimBoxIntersection* copy = dynamic_cast<RimBoxIntersection*>(
                    intersectionBox->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
                CVF_ASSERT( copy );

                destCollection->appendIntersectionBoxAndUpdate( copy );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection* selectedIntersectionCollection()
{
    std::vector<RimIntersectionCollection*> selObjects = caf::selectedObjectsByType<RimIntersectionCollection*>();
    return !selObjects.empty() ? selObjects[0] : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimExtrudedCurveIntersection*> selectedIntersections()
{
    return caf::selectedObjectsByType<RimExtrudedCurveIntersection*>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimBoxIntersection*> selectedIntersectionBoxes()
{
    return caf::selectedObjectsByType<RimBoxIntersection*>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SelectionComposition selectionComposition()
{
    std::vector<caf::PdmUiItem*> allSelectedObjects;
    caf::SelectionManager::instance()->selectedItems( allSelectedObjects );

    RimCase* gridCase = commonGridCase( allSelectedObjects );
    if ( gridCase && gridCase->gridViews().size() > 1 )
    {
        RimIntersectionCollection*                 selColl              = selectedIntersectionCollection();
        std::vector<RimExtrudedCurveIntersection*> selIntersections     = selectedIntersections();
        std::vector<RimBoxIntersection*>           selIntersectionBoxes = selectedIntersectionBoxes();

        if ( selColl )
        {
            if ( allSelectedObjects.size() == 1 ) return SEL_COLLECTION;
        }
        else
        {
            if ( !selIntersections.empty() && !selIntersectionBoxes.empty() )
                return SEL_BOTH_INTERSECTION_TYPES;
            else if ( !selIntersections.empty() )
                return SEL_INTERSECTIONS;
            else if ( !selIntersectionBoxes.empty() )
                return SEL_INTERSECTION_BOXES;
        }
    }
    return SEL_INVALID;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* commonGridCase( std::vector<caf::PdmUiItem*> selectedItems )
{
    RimCase* gridCase = nullptr;

    for ( caf::PdmUiItem* item : selectedItems )
    {
        caf::PdmObjectHandle* obj = dynamic_cast<caf::PdmObjectHandle*>( item );
        if ( !obj )
        {
            continue;
        }

        RimCase* itemCase = nullptr;
        obj->firstAncestorOrThisOfType( itemCase );

        if ( gridCase == nullptr )
            gridCase = itemCase;
        else if ( gridCase != itemCase )
            return nullptr;
    }
    return gridCase;
}
