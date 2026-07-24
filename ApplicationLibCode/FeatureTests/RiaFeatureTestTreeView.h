/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#pragma once

#include <memory>

namespace caf
{
class PdmUiItem;
class PdmUiTreeView;
} // namespace caf

//==================================================================================================
/// RAII helper that builds a headless project tree view and registers it as the active tree view
/// via RiaFeatureCommandContext.
///
/// Tree-dependent features (e.g. the RicToggleItems*Feature family) resolve the children of the
/// selected object through RicToggleItemsFeatureImpl::findTreeView(), which first consults
/// RiaFeatureCommandContext for an externally supplied caf::PdmUiTreeView before falling back to the
/// RiuMainWindow project tree. The feature-test executable has no RiuMainWindow (and no OpenGL
/// context), so this helper supplies a stand-alone tree view instead, making those features testable.
///
/// The tree view eagerly builds the full tree ordering for its root at construction, so add every
/// object the test needs before creating the helper. The command-context registration is cleared and
/// the tree view destroyed when the helper goes out of scope.
//==================================================================================================
class RiaFeatureTestTreeView
{
public:
    // Roots the tree at the given item, or at RimProject::current() when null. The root item itself
    // is not shown as a node; its descendants are, so root above the object you intend to select.
    explicit RiaFeatureTestTreeView( caf::PdmUiItem* rootItem = nullptr );
    ~RiaFeatureTestTreeView();

    RiaFeatureTestTreeView( const RiaFeatureTestTreeView& )            = delete;
    RiaFeatureTestTreeView& operator=( const RiaFeatureTestTreeView& ) = delete;

    caf::PdmUiTreeView* treeView();

private:
    std::unique_ptr<caf::PdmUiTreeView> m_treeView;
};
