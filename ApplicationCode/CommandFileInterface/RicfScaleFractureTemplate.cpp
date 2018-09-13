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

#include "RicfScaleFractureTemplate.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimProject.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"

CAF_PDM_SOURCE_INIT(RicfScaleFractureTemplate, "scaleFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfScaleFractureTemplate::RicfScaleFractureTemplate()
{
    RICF_InitField(&m_id,                       "id",  -1, "Id",  "", "", "");
    RICF_InitField(&m_widthScaleFactor,         "width", 1.0, "WidthScaleFactor",  "", "", "");
    RICF_InitField(&m_heightScaleFactor,        "height", 1.0, "HeightScaleFactor", "", "", "");
    RICF_InitField(&m_dFactorScaleFactor,       "dFactor", 1.0, "DFactorScaleFactor", "", "", "");
    RICF_InitField(&m_conductivityScaleFactor,  "conductivity", 1.0, "ConductivityScaleFactor", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfScaleFractureTemplate::execute()
{
    if (m_id < 0)
    {
        RiaLogging::error("scaleFractureTemplate: Fracture template id not specified");
        return;
    }

    RimProject* project = RiaApplication::instance()->project();

    if (!project)
    {
        RiaLogging::error("scaleFractureTemplate: Project not found");
        return;
    }

    RimFractureTemplateCollection* templColl = !project->allFractureTemplateCollections().empty() ? project->allFractureTemplateCollections()[0] : nullptr;
    RimFractureTemplate* templ = templColl ? templColl->fractureTemplate(m_id) : nullptr;
    
    if (!templ)
    {
        RiaLogging::error(QString("scaleFractureTemplate: Fracture template not found. Id=%1").arg(m_id));
        return;
    }

    templ->setScaleFactors(m_widthScaleFactor, m_heightScaleFactor, m_dFactorScaleFactor, m_conductivityScaleFactor);
    templ->loadDataAndUpdateGeometryHasChanged();
}
