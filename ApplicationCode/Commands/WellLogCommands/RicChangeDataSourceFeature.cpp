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

#include "RicChangeDataSourceFeature.h"

#include "RicWellLogPlotCurveFeatureImpl.h"

#include "RimCase.h"
#include "RimWellLogCurve.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>

#include <set>

CAF_CMD_SOURCE_INIT(RicChangeDataSourceFeature, "RicChangeDataSourceFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicChangeDataSourceFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return false;
    if (RicWellLogPlotCurveFeatureImpl::parentWellRftPlot()) return false;

    std::vector<RimWellLogCurve*> curves;
    std::vector<RimWellLogTrack*> tracks;

    return selectedTracksAndCurves(&curves, &tracks);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicChangeDataSourceFeature::onActionTriggered(bool isChecked)
{
    std::vector<caf::PdmObject*> selectedObjects;
    caf::SelectionManager::instance()->objectsByType(&selectedObjects);

    std::vector<RimWellLogCurve*> curves;
    std::vector<RimWellLogTrack*> tracks;

    if (selectedTracksAndCurves(&curves, &tracks))
    {
        RimWellLogCurveCommonDataSource featureUi;
        featureUi.updateDefaultOptions(curves, tracks);

        caf::PdmUiPropertyViewDialog propertyDialog(nullptr, &featureUi, "Change Data Source for Multiple Curves", "");
        propertyDialog.resize(QSize(500, 200));

        if (propertyDialog.exec() == QDialog::Accepted)
        {
            featureUi.updateCurvesAndTracks(curves, tracks);
        }
    }    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicChangeDataSourceFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Change Data Source");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicChangeDataSourceFeature::selectedTracksAndCurves(std::vector<RimWellLogCurve*>* curves, std::vector<RimWellLogTrack*>* tracks)
{
    CVF_ASSERT(curves && tracks);
    std::vector<caf::PdmObject*> selectedObjects;
    caf::SelectionManager::instance()->objectsByType(&selectedObjects);

    if (selectedObjects.empty()) return false;

    for (caf::PdmObject* selectedObject : selectedObjects)
    {
        RimWellLogTrack*           wellLogTrack = dynamic_cast<RimWellLogTrack*>(selectedObject);
        RimWellLogExtractionCurve* wellLogExtractionCurve = dynamic_cast<RimWellLogExtractionCurve*>(selectedObject);
        RimWellLogFileCurve*       wellLogFileCurve = dynamic_cast<RimWellLogFileCurve*>(selectedObject);
        if (wellLogTrack)
        {
            tracks->push_back(wellLogTrack);
        }
        else if (wellLogExtractionCurve)
        {
            curves->push_back(wellLogExtractionCurve);
        }
        else if (wellLogFileCurve)
        {
            curves->push_back(wellLogFileCurve);
        }
    }

    return selectedObjects.size() == (curves->size() + tracks->size());
}

