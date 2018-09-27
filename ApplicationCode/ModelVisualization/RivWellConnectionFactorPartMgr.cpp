/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RivWellConnectionFactorPartMgr.h"

#include "RiaApplication.h"
#include "RiaExtractionTools.h"

#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellLogExtractor.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInViewCollection.h"
#include "RimVirtualPerforationResults.h"
#include "RimWellPath.h"

#include "RiuViewer.h"

#include "RivWellConnectionFactorGeometryGenerator.h"
#include "RivWellConnectionSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellConnectionFactorPartMgr::RivWellConnectionFactorPartMgr(RimWellPath*                  well,
                                                               RimVirtualPerforationResults* virtualPerforationResult)
    : m_rimWellPath(well)
    , m_virtualPerforationResult(virtualPerforationResult)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellConnectionFactorPartMgr::~RivWellConnectionFactorPartMgr() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellConnectionFactorPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    m_geometryGenerator = nullptr;

    RimEclipseView* eclView = nullptr;
    m_virtualPerforationResult->firstAncestorOrThisOfTypeAsserted(eclView);

    auto coordTransform = eclView->displayCoordTransform();

    RimEclipseCase* eclipseCase = nullptr;
    m_virtualPerforationResult->firstAncestorOrThisOfTypeAsserted(eclipseCase);

    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    const RigVirtualPerforationTransmissibilities* trans = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    if (!trans) return;

    auto completionsForWellPath = trans->multipleCompletionsPerEclipseCell(m_rimWellPath, frameIndex);

    // Remove connection factors for parent grid, they are not supposed to be visualized, but are relevant for export
    for (auto it = completionsForWellPath.begin(); it != completionsForWellPath.end();)
    {
        size_t gridIndex = it->first.globalCellIndex();

        const RigCell& rigCell = mainGrid->cell(gridIndex);
        if (rigCell.subGrid())
        {
            it = completionsForWellPath.erase(it);
        }
        else
        {
            ++it;
        }
    }

    std::vector<WellPathCellIntersectionInfo> wellPathCellIntersections;
    {
        RigEclipseWellLogExtractor* extractor = RiaExtractionTools::wellLogExtractorEclipseCase(m_rimWellPath, eclipseCase);
        if (extractor)
        {
            wellPathCellIntersections = extractor->cellIntersectionInfosAlongWellPath();
        }
    }

    std::vector<CompletionVizData> completionVizDataItems;
    for (const auto& completionsForCell : completionsForWellPath)
    {
        if (!m_virtualPerforationResult->showConnectionFactorsOnClosedConnections())
        {
            for (const auto& completion : completionsForCell.second)
            {
                if (completion.connectionState() == SHUT)
                {
                    continue;
                }
            }
        }

        size_t gridIndex = completionsForCell.first.globalCellIndex();

        const RigCell& rigCell = mainGrid->cell(gridIndex);

        cvf::Vec3d locationInDomainCoord = rigCell.center();
        cvf::Vec3d direction             = cvf::Vec3d::X_AXIS;
        bool       foundLocation         = false;

        {
            size_t i = 0;
            while (!foundLocation && (i < wellPathCellIntersections.size()))
            {
                const WellPathCellIntersectionInfo& intersectionInfo = wellPathCellIntersections[i];

                if (intersectionInfo.globCellIndex == completionsForCell.first.globalCellIndex())
                {
                    double startMD = intersectionInfo.startMD;
                    double endMD   = intersectionInfo.endMD;

                    double middleMD = (startMD + endMD) / 2.0;

                    locationInDomainCoord = m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath(middleMD);

                    cvf::Vec3d p1;
                    cvf::Vec3d p2;
                    m_rimWellPath->wellPathGeometry()->twoClosestPoints(locationInDomainCoord, &p1, &p2);

                    direction = (p2 - p1).getNormalized();

                    foundLocation = true;
                }

                i++;
            }
        }

        cvf::Vec3d displayCoord = coordTransform->transformToDisplayCoord(locationInDomainCoord);

        for (size_t i = 0; i < completionsForCell.second.size(); i++)
        {
            const RigCompletionData& completionData = completionsForCell.second[i];

            double transmissibility = completionData.transmissibility();

            completionVizDataItems.push_back(
                CompletionVizData(displayCoord, direction, transmissibility, completionsForCell.first.globalCellIndex()));
        }
    }

    if (!completionVizDataItems.empty())
    {
        double characteristicCellSize = eclView->ownerCase()->characteristicCellSize();

        double radius = m_rimWellPath->wellPathRadius(characteristicCellSize) * m_virtualPerforationResult->geometryScaleFactor();
        radius *= 2.0; // Enlarge the radius slightly to make the connection factor visible if geometry scale factor is set to 1.0

        m_geometryGenerator = new RivWellConnectionFactorGeometryGenerator(completionVizDataItems, radius);

        auto scalarMapper = m_virtualPerforationResult->legendConfig()->scalarMapper();

        cvf::ref<cvf::Part> part = m_geometryGenerator->createSurfacePart(scalarMapper, eclView->isLightingDisabled());
        if (part.notNull())
        {
            cvf::ref<RivWellConnectionSourceInfo> sourceInfo =
                new RivWellConnectionSourceInfo(m_rimWellPath, m_geometryGenerator.p());
            part->setSourceInfo(sourceInfo.p());

            model->addPart(part.p());
        }
    }
}
