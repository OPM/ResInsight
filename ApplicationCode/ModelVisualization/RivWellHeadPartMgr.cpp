/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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



#include "RivWellHeadPartMgr.h"

#include "RiaApplication.h"

#include "RigCaseData.h"
#include "RigCell.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimLegendConfig.h"
#include "RimReservoirCellResultsStorage.h"

#include "RivPipeGeometryGenerator.h"
#include "RivWellPipesPartMgr.h"

#include "cafEffectGenerator.h"
#include "cafPdmFieldCvfMat4d.h"

#include "cvfArrowGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfFixedAtlasFont.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfLibCore.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cvfTransform.h"
#include "cvfqtUtils.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellHeadPartMgr::RivWellHeadPartMgr(RimEclipseView* reservoirView, RimEclipseWell* well)
{
    m_rimReservoirView = reservoirView;
    m_rimWell = well;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellHeadPartMgr::~RivWellHeadPartMgr()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellHeadPartMgr::buildWellHeadParts(size_t frameIndex)
{
    m_wellHeadParts.clear();

    if (m_rimReservoirView.isNull()) return;

    RigCaseData* rigReservoir = m_rimReservoirView->eclipseCase()->reservoirData();

    RimEclipseWell* well = m_rimWell;

    RigSingleWellResultsData* wellResults = well->wellResults();

    if (wellResults->m_staticWellCells.m_wellResultBranches.size() == 0)
    {
        wellResults->computeStaticWellCellPath();
    }
    if (wellResults->m_staticWellCells.m_wellResultBranches.size() == 0) return;

    if (!wellResults->hasWellResult(frameIndex)) return;

    const RigWellResultFrame& wellResultFrame = wellResults->wellResultFrame(frameIndex);

    const RigCell& whCell = rigReservoir->cellFromWellResultCell(wellResultFrame.m_wellHead);

    double characteristicCellSize = rigReservoir->mainGrid()->characteristicIJCellSize();

    // Match this position with pipe start position in RivWellPipesPartMgr::calculateWellPipeCenterline()
    cvf::Vec3d whStartPos = whCell.faceCenter(cvf::StructGridInterface::NEG_K);
    whStartPos -= rigReservoir->mainGrid()->displayModelOffset();
    whStartPos.transformPoint(m_scaleTransform->worldTransform());

    // Compute well head based on the z position of the top of the K column the well head is part of
    cvf::Vec3d whEndPos = whStartPos;

    if (m_rimReservoirView->wellCollection()->wellHeadPosition() == RimEclipseWellCollection::WELLHEAD_POS_TOP_COLUMN)
    {
        // Position well head at top active cell of IJ-column

        size_t i, j, k;
        rigReservoir->mainGrid()->ijkFromCellIndex(whCell.mainGridCellIndex(), &i, &j, &k);

        size_t kIndexWellHeadCell = k;
        k = 0;
        
        size_t topActiveCellIndex = rigReservoir->mainGrid()->cellIndexFromIJK(i, j, k);
        while(k < kIndexWellHeadCell && !m_rimReservoirView->currentActiveCellInfo()->isActive(topActiveCellIndex))
        {
            k++;
            topActiveCellIndex = rigReservoir->mainGrid()->cellIndexFromIJK(i, j, k);
        }
        
        const RigCell& topActiveCell = rigReservoir->mainGrid()->cell(topActiveCellIndex);
        cvf::Vec3d topCellPos = topActiveCell.faceCenter(cvf::StructGridInterface::NEG_K);
        topCellPos -= rigReservoir->mainGrid()->displayModelOffset();
        topCellPos.transformPoint(m_scaleTransform->worldTransform());

        // Modify position if top active cell is closer to sea than well head
        if (kIndexWellHeadCell > k)
        {
            whEndPos.z() = topCellPos.z() + characteristicCellSize;
        }
    }
    else
    {
        // Position well head at top of active cells bounding box

        cvf::Vec3d activeCellsBoundingBoxMax = m_rimReservoirView->currentActiveCellInfo()->geometryBoundingBox().max();
        activeCellsBoundingBoxMax -= rigReservoir->mainGrid()->displayModelOffset();
        activeCellsBoundingBoxMax.transformPoint(m_scaleTransform->worldTransform());

        whEndPos.z() = activeCellsBoundingBoxMax.z();
    }

    cvf::Vec3d arrowPosition = whEndPos;
    arrowPosition.z() += 2.0;

    // Well head pipe geometry
    {
        cvf::ref<cvf::Vec3dArray> wellHeadPipeCoords = new cvf::Vec3dArray;
        wellHeadPipeCoords->resize(2);
        wellHeadPipeCoords->set(0, whStartPos);
        wellHeadPipeCoords->set(1, whEndPos);

        cvf::ref<RivPipeGeometryGenerator> pipeGeomGenerator = new RivPipeGeometryGenerator;
        pipeGeomGenerator->setPipeCenterCoords(wellHeadPipeCoords.p());
        pipeGeomGenerator->setPipeColor(well->wellPipeColor());
        pipeGeomGenerator->setCrossSectionVertexCount(m_rimReservoirView->wellCollection()->pipeCrossSectionVertexCount());

        double pipeRadius = m_rimReservoirView->wellCollection()->pipeRadiusScaleFactor() * m_rimWell->pipeRadiusScaleFactor() * characteristicCellSize;
        pipeGeomGenerator->setRadius(pipeRadius);

        cvf::ref<cvf::DrawableGeo> pipeSurface = pipeGeomGenerator->createPipeSurface();
        cvf::ref<cvf::DrawableGeo> centerLineDrawable = pipeGeomGenerator->createCenterLine();

        if (pipeSurface.notNull())
        {
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("RivWellHeadPartMgr: surface " + cvfqt::Utils::toString(well->name()));
            part->setDrawable(pipeSurface.p());

            caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(well->wellPipeColor()), caf::PO_1);
            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

            part->setEffect(eff.p());

            m_wellHeadParts.push_back(part.p());
        }

        if (centerLineDrawable.notNull())
        {
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("RivWellHeadPartMgr: centerline " + cvfqt::Utils::toString(well->name()));
            part->setDrawable(centerLineDrawable.p());

            caf::MeshEffectGenerator meshGen(well->wellPipeColor());
            cvf::ref<cvf::Effect> eff = meshGen.generateCachedEffect();

            part->setEffect(eff.p());

            m_wellHeadParts.push_back(part.p());
        }
    }

    double arrowLength = characteristicCellSize * m_rimReservoirView->wellCollection()->wellHeadScaleFactor();
    cvf::Vec3d textPosition = arrowPosition;
    textPosition.z() += 1.2 * arrowLength;
    
    cvf::Mat4f matr;
    if (wellResultFrame.m_productionType != RigWellResultFrame::PRODUCER)
    {
        matr = cvf::Mat4f::fromRotation(cvf::Vec3f(1.0f, 0.0f, 0.0f), cvf::Math::toRadians(180.0f));
    }

    double ijScaleFactor = arrowLength / 6;
    matr(0, 0) *= ijScaleFactor;
    matr(1, 1) *= ijScaleFactor;
    matr(2, 2) *= arrowLength;

    if (wellResultFrame.m_productionType != RigWellResultFrame::PRODUCER)
    {
        arrowPosition.z() += arrowLength;
    }

    matr.setTranslation(cvf::Vec3f(arrowPosition));

    cvf::GeometryBuilderFaceList builder;
    cvf::ArrowGenerator gen;
    gen.setShaftRelativeRadius(0.5f);
    gen.setHeadRelativeRadius(1.0f);
    gen.setHeadRelativeLength(0.4f);
    gen.setNumSlices(m_rimReservoirView->wellCollection()->pipeCrossSectionVertexCount());
    gen.generate(&builder);

    cvf::ref<cvf::Vec3fArray> vertices = builder.vertices();
    cvf::ref<cvf::UIntArray> faceList = builder.faceList();

    size_t i;
    for (i = 0; i < vertices->size(); i++)
    {
        cvf::Vec3f v = vertices->get(i);
        v.transformPoint(matr);
        vertices->set(i, v);
    }

    cvf::ref<cvf::DrawableGeo> geo1 = new cvf::DrawableGeo;
    geo1->setVertexArray(vertices.p());   
    geo1->setFromFaceList(*faceList);
    geo1->computeNormals();


    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("RivWellHeadPartMgr: arrow " + cvfqt::Utils::toString(well->name()));
        part->setDrawable(geo1.p());

        cvf::Color4f headColor(cvf::Color3::GRAY);
        if (wellResultFrame.m_isOpen)
        {
            if (wellResultFrame.m_productionType == RigWellResultFrame::PRODUCER)
            {
                headColor = cvf::Color4f(cvf::Color3::GREEN);
            }
            else if (wellResultFrame.m_productionType == RigWellResultFrame::OIL_INJECTOR)
            {
                headColor = cvf::Color4f(cvf::Color3::ORANGE);
            }
            else if (wellResultFrame.m_productionType == RigWellResultFrame::GAS_INJECTOR)
            {
                headColor = cvf::Color4f(cvf::Color3::RED);
            }
            else if (wellResultFrame.m_productionType == RigWellResultFrame::WATER_INJECTOR)
            {
                headColor = cvf::Color4f(cvf::Color3::BLUE);
            }
        }

        caf::SurfaceEffectGenerator surfaceGen(headColor, caf::PO_1);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

        part->setEffect(eff.p());
        m_wellHeadParts.push_back(part.p());
    }

    if (m_rimReservoirView->wellCollection()->showWellLabel() && well->showWellLabel() && !well->name().isEmpty())
    {
        cvf::Font* standardFont = RiaApplication::instance()->standardFont();

        cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
        drawableText->setFont(standardFont);
        drawableText->setCheckPosVisible(false);
        drawableText->setDrawBorder(false);
        drawableText->setDrawBackground(false);
        drawableText->setVerticalAlignment(cvf::TextDrawer::CENTER);
        drawableText->setTextColor(m_rimReservoirView->wellCollection()->wellLabelColor());

        cvf::String cvfString = cvfqt::Utils::toString(well->name());

        cvf::Vec3f textCoord(textPosition);
        drawableText->addText(cvfString, textCoord);

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("RivWellHeadPartMgr: text " + cvfString);
        part->setDrawable(drawableText.p());

        cvf::ref<cvf::Effect> eff = new cvf::Effect;

        part->setEffect(eff.p());
        part->setPriority(11);

        m_wellHeadParts.push_back(part.p());
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellHeadPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    if (m_rimWell.isNull()) return;
    if (m_rimReservoirView.isNull()) return;
    if (m_rimReservoirView->wellCollection()->showWellHead() == false) return;
    if (!m_rimWell->isWellPipeVisible(frameIndex)) return;

    buildWellHeadParts(frameIndex);

    size_t i;
    for (i = 0; i < m_wellHeadParts.size(); i++)
    {
        model->addPart(m_wellHeadParts.at(i));
    }
}

