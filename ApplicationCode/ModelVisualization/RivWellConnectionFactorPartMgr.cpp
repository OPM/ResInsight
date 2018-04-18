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
    : m_rimWell(well)
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

    auto conn = trans->multipleCompletionsPerEclipseCell(m_rimWell, frameIndex);

    std::vector<WellPathCellIntersectionInfo> wellPathCellIntersections;
    {
        RigEclipseWellLogExtractor* extractor = RiaExtractionTools::wellLogExtractorEclipseCase(m_rimWell, eclipseCase);
        if (extractor)
        {
            wellPathCellIntersections = extractor->cellIntersectionInfosAlongWellPath();
        }
    }

    std::vector<CompletionVizData> completionVizDataItems;
    for (const auto& cell : conn)
    {
        size_t gridIndex = cell.first.globalCellIndex();

        const RigCell& rigCell = mainGrid->cell(gridIndex);

        std::vector<std::pair<cvf::Vec3d, cvf::Vec3d>> locationAndDirection;

        for (const auto& intersectionInfo : wellPathCellIntersections)
        {
            if (intersectionInfo.globCellIndex == cell.first.globalCellIndex())
            {
                double startMD = intersectionInfo.startMD;
                double endMD   = intersectionInfo.endMD;

                double middleMD = (startMD + endMD) / 2.0;

                cvf::Vec3d defaultLocationInDomainCoord = m_rimWell->wellPathGeometry()->interpolatedPointAlongWellPath(middleMD);

                cvf::Vec3d p1;
                cvf::Vec3d p2;
                m_rimWell->wellPathGeometry()->twoClosestPoints(defaultLocationInDomainCoord, &p1, &p2);

                cvf::Vec3d defaultWellPathDirection = (p2 - p1).getNormalized();

                locationAndDirection.push_back(std::make_pair(defaultLocationInDomainCoord, defaultWellPathDirection));
            }
            else if (!locationAndDirection.empty())
            {
                continue;
            }
        }

        for (size_t i = 0; i < cell.second.size(); i++)
        {
            const RigCompletionData& completionData = cell.second[i];

            double transmissibility = completionData.transmissibility();

            cvf::Vec3d locationInDomainCoord = rigCell.center();
            cvf::Vec3d wellPathDirection     = cvf::Vec3d::X_AXIS;

            if (i < locationAndDirection.size())
            {
                locationInDomainCoord = locationAndDirection[i].first;
                wellPathDirection     = locationAndDirection[i].second;
            }

            cvf::Vec3d displayCoord = coordTransform->transformToDisplayCoord(locationInDomainCoord);

            completionVizDataItems.push_back(
                CompletionVizData(displayCoord, wellPathDirection, transmissibility, cell.first.globalCellIndex()));
        }
    }

    if (!completionVizDataItems.empty())
    {
        double characteristicCellSize = eclView->ownerCase()->characteristicCellSize();

        double radius = m_rimWell->wellPathRadius(characteristicCellSize) * m_virtualPerforationResult->geometryScaleFactor();
        radius *= 2.0; // Enlarge the radius slightly to make the connection factor visible if geometry scale factor is set to 1.0

        m_geometryGenerator = new RivWellConnectionFactorGeometryGenerator(completionVizDataItems, radius);

        auto scalarMapper = m_virtualPerforationResult->legendConfig()->scalarMapper();
        cvf::ref<cvf::Part> part = m_geometryGenerator->createSurfacePart(scalarMapper, eclView->isLightingDisabled());
        if (part.notNull())
        {
            cvf::ref<RivWellConnectionSourceInfo> sourceInfo = new RivWellConnectionSourceInfo(m_rimWell, m_geometryGenerator.p());
            part->setSourceInfo(sourceInfo.p());

            model->addPart(part.p());
        }
    }
}
