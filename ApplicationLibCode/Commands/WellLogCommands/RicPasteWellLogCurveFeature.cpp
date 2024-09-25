/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016      Statoil ASA
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

#include "RicPasteWellLogCurveFeature.h"
#include "RicWellLogPlotCurveFeatureImpl.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimWellBoreStabilityPlot.h"
#include "RimWellLogCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogLasFileCurve.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellMeasurementCurve.h"

#include "cafPdmObjectGroup.h"
#include "cafPdmObjectHandle.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicPasteWellLogCurveFeature, "RicPasteWellLogCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteWellLogCurveFeature::isCommandEnabled() const
{
    if ( RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot() ) return false;
    if ( RicWellLogPlotCurveFeatureImpl::parentWellRftPlot() ) return false;

    auto* destinationObject = dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !destinationObject ) return false;

    RimWellLogTrack* wellLogTrack = destinationObject->firstAncestorOrThisOfType<RimWellLogTrack>();
    if ( !wellLogTrack )
    {
        return false;
    }

    RimWellBoreStabilityPlot* wbsPlotToPasteInto = wellLogTrack->firstAncestorOrThisOfType<RimWellBoreStabilityPlot>();

    std::vector<caf::PdmPointer<RimWellLogCurve>> sourceObjects = RicPasteWellLogCurveFeature::curves();

    for ( const auto& sourceObject : sourceObjects )
    {
        RimWellBoreStabilityPlot* originalWbsPlot = sourceObject->firstAncestorOrThisOfType<RimWellBoreStabilityPlot>();
        if ( originalWbsPlot && originalWbsPlot != wbsPlotToPasteInto )
        {
            return false;
        }
    }
    return !RicPasteWellLogCurveFeature::curves().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteWellLogCurveFeature::onActionTriggered( bool isChecked )
{
    if ( RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot() ) return;

    auto* destinationObject = dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !destinationObject ) return;

    RimWellLogTrack* wellLogTrack = destinationObject->firstAncestorOrThisOfType<RimWellLogTrack>();
    if ( !wellLogTrack )
    {
        return;
    }

    RimWellBoreStabilityPlot* wbsPlotToPasteInto = wellLogTrack->firstAncestorOrThisOfType<RimWellBoreStabilityPlot>();

    std::vector<caf::PdmPointer<RimWellLogCurve>> sourceObjects = RicPasteWellLogCurveFeature::curves();

    for ( const auto& sourceObject : sourceObjects )
    {
        RimWellBoreStabilityPlot* originalWbsPlot = sourceObject->firstAncestorOrThisOfType<RimWellBoreStabilityPlot>();
        if ( originalWbsPlot && originalWbsPlot != wbsPlotToPasteInto )
        {
            continue;
        }

        auto* fileCurve        = dynamic_cast<RimWellLogCurve*>( sourceObject.p() );
        auto* measurementCurve = dynamic_cast<RimWellMeasurementCurve*>( sourceObject.p() );
        auto* extractionCurve  = dynamic_cast<RimWellLogExtractionCurve*>( sourceObject.p() );
        auto* rftCurve         = dynamic_cast<RimWellLogRftCurve*>( sourceObject.p() );
        if ( fileCurve || measurementCurve || extractionCurve || rftCurve )
        {
            auto* newObject = sourceObject->copyObject<RimWellLogCurve>();
            CVF_ASSERT( newObject );

            wellLogTrack->addCurve( newObject );

            // Resolve references after object has been inserted into the project data model
            newObject->resolveReferencesRecursively();
            newObject->initAfterReadRecursively();

            newObject->loadDataAndUpdate( true );

            wellLogTrack->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteWellLogCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste Well Log Curve" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimWellLogCurve>> RicPasteWellLogCurveFeature::curves()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimWellLogCurve>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    return typedObjects;
}
