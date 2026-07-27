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
#include "RimIjkIntersection.h"
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
    SEL_INTERSECTION_ITEMS
};

static RimIntersectionCollection*                 selectedIntersectionCollection();
static std::vector<RimExtrudedCurveIntersection*> selectedIntersections();
static std::vector<RimBoxIntersection*>           selectedIntersectionBoxes();
static std::vector<RimIjkIntersection*>           selectedIjkIntersections();
static SelectionComposition                       selectionComposition();
static RimCase*                                   commonGridCase( std::vector<caf::PdmUiItem*> selectedItems );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCopyIntersectionsToAllViewsInCaseFeature::isCommandEnabled() const
{
    return selectionComposition() != SEL_INVALID;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyIntersectionsToAllViewsInCaseFeature::onActionTriggered( bool isChecked )
{
    RimCase* gridCase = caf::firstAncestorOfTypeFromSelectedObject<RimCase>();

    if ( gridCase )
    {
        SelectionComposition compostion = selectionComposition();
        if ( compostion == SEL_COLLECTION )
        {
            RimIntersectionCollection* coll = selectedIntersectionCollection();
            copyIntersectionsToOtherViews( *gridCase, coll->intersections() );
            copyIntersectionBoxesToOtherViews( *gridCase, coll->intersectionBoxes() );
            copyIjkIntersectionsToOtherViews( *gridCase, coll->ijkIntersections() );
        }
        else if ( compostion == SEL_INTERSECTION_ITEMS )
        {
            copyIntersectionsToOtherViews( *gridCase, selectedIntersections() );
            copyIntersectionBoxesToOtherViews( *gridCase, selectedIntersectionBoxes() );
            copyIjkIntersectionsToOtherViews( *gridCase, selectedIjkIntersections() );
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
void RicCopyIntersectionsToAllViewsInCaseFeature::copyIntersectionsToOtherViews( RimCase&                                   gridCase,
                                                                                 std::vector<RimExtrudedCurveIntersection*> intersections )
{
    for ( RimExtrudedCurveIntersection* intersection : intersections )
    {
        for ( Rim3dView* const view : gridCase.views() )
        {
            RimGridView* currGridView = dynamic_cast<RimGridView*>( view );
            RimGridView* parentView   = intersection->firstAncestorOrThisOfType<RimGridView>();

            if ( currGridView && parentView != nullptr && parentView != currGridView )
            {
                RimIntersectionCollection* destCollection = currGridView->intersectionCollection();

                auto copy = intersection->copyObject<RimExtrudedCurveIntersection>();
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
void RicCopyIntersectionsToAllViewsInCaseFeature::copyIntersectionBoxesToOtherViews( RimCase&                         gridCase,
                                                                                     std::vector<RimBoxIntersection*> intersectionBoxes )
{
    for ( RimBoxIntersection* intersectionBox : intersectionBoxes )
    {
        for ( Rim3dView* const view : gridCase.views() )
        {
            RimGridView* currGridView = dynamic_cast<RimGridView*>( view );
            RimGridView* parentView   = intersectionBox->firstAncestorOrThisOfType<RimGridView>();

            if ( currGridView && parentView != nullptr && parentView != currGridView )
            {
                RimIntersectionCollection* destCollection = currGridView->intersectionCollection();

                auto copy = intersectionBox->copyObject<RimBoxIntersection>();
                CVF_ASSERT( copy );

                destCollection->appendIntersectionBoxAndUpdate( copy );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyIntersectionsToAllViewsInCaseFeature::copyIjkIntersectionsToOtherViews( RimCase&                         gridCase,
                                                                                    std::vector<RimIjkIntersection*> ijkIntersections )
{
    for ( RimIjkIntersection* ijkIntersection : ijkIntersections )
    {
        for ( Rim3dView* const view : gridCase.views() )
        {
            RimGridView* currGridView = dynamic_cast<RimGridView*>( view );
            RimGridView* parentView   = ijkIntersection->firstAncestorOrThisOfType<RimGridView>();

            if ( currGridView && parentView != nullptr && parentView != currGridView )
            {
                RimIntersectionCollection* destCollection = currGridView->intersectionCollection();

                auto copy = ijkIntersection->copyObject<RimIjkIntersection>();
                CVF_ASSERT( copy );

                destCollection->appendIjkIntersectionAndUpdate( copy );
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
std::vector<RimIjkIntersection*> selectedIjkIntersections()
{
    return caf::selectedObjectsByType<RimIjkIntersection*>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SelectionComposition selectionComposition()
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();

    RimCase* gridCase = commonGridCase( selectedItems );
    if ( gridCase && gridCase->gridViews().size() > 1 )
    {
        RimIntersectionCollection* selColl = selectedIntersectionCollection();

        if ( selColl )
        {
            if ( selectedItems.size() == 1 ) return SEL_COLLECTION;
        }
        else
        {
            if ( !selectedIntersections().empty() || !selectedIntersectionBoxes().empty() || !selectedIjkIntersections().empty() )
                return SEL_INTERSECTION_ITEMS;
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

        RimCase* itemCase = obj->firstAncestorOrThisOfType<RimCase>();

        if ( gridCase == nullptr )
            gridCase = itemCase;
        else if ( gridCase != itemCase )
            return nullptr;
    }
    return gridCase;
}
