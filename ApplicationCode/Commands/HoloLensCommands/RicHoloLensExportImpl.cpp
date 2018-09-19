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

#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimFaultInView.h"
#include "RimGridView.h"
#include "RimSimWellInView.h"
#include "RimWellPath.h"

#include "RiuViewer.h"
#include "RivSimWellPipeSourceInfo.h"
#include "RivSourceInfo.h"
#include "RivWellPathSourceInfo.h"

#include "cafEffectGenerator.h"

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
std::vector<VdeExportPart> RicHoloLensExportImpl::partsForExport(const RimGridView& view)
{
    std::vector<VdeExportPart> exportParts;

    RimEclipseCase* rimEclipseCase = nullptr;
    view.firstAncestorOrThisOfType(rimEclipseCase);

    if (view.viewer())
    {
        cvf::Scene* scene = view.viewer()->mainScene();
        if (scene)
        {
            cvf::Collection<cvf::Part> sceneParts;
            scene->allParts(&sceneParts);

            for (auto& scenePart : sceneParts)
            {
                if (RicHoloLensExportImpl::isGrid(scenePart.p()))
                {
                    VdeExportPart partForExport(scenePart.p());
                    partForExport.setSourceObjectType(VdeExportPart::OBJ_TYPE_GRID);

                    if (rimEclipseCase && rimEclipseCase->mainGrid())
                    {
                        if (rimEclipseCase->mainGrid()->isFaceNormalsOutwards())
                        {
                            partForExport.setWinding(VdeExportPart::CLOCKWISE);
                        }
                        else
                        {
                            partForExport.setWinding(VdeExportPart::COUNTERCLOCKWISE);
                        }
                    }

                    auto* singleColorEffect = dynamic_cast<caf::SurfaceEffectGenerator*>(scenePart->effect());

                    auto* si = dynamic_cast<RivSourceInfo*>(scenePart->sourceInfo());
                    if (si)
                    {
                        RimFaultInView* faultInView = dynamic_cast<RimFaultInView*>(si->object());
                        if (faultInView && singleColorEffect)
                        {
                            partForExport.setSourceObjectName(faultInView->name());
                            partForExport.setColor(faultInView->faultColor());
                        }

                        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(si->object());
                        if (eclipseCase)
                        {
                            QString nameOfObject   = rimEclipseCase->gridFileName();
                            auto    gridSourceInfo = dynamic_cast<const RivSourceInfo*>(scenePart->sourceInfo());
                            if (gridSourceInfo)
                            {
                                size_t gridIndex = gridSourceInfo->gridIndex();

                                nameOfObject += " Grid " + QString::number(gridIndex);
                            }

                            partForExport.setSourceObjectName(nameOfObject);

                            QString text = RicHoloLensExportImpl::gridCellSetTypeText(si->cellSetType());
                            partForExport.setSourceObjectCellSetType(text);
                        }
                    }

                    exportParts.push_back(partForExport);
                }
                else if (RicHoloLensExportImpl::isPipe(scenePart.p()))
                {
                    VdeExportPart partForExport(scenePart.p());
                    partForExport.setSourceObjectType(VdeExportPart::OBJ_TYPE_PIPE);

                    auto simWellSourceInfo = dynamic_cast<const RivSimWellPipeSourceInfo*>(scenePart->sourceInfo());
                    if (simWellSourceInfo)
                    {
                        auto simWell = simWellSourceInfo->well();
                        if (simWell)
                        {
                            partForExport.setSourceObjectName(simWell->name());
                            partForExport.setColor(simWell->wellPipeColor());
                        }
                    }

                    auto wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(scenePart->sourceInfo());
                    if (wellPathSourceInfo)
                    {
                        RimWellPath* wellPath = wellPathSourceInfo->wellPath();
                        if (wellPath)
                        {
                            partForExport.setSourceObjectName(wellPath->name());
                            partForExport.setColor(wellPath->wellPathColor());
                        }
                    }

                    exportParts.push_back(partForExport);
                }
            }
        }
    }

    return exportParts;
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

            nameOfObject = "Grid " + QString::number(gridIndex);
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
QString RicHoloLensExportImpl::gridCellSetTypeText(RivCellSetEnum cellSetType)
{
    switch (cellSetType)
    {
        case OVERRIDDEN_CELL_VISIBILITY:
            return "OVERRIDDEN_CELL_VISIBILITY";
            break;
        case ALL_CELLS:
            return "ALL_CELLS";
            break;
        case ACTIVE:
            return "ACTIVE";
            break;
        case ALL_WELL_CELLS:
            return "ALL_WELL_CELLS";
            break;
        case VISIBLE_WELL_CELLS:
            return "VISIBLE_WELL_CELLS";
            break;
        case VISIBLE_WELL_FENCE_CELLS:
            return "VISIBLE_WELL_FENCE_CELLS";
            break;
        case INACTIVE:
            return "INACTIVE";
            break;
        case RANGE_FILTERED:
            return "RANGE_FILTERED";
            break;
        case RANGE_FILTERED_INACTIVE:
            return "RANGE_FILTERED_INACTIVE";
            break;
        case RANGE_FILTERED_WELL_CELLS:
            return "RANGE_FILTERED_WELL_CELLS";
            break;
        case VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER:
            return "VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER";
            break;
        case VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER:
            return "VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER";
            break;
        case PROPERTY_FILTERED:
            return "PROPERTY_FILTERED";
            break;
        case PROPERTY_FILTERED_WELL_CELLS:
            return "PROPERTY_FILTERED_WELL_CELLS";
            break;
        default:
            break;
    }

    return "INVALID_CELL_SET_TYPE";
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
