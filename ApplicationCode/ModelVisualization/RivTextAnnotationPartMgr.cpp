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
#include "RiaColorTools.h"
#include "RiaPreferences.h"

#include "RimAnnotationCollection.h"
#include "RimTextAnnotation.h"

#include "RivPolylineGenerator.h"
#include "RivPartPriority.h"
#include "RivTextAnnotationSourceInfo.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfqtUtils.h"
#include "cafDisplayCoordTransform.h"


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
void RivTextAnnotationPartMgr::buildParts(const caf::DisplayCoordTransform * displayXf,
                                          bool doFlatten,
                                          double xOffset)
{
    clearAllGeometry();

    cvf::ref<RivTextAnnotationSourceInfo> sourceInfo = new RivTextAnnotationSourceInfo(m_rimAnnotation);

    cvf::Vec3d anchorPositionInDomain = m_rimAnnotation->anchorPoint();
    cvf::Vec3d labelPositionInDomain  = m_rimAnnotation->labelPoint();

    {
        auto* collection = dynamic_cast<RimAnnotationCollection*>(annotationCollection());
        if (collection && collection->snapAnnotations())
        {
            anchorPositionInDomain.z() = collection->annotationPlaneZ();
            labelPositionInDomain.z()  = collection->annotationPlaneZ();
        }
    }

    cvf::Vec3d anchorPosition = displayXf->transformToDisplayCoord(anchorPositionInDomain);
    cvf::Vec3d labelPosition = displayXf->transformToDisplayCoord(labelPositionInDomain);
    QString text = m_rimAnnotation->text();

    // Line part
    {
        std::vector<cvf::Vec3d> points = { anchorPosition, labelPosition };

        cvf::ref<cvf::DrawableGeo> drawableGeo = RivPolylineGenerator::createLineAlongPolylineDrawable(points);

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(drawableGeo.p());

        caf::MeshEffectGenerator    colorEffgen(cvf::Color3f::BLACK);
        cvf::ref<cvf::Effect>       eff = colorEffgen.generateUnCachedEffect();

        part->setEffect(eff.p());
        part->setPriority(RivPartPriority::PartType::MeshLines);
        part->setSourceInfo(sourceInfo.p());

        m_linePart = part;
    }

    // Text part
    {
        auto app = RiaApplication::instance();
        cvf::Font* font = app->customFont();
        auto prefs = app->preferences();

        cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
        drawableText->setFont(font);
        drawableText->setCheckPosVisible(false);
        drawableText->setDrawBorder(true);
        drawableText->setDrawBackground(true);
        drawableText->setVerticalAlignment(cvf::TextDrawer::BASELINE);
        drawableText->setBackgroundColor(prefs->defaultViewerBackgroundColor);
        drawableText->setBorderColor(RiaColorTools::computeOffsetColor(prefs->defaultViewerBackgroundColor, 0.3f));
        drawableText->setTextColor(cvf::Color3f::BLACK);

        cvf::String cvfString = cvfqt::Utils::toString(text);

        cvf::Vec3f textCoord(labelPosition);
        drawableText->addText(cvfString, textCoord);

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("RivTextAnnotationPartMgr: " + cvfString);
        part->setDrawable(drawableText.p());

        cvf::ref<cvf::Effect> eff = new cvf::Effect();
        part->setEffect(eff.p());
        part->setPriority(RivPartPriority::PartType::Text);
        part->setSourceInfo(sourceInfo.p());

        m_labelPart = part;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTextAnnotationPartMgr::clearAllGeometry()
{
    m_linePart = nullptr;
    m_labelPart = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTextAnnotationPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model,
                                                           const caf::DisplayCoordTransform * displayXf)
{
    if (m_rimAnnotation.isNull()) return;
    if (!validateAnnotation(m_rimAnnotation)) return;

    buildParts(displayXf, false, 0.0);
    model->addPart(m_linePart.p());
    model->addPart(m_labelPart.p());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTextAnnotationPartMgr::validateAnnotation(const RimTextAnnotation* annotation) const
{
    return m_rimAnnotation->anchorPoint() != cvf::Vec3d::ZERO && !m_rimAnnotation->text().isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationCollectionBase* RivTextAnnotationPartMgr::annotationCollection() const
{
    RimAnnotationCollectionBase* coll;
    m_rimAnnotation->firstAncestorOrThisOfType(coll);
    return coll;
}
