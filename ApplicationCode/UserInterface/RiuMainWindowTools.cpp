/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiuMainWindowTools.h"

#include "RiaGuiApplication.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeView.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowTools::collapseSiblings( const caf::PdmUiItem* sourceUiItem )
{
    if ( !sourceUiItem ) return;

    if ( !RiaGuiApplication::isRunning() ) return;

    {
        caf::PdmUiTreeView*     sourceTreeView         = nullptr;
        caf::PdmUiTreeOrdering* sourceTreeOrderingItem = nullptr;

        {
            QModelIndex modIndex;

            if ( RiuMainWindow::instance() )
            {
                modIndex = RiuMainWindow::instance()->projectTreeView()->findModelIndex( sourceUiItem );
            }

            if ( modIndex.isValid() )
            {
                sourceTreeView = RiuMainWindow::instance()->projectTreeView();
            }
            else
            {
                RiuPlotMainWindow* mpw = RiaGuiApplication::instance()->mainPlotWindow();
                if ( mpw )
                {
                    modIndex = mpw->projectTreeView()->findModelIndex( sourceUiItem );
                    if ( modIndex.isValid() )
                    {
                        sourceTreeView = mpw->projectTreeView();
                    }
                }
            }

            if ( !modIndex.isValid() ) return;

            sourceTreeOrderingItem = static_cast<caf::PdmUiTreeOrdering*>( modIndex.internalPointer() );
        }

        if ( sourceTreeView && sourceTreeOrderingItem && sourceTreeOrderingItem->parent() )
        {
            for ( int i = 0; i < sourceTreeOrderingItem->parent()->childCount(); i++ )
            {
                auto siblingTreeOrderingItem = sourceTreeOrderingItem->parent()->child( i );
                if ( siblingTreeOrderingItem != sourceTreeOrderingItem )
                {
                    sourceTreeView->setExpanded( siblingTreeOrderingItem->activeItem(), false );
                }
            }
        }
    }
}
