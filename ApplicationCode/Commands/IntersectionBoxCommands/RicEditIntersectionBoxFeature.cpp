/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicEditIntersectionBoxFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimIntersectionBox.h"
#include "RimView.h"

#include "RiuViewer.h"

#include "cafBoxManipulatorGeometryGenerator.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicEditIntersectionBoxFeature, "RicEditIntersectionBoxFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditIntersectionBoxFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditIntersectionBoxFeature::onActionTriggered(bool isChecked)
{
    RiuViewer* viewer = nullptr;

    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (activeView && activeView->viewer())
    {
        viewer = activeView->viewer();
    }

    if (isCommandChecked() && m_model.notNull())
    {
        if (viewer) viewer->removeStaticModel(m_model.p());
        m_model = nullptr;
    }
    else if (viewer)
    {
        RimIntersectionBox* intersectionBox = dynamic_cast<RimIntersectionBox*>(viewer->lastPickedObject());
        if (intersectionBox)
        {
            caf::BoxManipulatorGeometryGenerator boxGen;

            // TODO: boxGen operates in display coordinates, conversion is missing
            boxGen.setOrigin(intersectionBox->boxOrigin());
            boxGen.setSize(intersectionBox->boxSize());

            cvf::ref<cvf::DrawableGeo> geoMesh = boxGen.createBoundingBoxMeshDrawable();
            if (geoMesh.notNull())
            {
                cvf::ref<cvf::Part> part = new cvf::Part;
                part->setName("Cross Section mesh");
                part->setDrawable(geoMesh.p());

                part->updateBoundingBox();
//                     part->setEnableMask(meshFaultBit);
//                     part->setPriority(priMesh);

                cvf::ref<cvf::Effect> eff;
                caf::MeshEffectGenerator CrossSectionEffGen(cvf::Color3::WHITE);
                eff = CrossSectionEffGen.generateCachedEffect();
                part->setEffect(eff.p());

                m_model = new cvf::ModelBasicList;
                m_model->addPart(part.p());
                viewer->addStaticModelOnce(m_model.p());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditIntersectionBoxFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/IntersectionBox16x16.png"));
    actionToSetup->setText("Edit Intersection Box");
    actionToSetup->setCheckable(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditIntersectionBoxFeature::isCommandChecked()
{
    return m_model.notNull();
}
