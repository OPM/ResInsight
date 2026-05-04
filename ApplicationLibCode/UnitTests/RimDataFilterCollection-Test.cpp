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

#include "RimCellFilter.h"
#include "RimCellRangeFilter.h"
#include "RimCombinedFilter.h"
#include "RimDataFilterCollection.h"

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
