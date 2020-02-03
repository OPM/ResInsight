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

#include "RiaApplication.h"
#include "RiaSummaryTools.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RifEclipseSummaryAddress.h"
#include "RifSummaryReaderInterface.h"

#include "RigSimWellData.h"

#include "Rim3dView.h"
#include "RimEclipseResultCase.h"
#include "RimGridSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimSummaryCaseMainCollection.h"

#include "cvfAssert.h"

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSimWellInViewTools::extractValueForTimeStep( RifSummaryReaderInterface* summaryReader,
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

    RifEclipseSummaryAddress addr( RifEclipseSummaryAddress::SUMMARY_WELL,
                                   vectorName,
                                   -1,
                                   -1,
                                   "",
                                   wellName.toStdString(),
                                   -1,
                                   "",
                                   -1,
                                   -1,
                                   -1,
                                   -1,
                                   false,
                                   -1 );

    if ( !summaryReader->hasAddress( addr ) )
    {
        // TODO: better error handling
        std::cerr << "ERROR: no address found for well " << wellName.toStdString() << " " << vectorName << std::endl;
        *isOk = false;
        return 0.0;
    }

    std::vector<double> values;
    summaryReader->values( addr, &values );
    std::vector<time_t> timeSteps = summaryReader->timeSteps( addr );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData( values, timeSteps );
    if ( RiaSummaryTools::hasAccumulatedData( addr ) )
    {
        resampler.resampleAndComputePeriodEndValues( DateTimePeriod::DAY );
    }
    else
    {
        resampler.resampleAndComputeWeightedMeanValues( DateTimePeriod::DAY );
    }

    // Find the data point which best matches the selected time step
    std::vector<time_t> resampledTimeSteps = resampler.resampledTimeSteps();
    std::vector<double> resampledValues    = resampler.resampledValues();
    for ( unsigned int i = 0; i < resampledTimeSteps.size(); i++ )
    {
        QDateTime t = QDateTime::fromTime_t( resampledTimeSteps[i] );
        if ( t > currentDate )
        {
            *isOk = true;
            return resampledValues[i];
        }
    }

    std::cerr << "ERROR: no resampled value found for well " << wellName.toStdString() << " " << vectorName << std::endl;
    *isOk = false;
    return -1;
}
