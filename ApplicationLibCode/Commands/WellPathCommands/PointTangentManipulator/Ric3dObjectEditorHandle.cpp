/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "Ric3dObjectEditorHandle.h"

#include "Rim3dView.h"
#include "RiuViewer.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* Ric3dObjectEditorHandle::mainOrComparisonView()
{
    RiuViewer* viewer = dynamic_cast<RiuViewer*>( ownerViewer() );

    if ( !viewer ) return nullptr;

    if ( isInComparisonView() )
    {
        if ( viewer->currentScene( isInComparisonView() ) )
        {
            Rim3dView* view = dynamic_cast<Rim3dView*>( viewer->ownerReservoirView() );

            if ( view )
            {
                return view->activeComparisonView();
            }
        }
        return nullptr;
    }
    else
    {
        return dynamic_cast<Rim3dView*>( viewer->ownerReservoirView() );
    }
}
