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

#include "RimCellFilterCollection.h"
#include "RimDataFilterInViewCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimFilterInViewCollection.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmXmlObjectHandle.h"
#include "cafSignal.h"

#include <memory>

namespace
{
bool observesSignal( const caf::SignalObserver* observer, const caf::AbstractSignal* signal )
{
    for ( const auto* observed : observer->observedSignals() )
    {
        if ( observed == signal ) return true;
    }
    return false;
}

RimFilterInViewCollection* filterFacade( RimEclipseView* view )
{
    auto facades = view->descendantsIncludingThisOfType<RimFilterInViewCollection>();
    return facades.empty() ? nullptr : facades.front();
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimFilterInViewCollection, facadeObservesCellFiltersInNewView )
{
    auto view = std::make_unique<RimEclipseView>();

    auto* facade = filterFacade( view.get() );
    ASSERT_TRUE( facade != nullptr );

    EXPECT_EQ( view->cellFilterCollection(), facade->cellFilters() );
    EXPECT_TRUE( observesSignal( facade, &facade->cellFilters()->filtersChanged ) );
    EXPECT_TRUE( observesSignal( facade, &facade->propertyFilters()->filtersChanged ) );
    EXPECT_TRUE( observesSignal( facade, &facade->dataFiltersInView()->wrappersChanged ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimFilterInViewCollection, facadeObservesCellFiltersAfterProjectRead )
{
    auto    sourceView = std::make_unique<RimEclipseView>();
    QString xml        = sourceView->xmlCapability()->writeObjectToXmlString();

    auto readView = std::make_unique<RimEclipseView>();

    const auto* facadeBeforeRead     = filterFacade( readView.get() );
    const auto* collectionBeforeRead = readView->cellFilterCollection();

    readView->xmlCapability()->readObjectFromXmlString( xml, caf::PdmDefaultObjectFactory::instance() );
    readView->xmlCapability()->resolveReferencesRecursively();
    readView->xmlCapability()->initAfterReadRecursively();

    auto* facade = filterFacade( readView.get() );
    ASSERT_TRUE( facade != nullptr );

    EXPECT_EQ( facadeBeforeRead, facade );
    EXPECT_EQ( collectionBeforeRead, readView->cellFilterCollection() );
    EXPECT_EQ( readView->cellFilterCollection(), facade->cellFilters() );

    // Reading the ptr fields disconnects the signals set up by the view constructor. Without a
    // reconnect, the tree node is not refreshed when a filter is added to a loaded project.
    EXPECT_TRUE( observesSignal( facade, &facade->cellFilters()->filtersChanged ) );
    EXPECT_TRUE( observesSignal( facade, &facade->propertyFilters()->filtersChanged ) );
    EXPECT_TRUE( observesSignal( facade, &facade->dataFiltersInView()->wrappersChanged ) );
}
