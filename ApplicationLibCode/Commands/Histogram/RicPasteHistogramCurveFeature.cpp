/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025   Equinor ASA
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

#include "RicPasteHistogramCurveFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RicHistogramPlotTools.h"

#include "Histogram/RimHistogramCurve.h"
#include "Histogram/RimHistogramCurveCollection.h"
#include "Histogram/RimHistogramPlot.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicPasteHistogramCurveFeature, "RicPasteHistogramCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramCurve* RicPasteHistogramCurveFeature::copyCurveAndAddToPlot( RimHistogramCurve* sourceCurve )
{
    if ( RimHistogramPlot* histPlot = caf::firstAncestorOfTypeFromSelectedObject<RimHistogramPlot>() )
    {
        if ( auto newCurve = sourceCurve->copyObject<RimHistogramCurve>() )
        {
            RicHistogramPlotTools::addHistogramCurveToPlot( histPlot, newCurve, true /*resolve references*/ );
            return newCurve;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteHistogramCurveFeature::isCommandEnabled() const
{
    caf::PdmObject* destinationObject = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !destinationObject ) return false;

    auto destPlot = destinationObject->firstAncestorOrThisOfType<RimHistogramPlot>();
    if ( destPlot == nullptr ) return false;

    if ( histogramCurvesOnClipboard().empty() )
    {
        return false;
    }

    for ( caf::PdmPointer<RimHistogramCurve> curve : histogramCurvesOnClipboard() )
    {
        // Check that owner plot is correct type
        auto ownerPlot = curve->firstAncestorOrThisOfType<RimHistogramPlot>();
        if ( ownerPlot == nullptr ) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteHistogramCurveFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmPointer<RimHistogramCurve>> sourceObjects = RicPasteHistogramCurveFeature::histogramCurvesOnClipboard();

    for ( size_t i = 0; i < sourceObjects.size(); i++ )
    {
        copyCurveAndAddToPlot( sourceObjects[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteHistogramCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste Histogram Curve" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimHistogramCurve>> RicPasteHistogramCurveFeature::histogramCurvesOnClipboard()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimHistogramCurve>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    return typedObjects;
}
