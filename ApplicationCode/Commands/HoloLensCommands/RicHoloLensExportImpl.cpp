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

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimGridView.h"
#include "RimSimWellInView.h"
#include "RimWellPath.h"

#include "RiuViewer.h"
#include "RivFemPickSourceInfo.h"
#include "RivIntersectionBoxSourceInfo.h"
#include "RivIntersectionSourceInfo.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivSimWellPipeSourceInfo.h"
#include "RivSourceInfo.h"
#include "RivWellPathSourceInfo.h"

#include "cafEffectGenerator.h"

#include "cvfPart.h"
#include "cvfRenderState.h"
#include "cvfRenderStateTextureBindings.h"
#include "cvfRenderState_FF.h"
#include "cvfTexture.h"
#include "cvfTexture2D_FF.h"

#include <QString>

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
        auto visibleParts = view.viewer()->visibleParts();

        for (auto& visiblePart : visibleParts)
        {
            if (RicHoloLensExportImpl::isGrid(visiblePart.p()))
            {
                VdeExportPart exportPart(visiblePart.p());
                exportPart.setSourceObjectType(VdeExportPart::OBJ_TYPE_GRID);

                if (rimEclipseCase && rimEclipseCase->mainGrid())
                {
                    if (rimEclipseCase->mainGrid()->isFaceNormalsOutwards())
                    {
                        exportPart.setWinding(VdeExportPart::COUNTERCLOCKWISE);
                    }
                    else
                    {
                        exportPart.setWinding(VdeExportPart::CLOCKWISE);
                    }
                }

                auto* si = dynamic_cast<RivSourceInfo*>(visiblePart->sourceInfo());
                if (si)
                {
                    RimFaultInView* faultInView = dynamic_cast<RimFaultInView*>(si->object());
                    if (faultInView)
                    {
                        exportPart.setSourceObjectName(faultInView->name());
                        exportPart.setColor(faultInView->faultColor());
                    }

                    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(si->object());
                    if (eclipseCase)
                    {
                        QString nameOfObject   = rimEclipseCase->gridFileName();
                        auto    gridSourceInfo = dynamic_cast<const RivSourceInfo*>(visiblePart->sourceInfo());
                        if (gridSourceInfo)
                        {
                            size_t gridIndex = gridSourceInfo->gridIndex();

                            nameOfObject += " Grid " + QString::number(gridIndex);
                        }

                        const RimEclipseView* eclipseView = dynamic_cast<const RimEclipseView*>(&view);
                        if (eclipseView)
                        {
                            cvf::Color4f color = eclipseView->colorFromCellCategory(si->cellSetType());
                            exportPart.setColor(color.toColor3f());
                            exportPart.setOpacity(color.a());

                            QString text = RicHoloLensExportImpl::gridCellSetTypeText(si->cellSetType());
                            exportPart.setSourceObjectCellSetType(text);
                        }

                        exportPart.setSourceObjectName(nameOfObject);
                    }
                }

                appendTextureImage(exportPart, visiblePart.p());

                exportParts.push_back(exportPart);
            }
            else if (RicHoloLensExportImpl::isPipe(visiblePart.p()))
            {
                VdeExportPart exportPart(visiblePart.p());
                exportPart.setSourceObjectType(VdeExportPart::OBJ_TYPE_PIPE);

                auto simWellSourceInfo = dynamic_cast<const RivSimWellPipeSourceInfo*>(visiblePart->sourceInfo());
                if (simWellSourceInfo)
                {
                    auto simWell = simWellSourceInfo->well();
                    if (simWell)
                    {
                        exportPart.setSourceObjectName(simWell->name());
                        exportPart.setColor(simWell->wellPipeColor());
                    }
                }

                auto wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(visiblePart->sourceInfo());
                if (wellPathSourceInfo)
                {
                    RimWellPath* wellPath = wellPathSourceInfo->wellPath();
                    if (wellPath)
                    {
                        exportPart.setSourceObjectName(wellPath->name());
                        exportPart.setColor(wellPath->wellPathColor());
                    }
                }

                appendTextureImage(exportPart, visiblePart.p());

                exportParts.push_back(exportPart);
            }
            else if (RicHoloLensExportImpl::isMeshLines(visiblePart.p()))
            {
                VdeExportPart exportPart(visiblePart.p());
                exportPart.setSourceObjectType(VdeExportPart::OBJ_TYPE_GRID);

                cvf::Color3f lineColor = RiaApplication::instance()->preferences()->defaultGridLineColors();

                auto linesSourceInfo = dynamic_cast<const RivMeshLinesSourceInfo*>(visiblePart->sourceInfo());
                if (linesSourceInfo)
                {
                    if (dynamic_cast<RimFaultInView*>(linesSourceInfo->object()))
                    {
                        lineColor = RiaApplication::instance()->preferences()->defaultFaultGridLineColors();
                    }
                }

                exportPart.setColor(lineColor);
                exportPart.setRole(VdeExportPart::MESH_LINES);

                appendTextureImage(exportPart, visiblePart.p());

                exportParts.push_back(exportPart);
            }
        }
    }

    return exportParts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensExportImpl::appendTextureImage(VdeExportPart& exportPart, cvf::Part* part)
{
    if (part && part->effect())
    {
        {
            auto textureBindings = dynamic_cast<cvf::RenderStateTextureBindings*>(
                part->effect()->renderStateOfType(cvf::RenderState::TEXTURE_BINDINGS));

            if (textureBindings && textureBindings->bindingCount() > 0)
            {
                cvf::Texture* textureBinding = textureBindings->texture(0);
                if (textureBinding)
                {
                    exportPart.setTextureImage(textureBinding->image());
                }
            }
        }

        {
            auto textureMappingFF = dynamic_cast<cvf::RenderStateTextureMapping_FF*>(
                part->effect()->renderStateOfType(cvf::RenderState::TEXTURE_MAPPING_FF));

            if (textureMappingFF && textureMappingFF->texture())
            {
                auto* texture = textureMappingFF->texture();
                if (texture)
                {
                    exportPart.setTextureImage(texture->image());
                }
            }
        }
    }
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
        {
            auto sourceInfoOfType = dynamic_cast<const RivSourceInfo*>(sourceInfo);
            if (sourceInfoOfType)
            {
                return true;
            }
        }

        {
            auto sourceInfoOfType = dynamic_cast<const RivIntersectionSourceInfo*>(sourceInfo);
            if (sourceInfoOfType)
            {
                return true;
            }
        }

        {
            auto sourceInfoOfType = dynamic_cast<const RivIntersectionBoxSourceInfo*>(sourceInfo);
            if (sourceInfoOfType)
            {
                return true;
            }
        }

        {
            auto sourceInfoOfType = dynamic_cast<const RivFemPickSourceInfo*>(sourceInfo);
            if (sourceInfoOfType)
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensExportImpl::isPipe(const cvf::Part* part)
{
    if (!part) return false;

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensExportImpl::isMeshLines(const cvf::Part* part)
{
    if (!part) return false;

    auto sourceInfo = part->sourceInfo();

    {
        auto linesSourceInfo = dynamic_cast<const RivMeshLinesSourceInfo*>(sourceInfo);
        if (linesSourceInfo)
        {
            return true;
        }
    }

    return false;
}
