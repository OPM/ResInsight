/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiaLogging.h"
#include "Summary/RiaSummaryTools.h"

#include "RifEclipseSummaryAddress.h"
#include "RifSummaryReaderInterface.h"

#include "Well/RigSimWellData.h"
#include "Well/RigWellResultFrame.h"

#include "Rim3dView.h"
#include "RimEclipseResultCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimSummaryCaseMainCollection.h"

#include "cvfAssert.h"

namespace RimSimWellInViewTools
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* summaryCaseForWell( RimSimWellInView* well )
{
    RimProject* project = RimProject::current();
    if ( !project ) return nullptr;

    auto eclCase = well->firstAncestorOrThisOfType<RimEclipseResultCase>();
    if ( eclCase )
    {
        return RimReloadCaseTools::findSummaryCaseFromEclipseResultCase( eclCase );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> summaryCases()
{
    std::vector<RimSummaryCase*> cases;

    RimProject* project = RimProject::current();
    if ( project )
    {
        RimSummaryCaseMainCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection()
                                                                              : nullptr;
        if ( sumCaseColl )
        {
            cases = sumCaseColl->allSummaryCases();
        }
    }

    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isInjector( RimSimWellInView* well )
{
    RigSimWellData* wRes = well->simWellData();
    if ( wRes )
    {
        auto rimView = well->firstAncestorOrThisOfTypeAsserted<Rim3dView>();

        int currentTimeStep = rimView->currentTimeStep();

        if ( wRes->hasWellResult( currentTimeStep ) )
        {
            const RigWellResultFrame* wrf = wRes->wellResultFrame( currentTimeStep );

            if ( RiaDefines::isInjector( wrf->productionType() ) )
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isProducer( RimSimWellInView* well )
{
    RigSimWellData* wRes = well->simWellData();
    if ( wRes )
    {
        auto rimView = well->firstAncestorOrThisOfTypeAsserted<Rim3dView>();

        int currentTimeStep = rimView->currentTimeStep();

        if ( wRes->hasWellResult( currentTimeStep ) )
        {
            const RigWellResultFrame* wrf = wRes->wellResultFrame( currentTimeStep );

            if ( RiaDefines::WellProductionType::PRODUCER == wrf->productionType() )
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double extractValueForTimeStep( RifSummaryReaderInterface* summaryReader,
                                const QString&             wellName,
                                const std::string&         vectorName,
                                const QDateTime&           currentDate,
                                bool*                      isOk )

{
    CVF_ASSERT( summaryReader );

    if ( vectorName.empty() )
    {
        *isOk = true;
        return 0.0;
    }

    auto addr = RifEclipseSummaryAddress::wellAddress( vectorName, wellName.toStdString(), -1 );

    if ( !summaryReader->hasAddress( addr ) )
    {
        QString message = "ERROR: no address found for well " + wellName + " " + QString::fromStdString( vectorName );
        RiaLogging::warning( message );

        *isOk = false;
        return 0.0;
    }

    auto [isOkReader, values]     = summaryReader->values( addr );
    std::vector<time_t> timeSteps = summaryReader->timeSteps( addr );
    if ( values.empty() || timeSteps.empty() )
    {
        QString message = "ERROR: no data found for well " + wellName + " " + QString::fromStdString( vectorName );
        RiaLogging::warning( message );

        *isOk = false;
        return 0.0;
    }

    auto [resampledTimeSteps, resampledValues] =
        RiaSummaryTools::resampledValuesForPeriod( addr, timeSteps, values, RiaDefines::DateTimePeriod::DAY );

    time_t currentTime_t = currentDate.toSecsSinceEpoch();

    for ( unsigned int i = 0; i < resampledTimeSteps.size(); i++ )
    {
        if ( resampledTimeSteps[i] > currentTime_t )
        {
            *isOk = true;
            return resampledValues[i];
        }
    }

    QString message = "ERROR: no resampled values found for well " + wellName + " " + QString::fromStdString( vectorName );
    RiaLogging::warning( message );

    *isOk = false;
    return -1;
}

} // namespace RimSimWellInViewTools
