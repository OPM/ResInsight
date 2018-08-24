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
#include "RimWellPath.h"

#include "cafPdmUiPropertyViewDialog.h"

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

    std::vector<RimWellLogCurve*> curves = RicWellLogPlotCurveFeatureImpl::selectedWellLogCurves();

    return curves.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicChangeDataSourceFeature::onActionTriggered(bool isChecked)
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return;

    std::vector<RimWellLogCurve*> curves = RicWellLogPlotCurveFeatureImpl::selectedWellLogCurves();
    if (curves.size() == 0) return;

    RimWellLogCurveCommonDataSource featureUi;
    featureUi.updateDefaultOptions(curves);

    caf::PdmUiPropertyViewDialog propertyDialog(nullptr, &featureUi, "Change Data Source for Multiple Curves", "");
    propertyDialog.resize(QSize(500, 200));

    if (propertyDialog.exec() == QDialog::Accepted)
    {
        featureUi.updateCurves(curves);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicChangeDataSourceFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Change Data Source");
}

