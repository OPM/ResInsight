/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicWellLogPlotCurveFeatureImpl.h"

#include "RiaColorTables.h"

#include "RimWellAllocationPlot.h"
#include "RimWellBoreStabilityPlot.h"
#include "RimWellLogCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellRftPlot.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QColor>

//--------------------------------------------------------------------------------------------------
/// Pick default curve color from an index based palette
//--------------------------------------------------------------------------------------------------
cvf::Color3f RicWellLogPlotCurveFeatureImpl::curveColorFromTable( size_t index )
{
    return RiaColorTables::wellLogPlotPaletteColors().cycledColor3f( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogCurve*> RicWellLogPlotCurveFeatureImpl::selectedWellLogCurves()
{
    // Use std::set to determine uniqueness but a vector for inserting curves.
    // This is to retain deterministic order.
    std::vector<RimWellLogCurve*> allCurves;
    std::set<RimWellLogCurve*>    uniqueCurves;

    {
        const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
        for ( caf::PdmUiItem* selectedItem : selectedItems )
        {
            caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>( selectedItem );
            if ( objHandle )
            {
                std::vector<RimWellLogTrack*> childTracks = objHandle->descendantsIncludingThisOfType<RimWellLogTrack>();
                for ( auto track : childTracks )
                {
                    if ( track->showWindow() )
                    {
                        for ( RimWellLogCurve* curve : track->visibleCurves() )
                        {
                            if ( !uniqueCurves.count( curve ) )
                            {
                                uniqueCurves.insert( curve );
                                allCurves.push_back( curve );
                            }
                        }
                    }
                }
            }
        }
    }

    return allCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot* RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()
{
    return caf::firstAncestorOfTypeFromSelectedObject<RimWellAllocationPlot>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellRftPlot* RicWellLogPlotCurveFeatureImpl::parentWellRftPlot()
{
    return caf::firstAncestorOfTypeFromSelectedObject<RimWellRftPlot>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellBoreStabilityPlot* RicWellLogPlotCurveFeatureImpl::parentWellBoreStabilityPlot()
{
    return caf::firstAncestorOfTypeFromSelectedObject<RimWellBoreStabilityPlot>();
}
