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

#include "RiaFeatureTestTreeView.h"

#include "RiaFeatureCommandContext.h"

#include "RimProject.h"

#include "cafPdmUiTreeView.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureTestTreeView::RiaFeatureTestTreeView( caf::PdmUiItem* rootItem )
    : m_treeView( std::make_unique<caf::PdmUiTreeView>() )
{
    caf::PdmUiItem* root = rootItem ? rootItem : RimProject::current();
    m_treeView->setPdmItem( root );

    // Register the tree view so RicToggleItemsFeatureImpl::findTreeView() picks it up instead of
    // looking for a (non-existent) RiuMainWindow project tree.
    RiaFeatureCommandContext::instance()->setObject( m_treeView.get() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureTestTreeView::~RiaFeatureTestTreeView()
{
    RiaFeatureCommandContext::instance()->setObject( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiTreeView* RiaFeatureTestTreeView::treeView()
{
    return m_treeView.get();
}
