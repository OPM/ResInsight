#include "gtest/gtest.h"

#include "RimEclipseView.h"
#include "RimIntersectionCollection.h"

//--------------------------------------------------------------------------------------------------
/// Synchronizing 2D intersection views for a view whose case is not yet assigned (ownerCase() returns
/// null, e.g. RimEclipseView::eclipseCase() before a case has been set) must not crash. See crash in
/// RimCase::intersectionViewCollection() being called on a null case.
//--------------------------------------------------------------------------------------------------
TEST( RimIntersectionCollectionTest, SynchronizeWithoutOwnerCaseDoesNotCrash )
{
    auto* view = new RimEclipseView();

    RimIntersectionCollection* intersectionCollection = view->intersectionCollection();
    ASSERT_NE( nullptr, intersectionCollection );
    ASSERT_EQ( nullptr, view->ownerCase() );

    EXPECT_NO_FATAL_FAILURE( intersectionCollection->synchronize2dIntersectionViews() );

    delete view;
}
