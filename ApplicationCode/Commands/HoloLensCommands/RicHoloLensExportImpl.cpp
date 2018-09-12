/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RicHoloLensExportImpl.h"

#include "RimGridView.h"
#include "RimSimWellInView.h"
#include "RimWellPath.h"

#include "RivSimWellPipeSourceInfo.h"
#include "RivSourceInfo.h"
#include "RivWellPathSourceInfo.h"

#include "RiuViewer.h"

#include "cvfPart.h"
#include "cvfScene.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensExportImpl::partsForExport(const RimGridView* view, cvf::Collection<cvf::Part>* partCollection)
{
    CVF_ASSERT(partCollection);

    if (!view) return;

    if (view->viewer())
    {
        cvf::Scene* scene = view->viewer()->mainScene();
        if (scene)
        {
            cvf::Collection<cvf::Part> sceneParts;
            scene->allParts(&sceneParts);

            for (auto& scenePart : sceneParts)
            {
                if (RicHoloLensExportImpl::isGrid(scenePart.p()) || RicHoloLensExportImpl::isPipe(scenePart.p()))
                {
                    partCollection->push_back(scenePart.p());
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicHoloLensExportImpl::nameFromPart(const cvf::Part* part)
{
    if (!part) return "";

    QString nameOfObject;

    auto sourceInfo = part->sourceInfo();

    {
        auto gridSourceInfo = dynamic_cast<const RivSourceInfo*>(sourceInfo);
        if (gridSourceInfo)
        {
            size_t gridIndex = gridSourceInfo->gridIndex();

            nameOfObject = QString::number(gridIndex);
        }
    }

    {
        auto simWellSourceInfo = dynamic_cast<const RivSimWellPipeSourceInfo*>(sourceInfo);
        if (simWellSourceInfo)
        {
            RimSimWellInView* simulationWell = simWellSourceInfo->well();
            nameOfObject                     = simulationWell->name();
        }
    }

    {
        auto wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(sourceInfo);
        if (wellPathSourceInfo)
        {
            RimWellPath* wellPath = wellPathSourceInfo->wellPath();
            nameOfObject          = wellPath->name();
        }
    }

    return nameOfObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensExportImpl::isGrid(const cvf::Part* part)
{
    if (!part) return false;

    auto sourceInfo = part->sourceInfo();

    {
        auto gridSourceInfo = dynamic_cast<const RivSourceInfo*>(sourceInfo);
        if (gridSourceInfo)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensExportImpl::isPipe(const cvf::Part* part)
{
    if (!part) return "";

    QString nameOfObject;

    auto sourceInfo = part->sourceInfo();

    {
        auto simWellSourceInfo = dynamic_cast<const RivSimWellPipeSourceInfo*>(sourceInfo);
        if (simWellSourceInfo)
        {
            return true;
        }
    }

    {
        auto wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(sourceInfo);
        if (wellPathSourceInfo)
        {
            return true;
        }
    }

    return false;
}
