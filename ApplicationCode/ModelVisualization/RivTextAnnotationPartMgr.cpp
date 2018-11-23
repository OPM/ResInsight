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



#include "RivTextAnnotationPartMgr.h"

#include "RiaApplication.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"

//#include "RimAnnotationInView.h"
#include "RimTextAnnotation.h"
#include "RimAnnotationInViewCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"
#include "RimSimWellInView.h"

#include "RivPipeGeometryGenerator.h"
#include "RivPolylineGenerator.h"
#include "RivPartPriority.h"
#include "RivTextAnnotationSourceInfo.h"

#include "cafEffectGenerator.h"

#include "cvfArrowGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"
#include "cvfqtUtils.h"
#include "cafDisplayCoordTransform.h"
#include "RivSectionFlattner.h"


static RimSimWellInViewCollection* simWellInViewCollection() { return nullptr; }

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivTextAnnotationPartMgr::RivTextAnnotationPartMgr(RimTextAnnotation* annotation)
: m_rimAnnotation(annotation)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivTextAnnotationPartMgr::~RivTextAnnotationPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTextAnnotationPartMgr::buildTextAnnotationParts(const caf::DisplayCoordTransform * displayXf,
                                            bool doFlatten, 
                                            double xOffset)
{
    clearAllGeometry();

    auto textAnnotation = dynamic_cast<RimTextAnnotation*>(m_rimAnnotation.p());

    if(textAnnotation)
    {
        cvf::ref<RivTextAnnotationSourceInfo> sourceInfo = new RivTextAnnotationSourceInfo(m_rimAnnotation);

        cvf::Vec3d textPosition = displayXf->transformToDisplayCoord(m_rimAnnotation->anchorPoint());
        QString text = textAnnotation->text();

        if (!text.isEmpty())
        {
            cvf::Font* font = RiaApplication::instance()->customFont();

            cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
            drawableText->setFont(font);
            drawableText->setCheckPosVisible(false);
            drawableText->setDrawBorder(false);
            drawableText->setDrawBackground(false);
            drawableText->setVerticalAlignment(cvf::TextDrawer::CENTER);
            drawableText->setTextColor(cvf::Color3f::BLACK);//   simWellInViewCollection()->wellLabelColor());

            cvf::String cvfString = cvfqt::Utils::toString(text);

            cvf::Vec3f textCoord(textPosition);
            drawableText->addText(cvfString, textCoord);

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("RivAnnotationPartMgr: text " + cvfString);
            part->setDrawable(drawableText.p());

            cvf::ref<cvf::Effect> eff = new cvf::Effect;

            part->setEffect(eff.p());
            part->setPriority(RivPartPriority::PartType::Text);
            part->setSourceInfo(sourceInfo.p());

            m_part = part;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTextAnnotationPartMgr::clearAllGeometry()
{
    m_part = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTextAnnotationPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model,
                                                           const caf::DisplayCoordTransform * displayXf)
{
    if (m_rimAnnotation.isNull()) return;
    if (!validateAnnotation(m_rimAnnotation)) return;

    buildTextAnnotationParts(displayXf, false, 0.0);
    model->addPart(m_part.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTextAnnotationPartMgr::appendFlattenedDynamicGeometryPartsToModel(cvf::ModelBasicList* model,
                                                           size_t frameIndex, 
                                                           const caf::DisplayCoordTransform * displayXf,
                                                           double xOffset)
{
    ///////////////////////////////////////////
    caf::PdmPointer<RimSimWellInView> m_rimWell;
    cvf::ref<cvf::Part>               m_wellHeadPipeSurfacePart;
    cvf::ref<cvf::Part>               m_wellHeadPipeCenterPart;
    cvf::ref<cvf::Part>               m_wellHeadArrowPart;
    cvf::ref<cvf::Part>               m_wellHeadLabelPart;
    ///////////////////////////////////////////

    if (m_rimWell.isNull()) return;
    if (!viewWithSettings()) return;

    if (!m_rimWell->isWellPipeVisible(frameIndex)) return;

    //buildParts(displayXf, true, xOffset);

    // Always add pipe part of well head
    if (m_wellHeadPipeCenterPart.notNull()) model->addPart(m_wellHeadPipeCenterPart.p());
    if (m_wellHeadPipeSurfacePart.notNull()) model->addPart(m_wellHeadPipeSurfacePart.p());

    if (m_rimWell->showWellLabel() && 
        m_wellHeadLabelPart.notNull())
    {
        model->addPart(m_wellHeadLabelPart.p());
    }

    if (m_rimWell->showWellHead() &&
        m_wellHeadArrowPart.notNull())
    {
        model->addPart(m_wellHeadArrowPart.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dView* RivTextAnnotationPartMgr::viewWithSettings()
{
    Rim3dView* view = nullptr;
    if (m_rimAnnotation) m_rimAnnotation->firstAncestorOrThisOfType(view);

    return view;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationInViewCollection* RivTextAnnotationPartMgr::annotatationInViewCollection()
{
    RimAnnotationInViewCollection* coll = nullptr;
    if (m_rimAnnotation)  m_rimAnnotation->firstAncestorOrThisOfType(coll);

    return coll;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTextAnnotationPartMgr::validateAnnotation(const RimTextAnnotation* annotation) const
{
    return m_rimAnnotation->anchorPoint() != cvf::Vec3d::ZERO && !m_rimAnnotation->text().isEmpty();
}

