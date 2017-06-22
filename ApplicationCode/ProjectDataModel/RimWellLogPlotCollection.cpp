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

#include "RigEclipseWellLogExtractor.h"
#include "RigGeoMechCaseData.h"
#include "RigGeoMechWellLogExtractor.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimWellLogPlot.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cvfAssert.h"

CAF_PDM_SOURCE_INIT(RimWellLogPlotCollection, "WellLogPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection::RimWellLogPlotCollection()
{
    CAF_PDM_InitObject("Well Log Plots", ":/WellLogPlots16x16.png", "", "");

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
RigEclipseWellLogExtractor* RimWellLogPlotCollection::findOrCreateSimWellExtractor(const QString& simWellName, 
                                                                                   const QString& caseUserDescription, 
                                                                                   const RigWellPath* wellPathGeom, 
                                                                                   const RigEclipseCaseData* eclCaseData)
{
    if (!(wellPathGeom && eclCaseData))
    {
        return nullptr;
    }

    for (size_t exIdx = 0; exIdx < m_extractors.size(); ++exIdx)
    {
        if (m_extractors[exIdx]->caseData() == eclCaseData && m_extractors[exIdx]->wellPathData() == wellPathGeom)
        {
            return m_extractors[exIdx].p();
        }
    }

    std::string errorIdName = (simWellName + " " + caseUserDescription).toStdString();
    cvf::ref<RigEclipseWellLogExtractor> extractor = new RigEclipseWellLogExtractor(eclCaseData, wellPathGeom, errorIdName);
    m_extractors.push_back(extractor.p());

    return extractor.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RimWellLogPlotCollection::findOrCreateExtractor(RimWellPath* wellPath, RimEclipseCase* eclCase)
{
    if (!(wellPath && eclCase && wellPath->wellPathGeometry() && eclCase->eclipseCaseData()))
    {
        return NULL;
    }

    RigEclipseCaseData* eclCaseData = eclCase->eclipseCaseData();
    RigWellPath* wellPathGeom = wellPath->wellPathGeometry();
    for (size_t exIdx = 0; exIdx < m_extractors.size(); ++exIdx)
    {
         if (m_extractors[exIdx]->caseData() == eclCaseData && m_extractors[exIdx]->wellPathData() == wellPathGeom)
         {
            return m_extractors[exIdx].p();
         }
    }

    std::string errorIdName = (wellPath->name() + " " + eclCase->caseUserDescription()).toStdString();
    cvf::ref<RigEclipseWellLogExtractor> extractor = new RigEclipseWellLogExtractor(eclCaseData, wellPathGeom, errorIdName);
    m_extractors.push_back(extractor.p());

    return extractor.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor* RimWellLogPlotCollection::findOrCreateExtractor(RimWellPath* wellPath, RimGeoMechCase* geomCase)
{
    if (!(wellPath && geomCase && wellPath->wellPathGeometry() && geomCase->geoMechData()))
    {
        return NULL;
    }

    RigGeoMechCaseData* geomCaseData = geomCase->geoMechData();
    RigWellPath* wellPathGeom = wellPath->wellPathGeometry();
    for (size_t exIdx = 0; exIdx < m_geomExtractors.size(); ++exIdx)
    {
         if (m_geomExtractors[exIdx]->caseData() == geomCaseData && m_geomExtractors[exIdx]->wellPathData() == wellPathGeom)
         {
            return m_geomExtractors[exIdx].p();
         }
    }

    std::string errorIdName = (wellPath->name() + " " + geomCase->caseUserDescription()).toStdString();
    cvf::ref<RigGeoMechWellLogExtractor> extractor = new RigGeoMechWellLogExtractor(geomCaseData, wellPathGeom, errorIdName);
    m_geomExtractors.push_back(extractor.p());

    return extractor.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCollection::removeExtractors(const RigWellPath* wellPath)
{
    for (int eIdx = (int) m_extractors.size() - 1; eIdx >= 0; eIdx--)
    {
        if (m_extractors[eIdx]->wellPathData() == wellPath)
        {
            m_extractors.eraseAt(eIdx);
        }
    }

    for (int eIdx = (int) m_geomExtractors.size() - 1; eIdx >= 0; eIdx--)
    {
        if (m_geomExtractors[eIdx]->wellPathData() == wellPath)
        {
            m_geomExtractors.eraseAt(eIdx);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCollection::removeExtractors(const RigEclipseCaseData* caseData)
{
    for (int eIdx = (int) m_extractors.size() - 1; eIdx >= 0; eIdx--)
    {
        if (m_extractors[eIdx]->caseData() == caseData)
        {
            m_extractors.eraseAt(eIdx);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCollection::removeExtractors(const RigGeoMechCaseData* caseData)
{
    for (int eIdx = (int) m_geomExtractors.size() - 1; eIdx >= 0; eIdx--)
    {
        if (m_geomExtractors[eIdx]->caseData() == caseData)
        {
            m_geomExtractors.eraseAt(eIdx);
        }
    }
}
