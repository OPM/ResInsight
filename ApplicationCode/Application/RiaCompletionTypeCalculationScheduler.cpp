/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RiaCompletionTypeCalculationScheduler.h"

#include "RiaApplication.h"

#include "RigEclipseCaseData.h"
#include "RigCaseCellResultsData.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include "cafPdmUiTreeView.h"

#include <QTimer>
#include <QTreeView>

#include <set>
#include "cafProgressState.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaCompletionTypeCalculationScheduler* RiaCompletionTypeCalculationScheduler::instance()
{
    static RiaCompletionTypeCalculationScheduler theInstance;

    return &theInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCompletionTypeCalculationScheduler::scheduleRecalculateCompletionTypeAndRedrawAllViews()
{
    std::vector<RimEclipseCase*> eclipseCases =
        RiaApplication::instance()->project()->activeOilField()->analysisModels->cases().childObjects();

    scheduleRecalculateCompletionTypeAndRedrawAllViews(eclipseCases);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCompletionTypeCalculationScheduler::scheduleRecalculateCompletionTypeAndRedrawAllViews(RimEclipseCase* eclipseCase)
{
    std::vector<RimEclipseCase*> eclipseCases;
    eclipseCases.push_back(eclipseCase);

    scheduleRecalculateCompletionTypeAndRedrawAllViews(eclipseCases);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCompletionTypeCalculationScheduler::scheduleRecalculateCompletionTypeAndRedrawAllViews(
    const std::vector<RimEclipseCase*>& eclipseCases)
{
    for (RimEclipseCase* eclipseCase : eclipseCases)
    {
        CVF_ASSERT(eclipseCase);

        if (eclipseCase->eclipseCaseData())
        {
            eclipseCase->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->clearScalarResult(RiaDefines::DYNAMIC_NATIVE, RiaDefines::completionTypeResultName());

            // Delete virtual perforation transmissibilities, as these are the basis for the computation of completion type
            eclipseCase->eclipseCaseData()->setVirtualPerforationTransmissibilities(nullptr);
        }

        m_eclipseCasesToRecalculate.push_back(eclipseCase);
    }

    startTimer();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCompletionTypeCalculationScheduler::slotRecalculateCompletionType()
{
    if ( caf::ProgressState::isActive() )
    {
        startTimer();
        return;
    }

    std::set<RimEclipseCase*> uniqueCases(m_eclipseCasesToRecalculate.begin(), m_eclipseCasesToRecalculate.end());

    Rim3dView*  activeView = RiaApplication::instance()->activeReservoirView();
    QModelIndex mi         = RiuMainWindow::instance()->projectTreeView()->treeView()->currentIndex();

    for (RimEclipseCase* eclipseCase : uniqueCases)
    {
        for (const auto& w : eclipseCase->views())
        {
            RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(w);
            if (eclView)
            {
                eclView->calculateCompletionTypeAndRedrawIfRequired();
            }
        }
    }

    m_eclipseCasesToRecalculate.clear();

    // Recalculation of completion type causes active view to be set to potentially a different view
    // Also current index in project tree is changed. Restore both to initial state.

    if (activeView && activeView->viewer())
    {
        RiaApplication::instance()->setActiveReservoirView(activeView);
        RiuMainWindow::instance()->setActiveViewer(activeView->viewer()->layoutWidget());
    }

    if (mi.isValid())
    {
        RiuMainWindow::instance()->projectTreeView()->treeView()->setCurrentIndex(mi);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaCompletionTypeCalculationScheduler::~RiaCompletionTypeCalculationScheduler()
{
    delete m_recalculateCompletionTypeTimer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCompletionTypeCalculationScheduler::startTimer()
{
    if (!m_recalculateCompletionTypeTimer)
    {
        m_recalculateCompletionTypeTimer = new QTimer(this);
        m_recalculateCompletionTypeTimer->setSingleShot(true);
        connect(m_recalculateCompletionTypeTimer, SIGNAL(timeout()), this, SLOT(slotRecalculateCompletionType()));
    }

    m_recalculateCompletionTypeTimer->start(1500);
}
