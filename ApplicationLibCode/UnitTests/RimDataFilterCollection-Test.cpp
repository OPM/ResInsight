/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "gtest/gtest.h"

#include "RigActiveCellInfo.h"
#include "RimCellFilter.h"
#include "RimCellRangeFilter.h"
#include "RimCombinedFilter.h"
#include "RimDataFilterCollection.h"
#include "RimDataFilterInView.h"
#include "RimDataFilterInViewCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimEclipseViewCollection.h"
#include "RimPerforationInterval.h"
#include "RimReservoirGridEnsemble.h"

#include "cafSignal.h"

#include <memory>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Lightweight slot owner used to capture caf::Signal emissions in tests.
//--------------------------------------------------------------------------------------------------
class SignalCounter : public caf::SignalObserver
{
public:
    int  count = 0;
    void onSignal( const caf::SignalEmitter* /*emitter*/ ) { ++count; }
};
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, emptyCollectionHasNoActiveFilters )
{
    RimDataFilterCollection coll;
    EXPECT_EQ( size_t{ 0 }, coll.count() );
    EXPECT_TRUE( coll.isEmpty() );
    EXPECT_TRUE( coll.filters().empty() );
    EXPECT_FALSE( coll.hasActiveFilters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, addFilterIncrementsCount )
{
    RimDataFilterCollection coll;

    auto* a = new RimCellRangeFilter();
    coll.addFilter( a );
    EXPECT_EQ( size_t{ 1 }, coll.count() );
    ASSERT_EQ( size_t{ 1 }, coll.filters().size() );
    EXPECT_EQ( a, coll.filters().front() );

    auto* b = new RimCellRangeFilter();
    coll.addFilter( b );
    EXPECT_EQ( size_t{ 2 }, coll.count() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, removeFilterDeletesAndDecrements )
{
    RimDataFilterCollection coll;

    auto* a = new RimCellRangeFilter();
    coll.addFilter( a );
    EXPECT_EQ( size_t{ 1 }, coll.count() );

    coll.removeFilter( a );
    EXPECT_EQ( size_t{ 0 }, coll.count() );
    EXPECT_TRUE( coll.filters().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, addAndRemoveEmitFiltersChangedSignal )
{
    RimDataFilterCollection coll;
    SignalCounter           counter;
    coll.filtersChanged.connect( &counter, &SignalCounter::onSignal );

    auto* a = new RimCellRangeFilter();
    coll.addFilter( a );
    EXPECT_EQ( 1, counter.count );

    coll.removeFilter( a );
    EXPECT_EQ( 2, counter.count );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, childFilterChangeForwardsSignal )
{
    RimDataFilterCollection coll;
    SignalCounter           counter;
    coll.filtersChanged.connect( &counter, &SignalCounter::onSignal );

    auto* a = new RimCellRangeFilter();
    coll.addFilter( a );
    const int afterAdd = counter.count;

    a->triggerFilterChanged();
    EXPECT_EQ( afterAdd + 1, counter.count );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, hasActiveFiltersReflectsChildState )
{
    RimDataFilterCollection coll;

    auto* a = new RimCellRangeFilter();
    auto* b = new RimCellRangeFilter();
    coll.addFilter( a );
    coll.addFilter( b );

    // Default-constructed RimCellFilter has isActive == true.
    EXPECT_TRUE( coll.hasActiveFilters() );

    a->setActive( false );
    b->setActive( false );
    EXPECT_FALSE( coll.hasActiveFilters() );

    a->setActive( true );
    EXPECT_TRUE( coll.hasActiveFilters() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, combineFiltersMovesChildrenIntoCombinedFilterAtFirstPosition )
{
    RimDataFilterCollection coll;

    auto* a = new RimCellRangeFilter();
    auto* b = new RimCellRangeFilter();
    auto* c = new RimCellRangeFilter();
    coll.addFilter( a );
    coll.addFilter( b );
    coll.addFilter( c );

    EXPECT_EQ( nullptr, coll.combineFilters( { a } ) );
    EXPECT_EQ( size_t{ 3 }, coll.count() );

    RimCombinedFilter* combined = coll.combineFilters( { b, c } );
    ASSERT_NE( nullptr, combined );

    ASSERT_EQ( size_t{ 2 }, coll.count() );
    EXPECT_EQ( a, coll.filters()[0] );
    EXPECT_EQ( combined, coll.filters()[1] );

    const auto children = combined->filters();
    ASSERT_EQ( size_t{ 2 }, children.size() );
    EXPECT_EQ( b, children[0] );
    EXPECT_EQ( c, children[1] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, addNewRangeFilterReturnsConfigured )
{
    RimDataFilterCollection coll;

    RimCellRangeFilter* rangeFilter = coll.addNewRangeFilter();
    ASSERT_NE( nullptr, rangeFilter );
    EXPECT_EQ( size_t{ 1 }, coll.count() );
    EXPECT_EQ( rangeFilter, coll.filters().front() );

    // setDefaultValues(-1, -1) leaves IJK starts >= 1 (1-based Eclipse indexing).
    EXPECT_GE( rangeFilter->startIndexI(), 1 );
    EXPECT_GE( rangeFilter->startIndexJ(), 1 );
    EXPECT_GE( rangeFilter->startIndexK(), 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, addNewCombinedFilterReturnsEmpty )
{
    RimDataFilterCollection coll;

    RimCombinedFilter* combined = coll.addNewCombinedFilter();
    ASSERT_NE( nullptr, combined );
    EXPECT_EQ( size_t{ 1 }, coll.count() );
    EXPECT_TRUE( combined->filters().empty() );

    // A combined filter must be deletable so the generic delete context-menu item is offered (#14148).
    EXPECT_TRUE( combined->isDeletable() );
}

//--------------------------------------------------------------------------------------------------
/// Adding a child to a combined filter that lives in the collection should bubble up the
/// filterChanged signal as a collection-level filtersChanged emission.
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, combinedFilterChildEmitsCollectionSignal )
{
    RimDataFilterCollection coll;

    RimCombinedFilter* combined = coll.addNewCombinedFilter();
    ASSERT_NE( nullptr, combined );

    SignalCounter counter;
    coll.filtersChanged.connect( &counter, &SignalCounter::onSignal );

    auto* child = combined->addNewFilter<RimCellRangeFilter>( []( RimCellRangeFilter* ) {} );
    ASSERT_NE( nullptr, child );

    // RimCombinedFilter::addFilter calls triggerFilterChanged, so the combined filter's signal
    // fires; the collection has connected to that signal and re-emits filtersChanged.
    EXPECT_GE( counter.count, 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, pdmRoundTripPreservesFilters )
{
    QString xml;
    {
        RimDataFilterCollection original;
        original.addNewRangeFilter();
        original.addNewCombinedFilter();
        ASSERT_EQ( size_t{ 2 }, original.count() );

        xml = original.writeObjectToXmlString();
    }

    RimDataFilterCollection restored;
    restored.readObjectFromXmlString( xml, caf::PdmDefaultObjectFactory::instance() );
    restored.resolveReferencesRecursively();

    EXPECT_EQ( size_t{ 2 }, restored.count() );

    auto items = restored.filters();
    ASSERT_EQ( size_t{ 2 }, items.size() );
    EXPECT_NE( nullptr, dynamic_cast<RimCellRangeFilter*>( items[0] ) );
    EXPECT_NE( nullptr, dynamic_cast<RimCombinedFilter*>( items[1] ) );
}

//--------------------------------------------------------------------------------------------------
/// Ensemble filters have no checkbox, so a stored unchecked state is reset to checked on load.
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, ensembleFilterStoredUncheckedIsCheckedOnLoad )
{
    QString xml;
    {
        RimReservoirGridEnsemble original;
        original.dataFilterCollection()->addNewRangeFilter()->setActive( false );
        xml = original.writeObjectToXmlString();
    }

    RimReservoirGridEnsemble restored;
    restored.readObjectFromXmlString( xml, caf::PdmDefaultObjectFactory::instance() );
    restored.xmlCapability()->initAfterReadRecursively();
    restored.resolveReferencesRecursively();

    auto filters = restored.dataFilterCollection()->filters();
    ASSERT_EQ( size_t{ 1 }, filters.size() );
    EXPECT_TRUE( filters[0]->isChecked() );
}

//--------------------------------------------------------------------------------------------------
/// Only ensemble data filters are always on. A filter in a case-level collection has a checkbox,
/// and its stored state must be kept when loading, and when other filters are added.
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, caseLevelFilterKeepsCheckState )
{
    EXPECT_FALSE( RimDataFilterCollection().isEnsembleLevel() );

    QString xml;
    {
        RimDataFilterCollection original;
        original.addNewRangeFilter()->setActive( false );
        xml = original.writeObjectToXmlString();
    }

    RimDataFilterCollection restored;
    restored.readObjectFromXmlString( xml, caf::PdmDefaultObjectFactory::instance() );
    restored.xmlCapability()->initAfterReadRecursively();
    restored.resolveReferencesRecursively();

    ASSERT_EQ( size_t{ 1 }, restored.count() );
    EXPECT_FALSE( restored.filters()[0]->isChecked() );

    restored.addNewRangeFilter();
    EXPECT_FALSE( restored.filters()[0]->isChecked() );
}

//--------------------------------------------------------------------------------------------------
/// Adding a filter must not change the state of the existing filters, also in the ensemble.
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, addingFilterKeepsExistingCheckStateInEnsemble )
{
    RimReservoirGridEnsemble ensemble;
    auto*                    collection = ensemble.dataFilterCollection();
    EXPECT_TRUE( collection->isEnsembleLevel() );

    auto* first = collection->addNewRangeFilter();
    first->setActive( false );
    collection->addNewRangeFilter();

    EXPECT_FALSE( first->isChecked() );
}

//--------------------------------------------------------------------------------------------------
/// Objects referring to a filter that is moved into a combined filter should refer to the combined
/// filter, which is the one selectable among the top-level filters.
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, combineFiltersRepointsReferencesToCombinedFilter )
{
    RimDataFilterCollection coll;
    auto*                   a = coll.addNewRangeFilter();
    auto*                   b = coll.addNewRangeFilter();
    auto*                   c = coll.addNewRangeFilter();

    RimPerforationInterval referringToB;
    referringToB.setCellFilter( b );
    RimPerforationInterval referringToC;
    referringToC.setCellFilter( c );
    RimPerforationInterval referringToA;
    referringToA.setCellFilter( a );

    RimCombinedFilter* combined = coll.combineFilters( { b, c } );
    ASSERT_NE( nullptr, combined );

    EXPECT_EQ( combined, referringToB.cellFilter() );
    EXPECT_EQ( combined, referringToC.cellFilter() );
    EXPECT_EQ( a, referringToA.cellFilter() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, ensembleViewUsesEnsembleDataFilters )
{
    auto  eclipseCase = std::make_unique<RimEclipseResultCase>();
    auto  ensemble    = std::make_unique<RimReservoirGridEnsemble>();
    auto* view        = new RimEclipseView();

    view->setEclipseCase( eclipseCase.get() );
    EXPECT_EQ( eclipseCase->dataFilterCollection(), view->dataFiltersInView()->sourceCollection() );

    ensemble->addView( view );
    EXPECT_EQ( ensemble->dataFilterCollection(), view->dataFiltersInView()->sourceCollection() );
}

//--------------------------------------------------------------------------------------------------
/// A view created via a case's own view collection should still resolve the ensemble's data filters.
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, viewForSingleRealizationUsesEnsembleDataFilters )
{
    auto ensemble = std::make_unique<RimReservoirGridEnsemble>();

    auto* eclipseCase = new RimEclipseResultCase();
    ensemble->addCase( eclipseCase );

    RimEclipseView* view = eclipseCase->viewCollection()->addView( eclipseCase );

    EXPECT_EQ( ensemble->dataFilterCollection(), view->dataFiltersInView()->sourceCollection() );
}

//--------------------------------------------------------------------------------------------------
/// A filter added to the ensemble after the view exists must still appear as a wrapper in the view.
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, viewWrapperAppearsWhenEnsembleFilterAddedAfterViewExists )
{
    auto ensemble = std::make_unique<RimReservoirGridEnsemble>();

    auto* eclipseCase = new RimEclipseResultCase();
    ensemble->addCase( eclipseCase );

    RimEclipseView* view = eclipseCase->viewCollection()->addView( eclipseCase );
    ASSERT_EQ( ensemble->dataFilterCollection(), view->dataFiltersInView()->sourceCollection() );
    EXPECT_TRUE( view->dataFiltersInView()->wrappers().empty() );

    ensemble->dataFilterCollection()->addNewPropertyFilter();

    ASSERT_EQ( size_t{ 1 }, view->dataFiltersInView()->wrappers().size() );
    EXPECT_EQ( ensemble->dataFilterCollection()->filters().front(), view->dataFiltersInView()->wrappers().front()->sourceFilter() );
}

//--------------------------------------------------------------------------------------------------
/// Combining filters replaces their view wrappers with one wrapper for the combined filter, which is
/// applied in the view if any of the combined filters was.
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, combiningFiltersReplacesViewWrappersAndKeepsActivation )
{
    auto ensemble = std::make_unique<RimReservoirGridEnsemble>();

    auto* eclipseCase = new RimEclipseResultCase();
    ensemble->addCase( eclipseCase );

    RimEclipseView* view = eclipseCase->viewCollection()->addView( eclipseCase );

    auto* collection = ensemble->dataFilterCollection();
    auto* filterA    = collection->addNewRangeFilter();
    auto* filterB    = collection->addNewRangeFilter();

    ASSERT_EQ( size_t{ 2 }, view->dataFiltersInView()->wrappers().size() );
    view->dataFiltersInView()->wrappers().front()->setCheckState( true );

    auto* combined = collection->combineFilters( { filterA, filterB } );
    ASSERT_NE( nullptr, combined );

    auto wrappers = view->dataFiltersInView()->wrappers();
    ASSERT_EQ( size_t{ 1 }, wrappers.size() );
    EXPECT_EQ( combined, wrappers.front()->sourceFilter() );
    EXPECT_TRUE( wrappers.front()->isChecked() );
}

//--------------------------------------------------------------------------------------------------
/// Reproduces RicNewViewForGridEnsembleFeature's "New View" command: the view is created via the
/// ensemble's own view collection, which used to lose the ensemble's data filter because the case
/// was assigned before the view was parented. See #14710.
//--------------------------------------------------------------------------------------------------
TEST( RimDataFilterCollection, viewCreatedViaEnsembleViewCollectionUsesEnsembleDataFilters )
{
    auto ensemble = std::make_unique<RimReservoirGridEnsemble>();

    auto* eclipseCase = new RimEclipseResultCase();
    ensemble->addCase( eclipseCase );
    ensemble->dataFilterCollection()->addNewPropertyFilter();

    // Exercises the same call path as RicNewViewForGridEnsembleFeature.
    RimEclipseView* view = ensemble->viewCollection()->addView( eclipseCase );

    EXPECT_EQ( ensemble->dataFilterCollection(), view->dataFiltersInView()->sourceCollection() );
    ASSERT_EQ( size_t{ 1 }, view->dataFiltersInView()->wrappers().size() );
}
