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

#include "RicCopyReferencesToClipboardFeature.h"

#include "RimBoxIntersection.h"
#include "RimCellFilter.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEnsembleCurveSet.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimFractureTemplate.h"
#include "RimGeoMechView.h"
#include "RimGridCrossPlotCurve.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimMimeData.h"
#include "RimModeledWellPath.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellRftPlot.h"

#include "cafPdmObject.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>

CAF_CMD_SOURCE_INIT( RicCopyReferencesToClipboardFeature, "RicCopyReferencesToClipboardFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCopyReferencesToClipboardFeature::isCommandEnabled() const
{
    return isAnyCopyableObjectSelected();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyReferencesToClipboardFeature::onActionTriggered( bool isChecked )
{
    disableModelChangeContribution();

    if ( !isAnyCopyableObjectSelected() ) return;

    std::vector<QString> referenceList;

    for ( caf::PdmObject* pdmObject : caf::SelectionManager::instance()->objectsByType<caf::PdmObject>() )
    {
        if ( RicCopyReferencesToClipboardFeature::isCopyOfObjectSupported( pdmObject ) )
        {
            QString itemRef =
                caf::PdmReferenceHelper::referenceFromRootToObject( caf::SelectionManager::instance()->pdmRootObject(), pdmObject );

            referenceList.push_back( itemRef );
        }
    }

    MimeDataWithReferences* myObject = new MimeDataWithReferences;
    myObject->setReferences( referenceList );

    QClipboard* clipboard = QApplication::clipboard();
    if ( clipboard )
    {
        clipboard->setMimeData( myObject );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyReferencesToClipboardFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Copy" );
    actionToSetup->setIcon( QIcon( ":/Copy.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Copy );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCopyReferencesToClipboardFeature::isAnyCopyableObjectSelected()
{
    for ( caf::PdmObject* pdmObject : caf::SelectionManager::instance()->objectsByType<caf::PdmObject>() )
    {
        if ( RicCopyReferencesToClipboardFeature::isCopyOfObjectSupported( pdmObject ) )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCopyReferencesToClipboardFeature::isCopyOfObjectSupported( caf::PdmObject* pdmObject )
{
    // Copy support based on direct dynamic cast
    if ( dynamic_cast<RimGeoMechView*>( pdmObject ) ) return true;
    if ( dynamic_cast<RimEclipseView*>( pdmObject ) ) return true;
    if ( dynamic_cast<RimEclipseCase*>( pdmObject ) ) return true;
    if ( dynamic_cast<RimExtrudedCurveIntersection*>( pdmObject ) ) return true;
    if ( dynamic_cast<RimBoxIntersection*>( pdmObject ) ) return true;
    if ( dynamic_cast<RimSummaryPlot*>( pdmObject ) ) return true;
    if ( dynamic_cast<RimFractureTemplate*>( pdmObject ) ) return true;
    if ( dynamic_cast<RimEnsembleCurveSet*>( pdmObject ) ) return true;
    if ( dynamic_cast<RimGridCrossPlotDataSet*>( pdmObject ) ) return true;
    if ( dynamic_cast<RimModeledWellPath*>( pdmObject ) ) return true;
    if ( dynamic_cast<RimSummaryMultiPlot*>( pdmObject ) ) return true;
    if ( dynamic_cast<RimCellFilter*>( pdmObject ) ) return true;

    // Copy support based combined logic
    RimWellAllocationPlot* wellAllocPlot = pdmObject->firstAncestorOrThisOfType<RimWellAllocationPlot>();
    RimWellRftPlot*        rftPlot       = pdmObject->firstAncestorOrThisOfType<RimWellRftPlot>();

    if ( dynamic_cast<RimPlotCurve*>( pdmObject ) && !dynamic_cast<RimGridCrossPlotCurve*>( pdmObject ) )
    {
        if ( !rftPlot ) return true;
    }
    if ( dynamic_cast<RimWellLogTrack*>( pdmObject ) )
    {
        if ( !wellAllocPlot && !rftPlot ) return true;
    }
    if ( dynamic_cast<RimWellLogPlot*>( pdmObject ) )
    {
        if ( !wellAllocPlot && !rftPlot ) return true;
    }

    return false;
}
