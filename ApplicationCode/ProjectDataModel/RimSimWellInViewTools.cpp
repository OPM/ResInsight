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

#include "RimSimWellInViewTools.h"

#include "RiaApplication.h"

#include "RigSimWellData.h"

#include "Rim3dView.h"
#include "RimEclipseResultCase.h"
#include "RimGridSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimSummaryCaseMainCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridSummaryCase* RimSimWellInViewTools::gridSummaryCaseForWell( RimSimWellInView* well )
{
    RimProject* project = RiaApplication::instance()->project();
    if ( !project ) return nullptr;

    RimSummaryCaseMainCollection* sumCaseColl = project->activeOilField()
                                                    ? project->activeOilField()->summaryCaseMainCollection()
                                                    : nullptr;
    if ( !sumCaseColl ) return nullptr;

    RimEclipseResultCase* eclCase = nullptr;
    well->firstAncestorOrThisOfType( eclCase );
    if ( eclCase )
    {
        RimGridSummaryCase* gridSummaryCase = dynamic_cast<RimGridSummaryCase*>(
            sumCaseColl->findSummaryCaseFromEclipseResultCase( eclCase ) );
        if ( gridSummaryCase )
        {
            return gridSummaryCase;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSimWellInViewTools::isInjector( RimSimWellInView* well )
{
    RigSimWellData* wRes = well->simWellData();
    if ( wRes )
    {
        Rim3dView* rimView = nullptr;
        well->firstAncestorOrThisOfTypeAsserted( rimView );

        int currentTimeStep = rimView->currentTimeStep();

        if ( wRes->hasWellResult( currentTimeStep ) )
        {
            const RigWellResultFrame& wrf = wRes->wellResultFrame( currentTimeStep );

            if ( wrf.m_productionType == RigWellResultFrame::OIL_INJECTOR ||
                 wrf.m_productionType == RigWellResultFrame::GAS_INJECTOR ||
                 wrf.m_productionType == RigWellResultFrame::WATER_INJECTOR )
            {
                return true;
            }
        }
    }

    return false;
}
