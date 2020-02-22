/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicPasteEnsembleCurveSetFeature.h"

#include "RiaSummaryTools.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimSummaryPlot.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicPasteEnsembleCurveSetFeature, "RicPasteEnsembleCurveSetFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet*
    RicPasteEnsembleCurveSetFeature::copyCurveSetAndAddToCollection( RimEnsembleCurveSetCollection* curveSetCollection,
                                                                     const RimEnsembleCurveSet*     sourceCurveSet )
{
    CVF_ASSERT( curveSetCollection );

    RimEnsembleCurveSet* newCurveSet = dynamic_cast<RimEnsembleCurveSet*>(
        sourceCurveSet->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( newCurveSet );

    curveSetCollection->addCurveSet( newCurveSet );

    // Resolve references after object has been inserted into the project data model
    newCurveSet->resolveReferencesRecursively();
    newCurveSet->initAfterReadRecursively();
    newCurveSet->loadDataAndUpdate( false );
    newCurveSet->updateConnectedEditors();

    return newCurveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteEnsembleCurveSetFeature::isCommandEnabled()
{
    caf::PdmObject* destinationObject = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !destinationObject ) return false;

    RimSummaryPlot*                plot = nullptr;
    RimEnsembleCurveSetCollection* coll = nullptr;
    destinationObject->firstAncestorOrThisOfType( plot );
    destinationObject->firstAncestorOrThisOfType( coll );
    if ( !coll && !plot )
    {
        return false;
    }

    if ( ensembleCurveSetsOnClipboard().empty() )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteEnsembleCurveSetFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmPointer<RimEnsembleCurveSet>> sourceObjects =
        RicPasteEnsembleCurveSetFeature::ensembleCurveSetsOnClipboard();
    if ( sourceObjects.empty() ) return;

    RimSummaryPlot*                plot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot*>();
    RimEnsembleCurveSetCollection* coll = caf::firstAncestorOfTypeFromSelectedObject<RimEnsembleCurveSetCollection*>();
    if ( !coll && plot )
    {
        coll = plot->ensembleCurveSetCollection();
    }

    if ( !coll ) return;

    for ( const auto& sourceObject : sourceObjects )
    {
        copyCurveSetAndAddToCollection( coll, sourceObject );
    }

    if ( plot )
    {
        plot->updateAll();
    }

    coll->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteEnsembleCurveSetFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste Ensemble Curve Set" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimEnsembleCurveSet>> RicPasteEnsembleCurveSetFeature::ensembleCurveSetsOnClipboard()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimEnsembleCurveSet>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    return typedObjects;
}
