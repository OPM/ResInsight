#include "RicfSetExportFolder.h"
/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfSetFractureContainment.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimProject.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"

CAF_PDM_SOURCE_INIT(RicfSetFractureContainment, "setFractureContainment");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfSetFractureContainment::RicfSetFractureContainment()
{
    RICF_InitField(&m_id,           "id",  -1, "Id",  "", "", "");
    RICF_InitField(&m_topLayer,     "topLayer", -1, "TopLayer",  "", "", "");
    RICF_InitField(&m_baseLayer,    "baseLayer", -1, "BaseLayer", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfSetFractureContainment::execute()
{
    if (m_id < 0 || m_topLayer < 0 || m_baseLayer < 0)
    {
        RiaLogging::error("setFractureContainment: Required argument missing");
        return;
    }

    RimProject* project = RiaApplication::instance()->project();

    if (!project)
    {
        RiaLogging::error("setFractureContainment: Project not found");
        return;
    }

    RimFractureTemplateCollection* templColl = !project->allFractureTemplateCollections().empty() ? project->allFractureTemplateCollections()[0] : nullptr;
    RimFractureTemplate* templ = templColl ? templColl->fractureTemplate(m_id) : nullptr;
    
    if (!templ)
    {
        RiaLogging::error(QString("setFractureContainment: Fracture template not found. Id=%1").arg(m_id));
        return;
    }

    templ->setContainmentTopKLayer(m_topLayer);
    templ->setContainmentBaseKLayer(m_baseLayer);
    templ->loadDataAndUpdateGeometryHasChanged();
}
