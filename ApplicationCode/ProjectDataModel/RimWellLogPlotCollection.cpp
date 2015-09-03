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

#include "RimWellLogPlotCollection.h"

#include "RimWellLogPlot.h"

#include "cafPdmUiTreeView.h"
#include "RigCaseData.h"
#include "RimWellPath.h"
#include "RimEclipseCase.h"
#include "RigEclipseWellLogExtractor.h"
#include "RimWellPathCollection.h"

CAF_PDM_SOURCE_INIT(RimWellLogPlotCollection, "WellLogPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection::RimWellLogPlotCollection()
{
    CAF_PDM_InitObject("Well Log Plots", "", "", "");

    CAF_PDM_InitFieldNoDefault(&wellLogPlots, "WellLogPlots", "",  "", "", "");
    wellLogPlots.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection::~RimWellLogPlotCollection()
{
    wellLogPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RimWellLogPlotCollection::findOrCreateExtractor(RimWellPath* wellPath, RimEclipseCase* eclCase)
{
    if (!(wellPath && eclCase && wellPath->wellPathGeometry() && eclCase->reservoirData()))
    {
        return NULL;
    }

    RigCaseData* eclCaseData = eclCase->reservoirData();
    RigWellPath* wellPathGeom = wellPath->wellPathGeometry();
    for (size_t exIdx = 0; exIdx < m_extractors.size(); ++exIdx)
    {
         if (m_extractors[exIdx]->caseData() == eclCaseData && m_extractors[exIdx]->wellPathData() == wellPathGeom)
         {
            return m_extractors[exIdx].p();
         }
    }

    cvf::ref<RigEclipseWellLogExtractor> extractor = new RigEclipseWellLogExtractor(eclCaseData, wellPathGeom);
    m_extractors.push_back(extractor.p());

    return extractor.p();
}
