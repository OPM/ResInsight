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

#include "RimReachCircleAnnotation.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaWellNameComparer.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "RimAnnotationInViewCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimGridView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellLogFile.h"
#include "RimWellPath.h"
#include "RimPerforationCollection.h"

#include "Riu3DMainWindowTools.h"

#include "RifWellPathFormationsImporter.h"
#include "RifWellPathImporter.h"

#include "cafPdmUiEditorHandle.h"
#include "cafProgressInfo.h"

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

#include <cmath>
#include <fstream>
#include "RimFileWellPath.h"
#include "RimModeledWellPath.h"


CAF_PDM_SOURCE_INIT(RimReachCircleAnnotation, "RimReachCircleAnnotation");


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReachCircleAnnotation::RimReachCircleAnnotation()
{
    CAF_PDM_InitObject("CircleAnnotation", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&m_centerPoint, "CenterPoint", Vec3d::ZERO, "Center Point", "", "", "");
    CAF_PDM_InitField(&m_radius, "Radius", 0.0, "Radius", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReachCircleAnnotation::setRadius(double radius)
{
    m_radius = radius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimReachCircleAnnotation::radius() const
{
    return m_radius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReachCircleAnnotation::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_centerPoint);
    uiOrdering.add(&m_radius);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReachCircleAnnotation::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                const QVariant&            oldValue,
                                                const QVariant&            newValue)
{
    auto views = gridViewsContainingAnnotations();
    if (!views.empty())
    {
        if (changedField == &m_centerPoint || changedField == &m_radius)
        {
            for (auto& view : views)
            {
                view->scheduleCreateDisplayModelAndRedraw();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridView*> RimReachCircleAnnotation::gridViewsContainingAnnotations() const
{
    std::vector<RimGridView*> views;
    RimProject*               project = nullptr;
    this->firstAncestorOrThisOfType(project);

    if (!project) return views;

    std::vector<RimGridView*> visibleGridViews;
    project->allVisibleGridViews(visibleGridViews);

    for (auto& gridView : visibleGridViews)
    {
        if (gridView->annotationCollection()->isActive()) views.push_back(gridView);
    }
    return views;
}
