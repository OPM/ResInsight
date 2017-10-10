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

#include "RicChangeDataSourceFeatureUi.h"
#include "RicWellLogPlotCurveFeatureImpl.h"

#include "RimCase.h"
#include "RimWellLogCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellPath.h"

#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicChangeDataSourceFeature, "RicChangeDataSourceFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicChangeDataSourceFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return false;
    if (RicWellLogPlotCurveFeatureImpl::parentWellRftPlot()) return false;

    std::vector<RimWellLogExtractionCurve*> extrCurves = extractionCurves();

    return extrCurves.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicChangeDataSourceFeature::onActionTriggered(bool isChecked)
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return;

    std::vector<RimWellLogExtractionCurve*> extrCurves = extractionCurves();
    if (extrCurves.size() == 0) return;

    RicChangeDataSourceFeatureUi featureUi;
    featureUi.wellPathToApply = extrCurves[0]->wellPath();
    featureUi.caseToApply = extrCurves[0]->rimCase();

    caf::PdmUiPropertyViewDialog propertyDialog(nullptr, &featureUi, "Change Data Source for Selected Curves", "");
    propertyDialog.resize(QSize(500, 200));
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        for (RimWellLogExtractionCurve* extractionCurve : extrCurves)
        {
            extractionCurve->setWellPath(featureUi.wellPathToApply);
            extractionCurve->setCase(featureUi.caseToApply);

            extractionCurve->loadDataAndUpdate(true);
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
std::vector<RimWellLogExtractionCurve*> RicChangeDataSourceFeature::extractionCurves()
{
    std::vector<RimWellLogExtractionCurve*> extrCurves;

    std::vector<RimWellLogCurve*> curves = RicWellLogPlotCurveFeatureImpl::selectedWellLogCurves();

    for (RimWellLogCurve* c : curves)
    {
        if (dynamic_cast<RimWellLogExtractionCurve*>(c))
        {
            extrCurves.push_back(dynamic_cast<RimWellLogExtractionCurve*>(c));
        }
    }

    return extrCurves;
}

