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

#include "RimPolylineAnnotation.h"

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


CAF_PDM_SOURCE_INIT(RimPolylineAnnotation, "RimPolylineAnnotation");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylineAnnotation::RimPolylineAnnotation()
{
    CAF_PDM_InitObject("PolylineAnnotation", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&m_points, "Points", {}, "", "", "", "");
}

RimPolylineAnnotation::~RimPolylineAnnotation()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineAnnotation::setPoints(const std::vector<Vec3d>& points)
{
    m_points = points;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RimPolylineAnnotation::Vec3d>& RimPolylineAnnotation::points() const
{
    return m_points();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineAnnotation::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_points);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineAnnotation::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue)
{
    auto views = gridViewsContainingAnnotations();
    if (!views.empty())
    {
        if (changedField == &m_points)
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
std::vector<RimGridView*> RimPolylineAnnotation::gridViewsContainingAnnotations() const
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
