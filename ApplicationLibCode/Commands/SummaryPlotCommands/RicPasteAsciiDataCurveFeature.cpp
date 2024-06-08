/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicPasteAsciiDataCurveFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>

#include "RimAsciiDataCurve.h"
#include "RimSummaryPlot.h"

CAF_CMD_SOURCE_INIT( RicPasteAsciiDataCurveFeature, "RicPasteAsciiDataCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteAsciiDataCurveFeature::isCommandEnabled() const
{
    auto summaryPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>();
    if ( !summaryPlot )
    {
        return false;
    }

    return !RicPasteAsciiDataCurveFeature::asciiDataCurves().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataCurveFeature::onActionTriggered( bool isChecked )
{
    auto summaryPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot>();
    if ( !summaryPlot )
    {
        return;
    }

    std::vector<caf::PdmPointer<RimAsciiDataCurve>> sourceObjects = RicPasteAsciiDataCurveFeature::asciiDataCurves();

    for ( size_t i = 0; i < sourceObjects.size(); i++ )
    {
        auto newObject = sourceObjects[i]->copyObject<RimAsciiDataCurve>();
        CVF_ASSERT( newObject );

        summaryPlot->addAsciiDataCruve( newObject );

        // Resolve references after object has been inserted into the project data model
        newObject->resolveReferencesRecursively();

        // If source curve is part of a curve filter, resolve of references to the summary case does not
        // work when pasting the new curve into a plot. Must set summary case manually.
        // newObject->setSummaryCase(sourceObjects[i]->summaryCase());

        newObject->initAfterReadRecursively();

        newObject->loadDataAndUpdate( true );
        newObject->updateConnectedEditors();

        summaryPlot->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste ASCII Data Curve" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimAsciiDataCurve>> RicPasteAsciiDataCurveFeature::asciiDataCurves()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimAsciiDataCurve>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    return typedObjects;
}
