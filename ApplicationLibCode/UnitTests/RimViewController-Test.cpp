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

#include "RimViewController.h"

//--------------------------------------------------------------------------------------------------
/// RimViewController::masterView() used to dereference ownerViewLinker() unchecked. When a view
/// controller has been detached from its RimViewLinker parent (e.g. mid-destruction, after the
/// owning child-array field has already erased it but before `delete` runs), ownerViewLinker()
/// returns null and the call used to crash. See the crash in RicDeleteItemExec::redo() -> ~RimViewController()
/// -> removeOverrides() -> ... -> masterView().
//--------------------------------------------------------------------------------------------------
TEST( RimViewController, MasterViewWithoutOwnerViewLinkerDoesNotCrash )
{
    RimViewController controller;
    EXPECT_EQ( nullptr, controller.masterView() );
    EXPECT_FALSE( controller.isPropertyFilterOveridden() );
}
