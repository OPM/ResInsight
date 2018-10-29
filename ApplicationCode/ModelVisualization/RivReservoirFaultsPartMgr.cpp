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

#include "RivReservoirFaultsPartMgr.h"

#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"

#include "RivFaultPartMgr.h"

#include "cvfColor3.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"

#include "cafPdmFieldCvfColor.h"

#include <QDebug>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReservoirFaultsPartMgr::RivReservoirFaultsPartMgr(const RigMainGrid* grid,  RimEclipseView* reservoirView)
:   m_reservoirView(reservoirView)
{
    CVF_ASSERT(grid);

    if (reservoirView)
    {
        RimFaultInViewCollection* faultCollection = reservoirView->faultCollection();
        if (faultCollection)
        {
            for (size_t i = 0; i < faultCollection->faults.size(); i++)
            {
                m_faultParts.push_back(new RivFaultPartMgr(grid, faultCollection, faultCollection->faults[i]));
            }
        }
    }

    m_forceWatertightGeometry = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReservoirFaultsPartMgr::~RivReservoirFaultsPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFaultsPartMgr::setTransform(cvf::Transform* scaleTransform)
{
    m_scaleTransform = scaleTransform;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFaultsPartMgr::setCellVisibility(cvf::UByteArray* cellVisibilities)
{
    CVF_ASSERT(cellVisibilities);

    for (size_t i = 0; i < m_faultParts.size(); i++)
    {
        m_faultParts.at(i)->setCellVisibility(cellVisibilities);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFaultsPartMgr::appendPartsToModel(cvf::ModelBasicList* model)
{
    CVF_ASSERT(model != nullptr);

    RimFaultInViewCollection* faultCollection = m_reservoirView->faultCollection();
    if (!faultCollection) return;

 
    bool isShowingGrid = m_reservoirView->isMainGridVisible();
    if (!faultCollection->showFaultCollection() && !isShowingGrid) return;
    
    // Check match between model fault count and fault parts
    CVF_ASSERT(faultCollection->faults.size() == m_faultParts.size());

    // Parts that is overridden by the grid settings
    bool forceDisplayOfFault = false;
    if (!faultCollection->isShowingFaultsAndFaultsOutsideFilters())
    {
        forceDisplayOfFault = isShowingGrid;
    }

    if (m_forceWatertightGeometry && isShowingGrid)
    {
        forceDisplayOfFault = true;
    }

    //qDebug() << forceDisplayOfFault;

    cvf::ModelBasicList parts;

    for (size_t i = 0; i < faultCollection->faults.size(); i++)
    {
        const RimFaultInView* rimFault = faultCollection->faults[i];

        cvf::ref<RivFaultPartMgr> rivFaultPart = m_faultParts[i];
        CVF_ASSERT(rivFaultPart.notNull());

        if ( (faultCollection->showFaultCollection() && rimFault->showFault()) || forceDisplayOfFault)
        {
            if (faultCollection->showFaultFaces() || forceDisplayOfFault)
            {
                rivFaultPart->appendNativeFaultFacesToModel(&parts);
            }

            if (faultCollection->showOppositeFaultFaces() || forceDisplayOfFault)
            {
                rivFaultPart->appendOppositeFaultFacesToModel(&parts);
            }

            if (faultCollection->showFaultFaces() || faultCollection->showOppositeFaultFaces() || faultCollection->showNNCs() || forceDisplayOfFault)
            {
                rivFaultPart->appendMeshLinePartsToModel(&parts);
            }
        }

        // Parts that is not overridden by the grid settings
        RimEclipseFaultColors* faultResultColors = m_reservoirView->faultResultSettings();
        RimEclipseCellColors* cellResultColors = m_reservoirView->cellResult();

        if (rimFault->showFault() && faultCollection->showFaultCollection())
        {
            if (faultCollection->showNNCs())
            {
                bool showNncs = true;
                if (faultCollection->hideNncsWhenNoResultIsAvailable())
                {
                    size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;
                    if (faultResultColors->showCustomFaultResult())
                    {
                        scalarResultIndex = faultResultColors->customFaultResult()->scalarResultIndex();
                    }
                    else
                    {
                        scalarResultIndex = cellResultColors->scalarResultIndex();
                    }

                    RigMainGrid* mainGrid = m_reservoirView->mainGrid();
                    if (!(mainGrid && mainGrid->nncData()->hasScalarValues(scalarResultIndex)))
                    {
                        showNncs = false;
                    }
                }

                if (showNncs)
                {
                    rivFaultPart->appendNNCFacesToModel(&parts);
                }
            }
        }
    }

    for (size_t i = 0; i < parts.partCount(); i++)
    {
        cvf::Part* part = parts.part(i);
        part->setTransform(m_scaleTransform.p());
        
        model->addPart(part);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFaultsPartMgr::applySingleColorEffect()
{
    for (size_t i = 0; i < m_faultParts.size(); i++)
    {
        m_faultParts[i]->applySingleColorEffect();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFaultsPartMgr::updateColors(size_t timeStepIndex, RimEclipseCellColors* cellResultColors)
{
    if (!m_reservoirView) return;

    RimFaultInViewCollection* faultCollection = m_reservoirView->faultCollection();
    CVF_ASSERT(faultCollection);

    for (size_t i = 0; i < faultCollection->faults.size(); i++)
    {
        if (cellResultColors && (cellResultColors->hasResult() || cellResultColors->isTernarySaturationSelected()) )
        {
            m_faultParts[i]->updateCellResultColor(timeStepIndex, cellResultColors);
        }
        else
        {
            m_faultParts[i]->applySingleColorEffect();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFaultsPartMgr::updateCellEdgeResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors, RimCellEdgeColors* cellEdgeResultColors)
{
    for (size_t i = 0; i < m_faultParts.size(); i++)
    {
        m_faultParts[i]->updateCellEdgeResultColor(timeStepIndex, cellResultColors, cellEdgeResultColors);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFaultsPartMgr::appendLabelPartsToModel(cvf::ModelBasicList* model)
{
    CVF_ASSERT(model != nullptr);
    if (!m_reservoirView) return;

    RimFaultInViewCollection* faultCollection = m_reservoirView->faultCollection();
    CVF_ASSERT(faultCollection);

    if (!faultCollection->showFaultCollection()) return;
    
    if (!faultCollection->showFaultLabel() ) return;

    // Check match between model fault count and fault parts
    CVF_ASSERT(faultCollection->faults.size() == m_faultParts.size());

    cvf::ModelBasicList parts;

    for (size_t i = 0; i < faultCollection->faults.size(); i++)
    {
        const RimFaultInView* rimFault = faultCollection->faults[i];

        cvf::ref<RivFaultPartMgr> rivFaultPart = m_faultParts[i];
        CVF_ASSERT(rivFaultPart.notNull());

        if (rimFault->showFault())
        {
            rivFaultPart->appendLabelPartsToModel(&parts);
        }
    }

    for (size_t i = 0; i < parts.partCount(); i++)
    {
        cvf::Part* part = parts.part(i);
        part->setTransform(m_scaleTransform.p());

        model->addPart(part);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFaultsPartMgr::forceWatertightGeometryOn()
{
    m_forceWatertightGeometry = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFaultsPartMgr::clearWatertightGeometryFlag()
{
    m_forceWatertightGeometry = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirFaultsPartMgr::setOpacityLevel(float opacity)
{
    for (size_t i = 0; i < m_faultParts.size(); i++)
    {
        m_faultParts[i]->setOpacityLevel(opacity);
    }
}

