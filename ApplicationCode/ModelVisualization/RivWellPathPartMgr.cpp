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


#include "cvfLibCore.h"

#include "RiaApplication.h"
#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimScriptCollection.h"
#include "RimCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimEclipseView.h"
#include "RigCaseData.h"
#include "RigCell.h"
#include "RivWellPathPartMgr.h"
#include "RivWellPathSourceInfo.h"
#include "RimWellPath.h"
#include "RivPipeGeometryGenerator.h"
#include "cvfModelBasicList.h"
#include "cvfTransform.h"
#include "cvfPart.h"
#include "cvfScalarMapperDiscreteLinear.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfRay.h"
#include "cafEffectGenerator.h"
#include "cvfqtUtils.h"
#include "RimOilField.h"
#include "RimEclipseCaseCollection.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPathPartMgr::RivWellPathPartMgr(RimWellPathCollection* wellPathCollection, RimWellPath* wellPath)
{
    m_wellPathCollection = wellPathCollection;
    m_rimWellPath      = wellPath;

    m_needsTransformUpdate = true;

    // Setup a scalar mapper
    cvf::ref<cvf::ScalarMapperDiscreteLinear> scalarMapper = new cvf::ScalarMapperDiscreteLinear;
    cvf::Color3ubArray legendColors;
    legendColors.resize(4);
    legendColors[0] = cvf::Color3::GRAY;
    legendColors[1] = cvf::Color3::GREEN;
    legendColors[2] = cvf::Color3::BLUE;
    legendColors[3] = cvf::Color3::RED;
    scalarMapper->setColors(legendColors);
    scalarMapper->setRange(0.0 , 4.0);
    scalarMapper->setLevelCount(4, true);

    m_scalarMapper = scalarMapper;

    caf::ScalarMapperEffectGenerator surfEffGen(scalarMapper.p(), caf::PO_1);
    m_scalarMapperSurfaceEffect = surfEffGen.generateCachedEffect();

    caf::ScalarMapperMeshEffectGenerator meshEffGen(scalarMapper.p());
    m_scalarMapperMeshEffect = meshEffGen.generateCachedEffect();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPathPartMgr::~RivWellPathPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// The pipe geometry needs to be rebuilt on scale change to keep the pipes round
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::buildWellPathParts(cvf::Vec3d displayModelOffset, double characteristicCellSize, 
                                            cvf::BoundingBox wellPathClipBoundingBox)
{
    if (m_wellPathCollection.isNull()) return;

    RigWellPath* wellPathGeometry = m_rimWellPath->wellPathGeometry();
    if (!wellPathGeometry) return;

    if (wellPathGeometry->m_wellPathPoints.size() < 2) return;

    m_wellBranches.clear();
    double wellPathRadius = m_wellPathCollection->wellPathRadiusScaleFactor() * m_rimWellPath->wellPathRadiusScaleFactor() * characteristicCellSize;

    cvf::Vec3d textPosition = wellPathGeometry->m_wellPathPoints[0];

    // Generate the well path geometry as a line and pipe structure
    {
        m_wellBranches.push_back(RivPipeBranchData());
        RivPipeBranchData& pbd = m_wellBranches.back();

        pbd.m_pipeGeomGenerator = new RivPipeGeometryGenerator;

        pbd.m_pipeGeomGenerator->setRadius(wellPathRadius);
        pbd.m_pipeGeomGenerator->setCrossSectionVertexCount(m_wellPathCollection->wellPathCrossSectionVertexCount());
        pbd.m_pipeGeomGenerator->setPipeColor( m_rimWellPath->wellPathColor());

        cvf::ref<cvf::Vec3dArray> cvfCoords = new cvf::Vec3dArray;
        if (m_wellPathCollection->wellPathClip)
        {
            std::vector<cvf::Vec3d> clippedPoints;
            for (size_t idx = 0; idx < wellPathGeometry->m_wellPathPoints.size(); idx++)
            {
                cvf::Vec3d point = wellPathGeometry->m_wellPathPoints[idx];
                if (point.z() < (wellPathClipBoundingBox.max().z() + m_wellPathCollection->wellPathClipZDistance))
                    clippedPoints.push_back(point);
            }
            if (clippedPoints.size() < 2) return;

            textPosition = clippedPoints[0];
            cvfCoords->assign(clippedPoints);
        }
        else
        {
            cvfCoords->assign(wellPathGeometry->m_wellPathPoints);
        }
        
        // Scale the centerline coordinates using the Z-scale transform of the grid and correct for the display offset.
        for (size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx)
        {
            cvf::Vec4d transfCoord = m_scaleTransform->worldTransform() * cvf::Vec4d((*cvfCoords)[cIdx] - displayModelOffset, 1);
            (*cvfCoords)[cIdx][0] = transfCoord[0];
            (*cvfCoords)[cIdx][1] = transfCoord[1];
            (*cvfCoords)[cIdx][2] = transfCoord[2];
        }

        pbd.m_pipeGeomGenerator->setPipeCenterCoords(cvfCoords.p());
        pbd.m_surfaceDrawable = pbd.m_pipeGeomGenerator->createPipeSurface();
        pbd.m_centerLineDrawable = pbd.m_pipeGeomGenerator->createCenterLine();

        if (pbd.m_surfaceDrawable.notNull())
        {
            pbd.m_surfacePart = new cvf::Part;
            pbd.m_surfacePart->setDrawable(pbd.m_surfaceDrawable.p());
            
            RivWellPathSourceInfo* sourceInfo = new RivWellPathSourceInfo(m_rimWellPath);
            pbd.m_surfacePart->setSourceInfo(sourceInfo);

            caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(m_rimWellPath->wellPathColor()), caf::PO_1);
            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

            pbd.m_surfacePart->setEffect(eff.p());
        }

        if (pbd.m_centerLineDrawable.notNull())
        {
            pbd.m_centerLinePart = new cvf::Part;
            pbd.m_centerLinePart->setDrawable(pbd.m_centerLineDrawable.p());

            caf::MeshEffectGenerator gen(m_rimWellPath->wellPathColor());
            cvf::ref<cvf::Effect> eff = gen.generateCachedEffect();

            pbd.m_centerLinePart->setEffect(eff.p());
        }
    }

    // Generate label with well-path name

    textPosition -= displayModelOffset;
    textPosition.transformPoint(m_scaleTransform->worldTransform());
    textPosition.z() += characteristicCellSize; // * m_rimReservoirView->wellCollection()->wellHeadScaleFactor();
    textPosition.z() += 1.2 * characteristicCellSize;

    m_wellLabelPart = NULL;
    if (m_wellPathCollection->showWellPathLabel() && m_rimWellPath->showWellPathLabel() && !m_rimWellPath->name().isEmpty())
    {
        cvf::Font* standardFont = RiaApplication::instance()->standardFont();

        cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
        drawableText->setFont(standardFont);
        drawableText->setCheckPosVisible(false);
        drawableText->setDrawBorder(false);
        drawableText->setDrawBackground(false);
        drawableText->setVerticalAlignment(cvf::TextDrawer::CENTER);
        drawableText->setTextColor(m_wellPathCollection->wellPathLabelColor());

        cvf::String cvfString = cvfqt::Utils::toString(m_rimWellPath->name());

        cvf::Vec3f textCoord(textPosition);
        drawableText->addText(cvfString, textCoord);

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("RivWellHeadPartMgr: text " + cvfString);
        part->setDrawable(drawableText.p());

        cvf::ref<cvf::Effect> eff = new cvf::Effect;

        part->setEffect(eff.p());
        part->setPriority(1000);

        m_wellLabelPart = part;
    }

    m_needsTransformUpdate = false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendStaticGeometryPartsToModel(cvf::ModelBasicList* model, cvf::Vec3d displayModelOffset, 
                                                          double characteristicCellSize, cvf::BoundingBox wellPathClipBoundingBox)
{
    if (m_wellPathCollection.isNull()) return;
    if (m_rimWellPath.isNull()) return;

    if (m_wellPathCollection->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF)
        return;

    if (m_wellPathCollection->wellPathVisibility() != RimWellPathCollection::FORCE_ALL_ON && m_rimWellPath->showWellPath() == false )
        return;

    if (m_needsTransformUpdate) 
    {
        // The pipe geometry needs to be rebuilt on scale change to keep the pipes round
        buildWellPathParts(displayModelOffset, characteristicCellSize, wellPathClipBoundingBox);
    }
 
    std::list<RivPipeBranchData>::iterator it;
    for (it = m_wellBranches.begin(); it != m_wellBranches.end(); it++)
    {
        if (it->m_surfacePart.notNull())
        {
            model->addPart(it->m_surfacePart.p());
        }
        if (it->m_centerLinePart.notNull())
        {
            model->addPart(it->m_centerLinePart.p());
        }
    }

    if (m_wellLabelPart.notNull())
    {
        model->addPart(m_wellLabelPart.p());
    }
}

void RivWellPathPartMgr::setScaleTransform( cvf::Transform * scaleTransform )
{
    if (m_scaleTransform.isNull() || m_scaleTransform.p() != scaleTransform) 
    {
        m_scaleTransform = scaleTransform; 
        scheduleGeometryRegen(); 
    }
}
