/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RivFaultPartMgr.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RigCaseCellResultsData.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFault.h"
#include "RimFaultCollection.h"
#include "RimLegendConfig.h"
#include "RimTernaryLegendConfig.h"

#include "RivFaultGeometryGenerator.h"
#include "RivNNCGeometryGenerator.h"
#include "RivPartPriority.h"
#include "RivResultToTextureMapper.h"
#include "RivScalarMapperUtils.h"
#include "RivSourceInfo.h"
#include "RivTernaryScalarMapper.h"
#include "RivTernaryTextureCoordsCreator.h"
#include "RivTextureCoordsCreator.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfqtUtils.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFaultPartMgr::RivFaultPartMgr(const RigGridBase* grid, const RimFaultCollection* rimFaultCollection, const RimFault* rimFault)
    :   m_grid(grid),
        m_rimFaultCollection(rimFaultCollection),
        m_rimFault(rimFault),
        m_opacityLevel(1.0f),
        m_defaultColor(cvf::Color3::WHITE)
{
    cvf::ref< cvf::Array<size_t> > connIdxes = new cvf::Array<size_t>;
    connIdxes->assign(rimFault->faultGeometry()->connectionIndices());

    m_nativeFaultGenerator = new RivFaultGeometryGenerator(grid, rimFault->faultGeometry(), true);
    m_oppositeFaultGenerator = new  RivFaultGeometryGenerator(grid, rimFault->faultGeometry(), false);

    m_NNCGenerator = new RivNNCGeometryGenerator(grid->mainGrid()->nncData(), grid->mainGrid()->displayModelOffset(), connIdxes.p());

    m_nativeFaultFacesTextureCoords = new cvf::Vec2fArray;
    m_oppositeFaultFacesTextureCoords = new cvf::Vec2fArray;
    m_NNCTextureCoords = new cvf::Vec2fArray;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::setCellVisibility(cvf::UByteArray* cellVisibilities)
{
    m_nativeFaultGenerator->setCellVisibility(cellVisibilities);
    m_oppositeFaultGenerator->setCellVisibility(cellVisibilities);
    m_NNCGenerator->setCellVisibility(cellVisibilities, m_grid.p());

    generatePartGeometry();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::applySingleColorEffect()
{
    m_defaultColor = m_rimFault->faultColor();
    this->updatePartEffect();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::updateCellResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors)
{
    CVF_ASSERT(cellResultColors);

    updateNNCColors(cellResultColors);

    RimEclipseView* eclipseView = cellResultColors->reservoirView();
    RigEclipseCaseData* eclipseCase = eclipseView->eclipseCase()->eclipseCaseData();

    // Faults
    if (m_nativeFaultFaces.notNull())
    {
        if (cellResultColors->isTernarySaturationSelected())
        {
            RivTernaryTextureCoordsCreator texturer(cellResultColors, cellResultColors->ternaryLegendConfig(),
                timeStepIndex,
                m_grid->gridIndex(),
                m_nativeFaultGenerator->quadToCellFaceMapper());

            texturer.createTextureCoords(m_nativeFaultFacesTextureCoords.p());

            const RivTernaryScalarMapper* mapper = cellResultColors->ternaryLegendConfig()->scalarMapper();
            RivScalarMapperUtils::applyTernaryTextureResultsToPart(m_nativeFaultFaces.p(), m_nativeFaultFacesTextureCoords.p(), mapper, m_opacityLevel, this->faceCullingMode(), eclipseView->isLightingDisabled());
        }
        else
        {
            RivTextureCoordsCreator texturer(cellResultColors, 
                timeStepIndex, 
                m_grid->gridIndex(),  
                m_nativeFaultGenerator->quadToCellFaceMapper());

            if (!texturer.isValid())
            {
                return;
            }

            texturer.createTextureCoords(m_nativeFaultFacesTextureCoords.p());

            const cvf::ScalarMapper* mapper = cellResultColors->legendConfig()->scalarMapper();
            RivScalarMapperUtils::applyTextureResultsToPart(m_nativeFaultFaces.p(), m_nativeFaultFacesTextureCoords.p(), mapper, m_opacityLevel, this->faceCullingMode(), eclipseView->isLightingDisabled());
        }
    }

    if (m_oppositeFaultFaces.notNull())
    {
        if (cellResultColors->isTernarySaturationSelected())
        {
            RivTernaryTextureCoordsCreator texturer(cellResultColors, cellResultColors->ternaryLegendConfig(),
                timeStepIndex,
                m_grid->gridIndex(),
                m_oppositeFaultGenerator->quadToCellFaceMapper());

            texturer.createTextureCoords(m_oppositeFaultFacesTextureCoords.p());

            const RivTernaryScalarMapper* mapper = cellResultColors->ternaryLegendConfig()->scalarMapper();
            RivScalarMapperUtils::applyTernaryTextureResultsToPart(m_oppositeFaultFaces.p(), m_oppositeFaultFacesTextureCoords.p(), mapper, m_opacityLevel, this->faceCullingMode(), eclipseView->isLightingDisabled());
        }
        else
        {
            RivTextureCoordsCreator texturer(cellResultColors,
                timeStepIndex,
                m_grid->gridIndex(),
                m_oppositeFaultGenerator->quadToCellFaceMapper());

            if (!texturer.isValid())
            {
                return;
            }

            texturer.createTextureCoords(m_oppositeFaultFacesTextureCoords.p());

            const cvf::ScalarMapper* mapper = cellResultColors->legendConfig()->scalarMapper();
            RivScalarMapperUtils::applyTextureResultsToPart(m_oppositeFaultFaces.p(), m_oppositeFaultFacesTextureCoords.p(), mapper, m_opacityLevel, this->faceCullingMode(), eclipseView->isLightingDisabled());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::updateCellEdgeResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors, RimCellEdgeColors* cellEdgeResultColors)
{
    updateNNCColors(cellResultColors);

    if (m_nativeFaultFaces.notNull())
    {
        cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(m_nativeFaultFaces->drawable());
        if (dg)
        {
            cvf::ref<cvf::Effect> eff = RivScalarMapperUtils::createCellEdgeEffect(dg, m_nativeFaultGenerator->quadToCellFaceMapper(),
                m_grid->gridIndex(),
                timeStepIndex, cellResultColors, cellEdgeResultColors, m_opacityLevel, m_defaultColor, this->faceCullingMode(), cellResultColors->reservoirView()->isLightingDisabled());

            m_nativeFaultFaces->setEffect(eff.p());
        }
    }

    if (m_oppositeFaultFaces.notNull())
    {
        cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(m_oppositeFaultFaces->drawable());
        if (dg)
        {
            cvf::ref<cvf::Effect> eff = RivScalarMapperUtils::createCellEdgeEffect(dg, m_oppositeFaultGenerator->quadToCellFaceMapper(), m_grid->gridIndex(),
                timeStepIndex, cellResultColors, cellEdgeResultColors, m_opacityLevel, m_defaultColor, this->faceCullingMode(), cellResultColors->reservoirView()->isLightingDisabled());

            m_oppositeFaultFaces->setEffect(eff.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::generatePartGeometry()
{

    bool useBufferObjects = true;
    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = m_nativeFaultGenerator->generateSurface();
        if (geo.notNull())
        {
            geo->computeNormals();

            if (useBufferObjects)
            {
                geo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid " + cvf::String(static_cast<int>(m_grid->gridIndex())));
            part->setDrawable(geo.p());

            // Set mapping from triangle face index to cell index
            cvf::ref<RivSourceInfo> si = new RivSourceInfo(m_grid->gridIndex());
            si->m_cellFaceFromTriangleMapper = m_nativeFaultGenerator->triangleToCellFaceMapper();
            part->setSourceInfo(si.p());

            part->updateBoundingBox();
            part->setEnableMask(faultBit);
            part->setPriority(RivPartPriority::PartType::Fault);

            m_nativeFaultFaces = part;
        }
    }

    // Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_nativeFaultGenerator->createMeshDrawable();
        if (geoMesh.notNull())
        {
            if (useBufferObjects)
            {
                geoMesh->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid mesh" + cvf::String(static_cast<int>(m_grid->gridIndex())));
            part->setDrawable(geoMesh.p());

            part->updateBoundingBox();
            part->setEnableMask(meshFaultBit);
            part->setPriority(RivPartPriority::PartType::FaultMeshLines);

            m_nativeFaultGridLines = part;
        }
    }


    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = m_oppositeFaultGenerator->generateSurface();
        if (geo.notNull())
        {
            geo->computeNormals();

            if (useBufferObjects)
            {
                geo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid " + cvf::String(static_cast<int>(m_grid->gridIndex())));
            part->setDrawable(geo.p());

            // Set mapping from triangle face index to cell index
            cvf::ref<RivSourceInfo> si = new RivSourceInfo(m_grid->gridIndex());
            si->m_cellFaceFromTriangleMapper = m_oppositeFaultGenerator->triangleToCellFaceMapper();
            part->setSourceInfo(si.p());

            part->updateBoundingBox();
            part->setEnableMask(faultBit);
            part->setPriority(RivPartPriority::PartType::Fault);

            m_oppositeFaultFaces = part;
        }
    }

    // Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_oppositeFaultGenerator->createMeshDrawable();
        if (geoMesh.notNull())
        {
            if (useBufferObjects)
            {
                geoMesh->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid mesh" + cvf::String(static_cast<int>(m_grid->gridIndex())));
            part->setDrawable(geoMesh.p());

            part->updateBoundingBox();
            part->setEnableMask(meshFaultBit);
            part->setPriority(RivPartPriority::PartType::FaultMeshLines);

            m_oppositeFaultGridLines = part;
        }
    }

    {
        cvf::ref<cvf::DrawableGeo> geo = m_NNCGenerator->generateSurface();
        if (geo.notNull())
        {
            geo->computeNormals();

            if (useBufferObjects)
            {
                geo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("NNC in Fault. Grid " + cvf::String(static_cast<int>(m_grid->gridIndex())));
            part->setDrawable(geo.p());

            // Set mapping from triangle face index to cell index
            cvf::ref<RivSourceInfo> si = new RivSourceInfo(m_grid->gridIndex());;
            si->m_NNCIndices = m_NNCGenerator->triangleToNNCIndex().p();
            part->setSourceInfo(si.p());

            part->updateBoundingBox();
            part->setEnableMask(faultBit);
            part->setPriority(RivPartPriority::PartType::Nnc);

            m_NNCFaces = part;
        }
    }
    
    createLabelWithAnchorLine(m_nativeFaultFaces.p());

    updatePartEffect();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::updatePartEffect()
{
    // Set default effect
    caf::SurfaceEffectGenerator geometryEffgen(m_defaultColor, caf::PO_1);
    geometryEffgen.setCullBackfaces(faceCullingMode());
  
    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();

    if (m_nativeFaultFaces.notNull())
    {
        m_nativeFaultFaces->setEffect(geometryOnlyEffect.p());
    }

    if (m_oppositeFaultFaces.notNull())
    {
        m_oppositeFaultFaces->setEffect(geometryOnlyEffect.p());
    }

    updateNNCColors(NULL);

    // Update mesh colors as well, in case of change
    RiaPreferences* prefs = RiaApplication::instance()->preferences();

    cvf::ref<cvf::Effect> eff;
    caf::MeshEffectGenerator faultEffGen(prefs->defaultFaultGridLineColors());
    eff = faultEffGen.generateCachedEffect();

    if (m_nativeFaultGridLines.notNull())
    {
        m_nativeFaultGridLines->setEffect(eff.p());
    }

    if (m_oppositeFaultGridLines.notNull())
    {
        m_oppositeFaultGridLines->setEffect(eff.p());
    }

    if (m_opacityLevel < 1.0f)
    {
        // Set priority to make sure this transparent geometry are rendered last
        if (m_nativeFaultFaces.notNull()) m_nativeFaultFaces->setPriority(RivPartPriority::PartType::TransparentFault);
        if (m_oppositeFaultFaces.notNull()) m_oppositeFaultFaces->setPriority(RivPartPriority::PartType::TransparentFault);
        if (m_NNCFaces.notNull())  m_NNCFaces->setPriority(RivPartPriority::PartType::TransparentNnc);

        if (m_nativeFaultGridLines.notNull())
        {
            m_nativeFaultGridLines->setPriority(RivPartPriority::PartType::FaultMeshLines);
        }

        if (m_oppositeFaultGridLines.notNull())
        {
            m_oppositeFaultGridLines->setPriority(RivPartPriority::PartType::FaultMeshLines);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::createLabelWithAnchorLine(const cvf::Part* part)
{
    m_faultLabelPart = NULL;
    m_faultLabelLinePart = NULL;

    if (!part) return;

    cvf::BoundingBox bb = part->boundingBox();

    cvf::Vec3d bbTopCenter = bb.center();
    bbTopCenter.z() = bb.max().z();

    const cvf::DrawableGeo* geo = dynamic_cast<const cvf::DrawableGeo*>(part->drawable());

    // Find closest vertex to top of bounding box.
    // Will be recomputed when filter changes, to make sure the label is always visible
    // for any filter combination
    cvf::Vec3f faultVertexToAttachLabel = findClosestVertex(cvf::Vec3f(bbTopCenter), geo->vertexArray());

    cvf::Vec3f labelPosition = faultVertexToAttachLabel;
    labelPosition.z() += bb.extent().z() / 2;

    // Fault label
    if (!m_rimFault->name().isEmpty())
    {
        cvf::Font* font = RiaApplication::instance()->customFont();

        cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
        drawableText->setFont(font);
        drawableText->setCheckPosVisible(false);
        drawableText->setDrawBorder(false);
        drawableText->setDrawBackground(false);
        drawableText->setVerticalAlignment(cvf::TextDrawer::CENTER);
        
        cvf::Color3f defWellLabelColor = RiaApplication::instance()->preferences()->defaultWellLabelColor();
        {
            RimFault* noConstRimFault = const_cast<RimFault*>(m_rimFault);
            if (noConstRimFault)
            {
                RimFaultCollection* parentObject;
                noConstRimFault->firstAncestorOrThisOfType(parentObject);
                if (parentObject)
                {
                    defWellLabelColor = parentObject->faultLabelColor();;
                }
            }
        }

        drawableText->setTextColor(defWellLabelColor);

        cvf::String cvfString = cvfqt::Utils::toString(m_rimFault->name());

        cvf::Vec3f textCoord(labelPosition);
        double characteristicCellSize = bb.extent().z() / 20;
        textCoord.z() += characteristicCellSize;

        drawableText->addText(cvfString, textCoord);

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("RivFaultPart : text " + cvfString);
        part->setDrawable(drawableText.p());

        cvf::ref<cvf::Effect> eff = new cvf::Effect;

        part->setEffect(eff.p());
        part->setPriority(RivPartPriority::PartType::Text);

        m_faultLabelPart = part;
    }


    // Line from fault geometry to label
    {
        cvf::ref<cvf::Vec3fArray> vertices = new cvf::Vec3fArray;
        vertices->reserve(2);
        vertices->add(faultVertexToAttachLabel);
        vertices->add(labelPosition);

        cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
        geo->setVertexArray(vertices.p());

        cvf::ref<cvf::PrimitiveSetDirect> primSet = new cvf::PrimitiveSetDirect(cvf::PT_LINES);
        primSet->setStartIndex(0);
        primSet->setIndexCount(vertices->size());
        geo->addPrimitiveSet(primSet.p());

        m_faultLabelLinePart = new cvf::Part;
        m_faultLabelLinePart->setName("Anchor line for label" + cvf::String(static_cast<int>(m_grid->gridIndex())));
        m_faultLabelLinePart->setDrawable(geo.p());

        m_faultLabelLinePart->updateBoundingBox();

        caf::MeshEffectGenerator gen(m_rimFault->faultColor());
        cvf::ref<cvf::Effect> eff = gen.generateCachedEffect();
        
        m_faultLabelLinePart->setEffect(eff.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RivFaultPartMgr::findClosestVertex(const cvf::Vec3f& point, const cvf::Vec3fArray* vertices)
{
    CVF_ASSERT(vertices);
    
    if (!vertices) return cvf::Vec3f::UNDEFINED;

    float closestDiff(HUGE_VAL);

    size_t closestIndex = cvf::UNDEFINED_SIZE_T;

    for (size_t i = 0; i < vertices->size(); i++)
    {
        float diff = point.pointDistance(vertices->get(i));

        if (diff < closestDiff)
        {
            closestDiff = diff;
            closestIndex = i;
        }
    }

    if (closestIndex != cvf::UNDEFINED_SIZE_T)
    {
        return vertices->get(closestIndex);
    }
    else
    {
        return cvf::Vec3f::UNDEFINED;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::appendNativeFaultFacesToModel(cvf::ModelBasicList* model)
{
    if (m_nativeFaultFaces.notNull())
    {
        model->addPart(m_nativeFaultFaces.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::appendOppositeFaultFacesToModel(cvf::ModelBasicList* model)
{
    if (m_oppositeFaultFaces.notNull())
    {
        model->addPart(m_oppositeFaultFaces.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::appendLabelPartsToModel(cvf::ModelBasicList* model)
{
    if (m_faultLabelPart.notNull())         model->addPart(m_faultLabelPart.p());
    if (m_faultLabelLinePart.notNull())     model->addPart(m_faultLabelLinePart.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::appendMeshLinePartsToModel(cvf::ModelBasicList* model)
{
    if (m_nativeFaultGridLines.notNull())   model->addPart(m_nativeFaultGridLines.p());
    if (m_oppositeFaultGridLines.notNull()) model->addPart(m_oppositeFaultGridLines.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::appendNNCFacesToModel(cvf::ModelBasicList* model)
{
    if (m_NNCFaces.notNull())   model->addPart(m_NNCFaces.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::FaceCulling RivFaultPartMgr::faceCullingMode() const
{
    bool isShowingGrid = m_rimFaultCollection->isGridVisualizationMode();
    if (!isShowingGrid )
    {
        if (m_rimFaultCollection->faultResult() == RimFaultCollection::FAULT_BACK_FACE_CULLING)
        {
            if (m_grid->mainGrid()->isFaceNormalsOutwards())
            {
                return caf::FC_BACK;
            }
            else
            {
                return caf::FC_FRONT;
            }
        }
        else if (m_rimFaultCollection->faultResult() == RimFaultCollection::FAULT_FRONT_FACE_CULLING)
        {
            if (m_grid->mainGrid()->isFaceNormalsOutwards())
            {
                return caf::FC_FRONT;
            }
            else
            {
                return caf::FC_BACK;
            }
        }
        else
        {
             return caf::FC_NONE;
        }
    }
    else
    {
        // Do not perform face culling in grid mode to make sure the displayed grid is watertight
        return caf::FC_NONE;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultPartMgr::updateNNCColors(RimEclipseCellColors* cellResultColors)
{
    if (m_NNCFaces.isNull()) return;

    bool showNncsWithScalarMappedColor = false;

    if (cellResultColors)
    {
        size_t scalarSetIndex = cellResultColors->scalarResultIndex();

        if (m_grid->mainGrid()->nncData()->hasScalarValues(scalarSetIndex))
        {
            showNncsWithScalarMappedColor = true;
        }
    }

    if (showNncsWithScalarMappedColor)
    {
        size_t scalarSetIndex = cellResultColors->scalarResultIndex();

        const cvf::ScalarMapper* mapper = cellResultColors->legendConfig()->scalarMapper();

        m_NNCGenerator->textureCoordinates(m_NNCTextureCoords.p(), mapper, scalarSetIndex);

        cvf::ref<cvf::Effect> nncEffect;

        if (m_rimFaultCollection->showFaultFaces || m_rimFaultCollection->showOppositeFaultFaces)
        {
            // Move NNC closer to camera to avoid z-fighting with grid surface
            caf::ScalarMapperEffectGenerator nncEffgen(mapper, caf::PO_NEG_LARGE);
            nncEffect = nncEffgen.generateCachedEffect();
        }
        else
        {
            // If no grid is present, use same offset as grid geometry to be able to see mesh lines
            caf::ScalarMapperEffectGenerator nncEffgen(mapper, caf::PO_1);
            nncEffect = nncEffgen.generateCachedEffect();
        }

        cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(m_NNCFaces->drawable());
        if (dg) dg->setTextureCoordArray(m_NNCTextureCoords.p());

        m_NNCFaces->setEffect(nncEffect.p());
    }
    else
    {
        // NNC faces a bit lighter than the fault for now
        cvf::Color3f nncColor = m_defaultColor;
        nncColor.r() +=  (1.0 - nncColor.r()) * 0.2;
        nncColor.g() +=  (1.0 - nncColor.g()) * 0.2;
        nncColor.g() +=  (1.0 - nncColor.b()) * 0.2;

        cvf::ref<cvf::Effect> nncEffect;

        if (m_rimFaultCollection->showFaultFaces || m_rimFaultCollection->showOppositeFaultFaces)
        {
            // Move NNC closer to camera to avoid z-fighting with grid surface
            caf::SurfaceEffectGenerator nncEffgen(nncColor, caf::PO_NEG_LARGE);
            nncEffect = nncEffgen.generateCachedEffect();
        }
        else
        {
            // If no grid is present, use same offset as grid geometry to be able to see mesh lines
            caf::SurfaceEffectGenerator nncEffgen(nncColor, caf::PO_1);
            nncEffect = nncEffgen.generateCachedEffect();
        }

        m_NNCFaces->setEffect(nncEffect.p());
    }
}

