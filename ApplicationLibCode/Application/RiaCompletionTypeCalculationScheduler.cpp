/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "RiaResultNames.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimProject.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include "cafPdmUiTreeView.h"

#include <QTimer>
#include <QTreeView>

#include <set>

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
    auto eclipseCases = RimProject::current()->eclipseCases();

    scheduleRecalculateCompletionTypeAndRedrawAllViews( eclipseCases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCompletionTypeCalculationScheduler::scheduleRecalculateCompletionTypeAndRedrawAllViews( const std::vector<RimEclipseCase*>& eclipseCases )
{
    clearCompletionTypeResults( eclipseCases );

    for ( RimEclipseCase* eclipseCase : eclipseCases )
    {
        if ( eclipseCase ) m_eclipseCasesToRecalculate.emplace_back( eclipseCase );
    }

    startTimer( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCompletionTypeCalculationScheduler::clearCompletionTypeResultsInAllCases()
{
    auto eclipseCases = RimProject::current()->eclipseCases();

    clearCompletionTypeResults( eclipseCases );

    // Clear geometry cache in views to recreate potential property filter geometry
    for ( auto eclipseCase : eclipseCases )
    {
        if ( !eclipseCase ) continue;

        for ( auto view : eclipseCase->views() )
        {
            if ( auto eclipseView = dynamic_cast<RimEclipseView*>( view ) )
            {
                eclipseView->scheduleReservoirGridGeometryRegen();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCompletionTypeCalculationScheduler::clearCompletionTypeResults( const std::vector<RimEclipseCase*>& eclipseCases )
{
    for ( RimEclipseCase* eclipseCase : eclipseCases )
    {
        if ( !eclipseCase || !eclipseCase->eclipseCaseData() ) continue;

        eclipseCase->eclipseCaseData()
            ->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
            ->clearScalarResult( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::completionTypeResultName() );

        // Delete virtual perforation transmissibilities, as these are the basis for the computation of completion type
        eclipseCase->eclipseCaseData()->setVirtualPerforationTransmissibilities( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCompletionTypeCalculationScheduler::performScheduledUpdates()
{
    std::set<RimEclipseCase*> uniqueCases( m_eclipseCasesToRecalculate.begin(), m_eclipseCasesToRecalculate.end() );

    for ( RimEclipseCase* eclipseCase : uniqueCases )
    {
        if ( eclipseCase )
        {
            for ( auto view : eclipseCase->views() )
            {
                if ( auto eclipseView = dynamic_cast<RimEclipseView*>( view ) )
                {
                    eclipseView->calculateCompletionTypeAndRedrawIfRequired();
                }
            }
        }
    }

    m_eclipseCasesToRecalculate.clear();

    // Recalculation of completion type causes active view to be set to potentially a different view
    // Also current index in project tree is changed. Restore both to initial state.

    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if ( activeView && activeView->viewer() )
    {
        RiaApplication::instance()->setActiveReservoirView( activeView );
        if ( RiuMainWindow::instance() )
        {
            RiuMainWindow::instance()->setActiveViewer( activeView->viewer()->layoutWidget() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaCompletionTypeCalculationScheduler::~RiaCompletionTypeCalculationScheduler()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaCompletionTypeCalculationScheduler::RiaCompletionTypeCalculationScheduler()
{
}
