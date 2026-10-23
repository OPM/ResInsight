#include "gtest/gtest.h"

#include "RimEclipseView.h"
#include "RimIntersectionCollection.h"

#include <memory>

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

//--------------------------------------------------------------------------------------------------
/// Synchronizing 2D intersection views for a collection detached from any Rim3dView ancestor (e.g.
/// mid-destruction, once the owning view has been removed from the PDM tree) must not crash.
//--------------------------------------------------------------------------------------------------
TEST( RimIntersectionCollectionTest, SynchronizeWithoutOwnerViewDoesNotCrash )
{
    auto intersectionCollection = std::make_unique<RimIntersectionCollection>();

    EXPECT_NO_FATAL_FAILURE( intersectionCollection->synchronize2dIntersectionViews() );
}
