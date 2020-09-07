/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaSummaryTools.h"

#include "RiaFilePathTools.h"
#include "RifEclipseSummaryAddress.h"

#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "cafPdmObject.h"

#include <QRegularExpression>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection* RiaSummaryTools::summaryPlotCollection()
{
    RimProject* project = RimProject::current();

    return project->mainPlotCollection()->summaryPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlotCollection* RiaSummaryTools::summaryCrossPlotCollection()
{
    RimProject* project = RimProject::current();

    return project->mainPlotCollection()->summaryCrossPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection* RiaSummaryTools::summaryCaseMainCollection()
{
    RimProject*                   project                   = RimProject::current();
    RimSummaryCaseMainCollection* summaryCaseMainCollection = project->activeOilField()->summaryCaseMainCollection();
    CVF_ASSERT( summaryCaseMainCollection );
    return summaryCaseMainCollection;
}

//--------------------------------------------------------------------------------------------------
/// Update the summary curves referencing this curve, as the curve is identified by the name
//--------------------------------------------------------------------------------------------------
void RiaSummaryTools::notifyCalculatedCurveNameHasChanged( int calculationId, const QString& currentCurveName )
{
    RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();

    for ( RimSummaryPlot* plot : summaryPlotColl->summaryPlots() )
    {
        for ( RimSummaryCurve* curve : plot->summaryCurves() )
        {
            RifEclipseSummaryAddress adr = curve->summaryAddressY();
            if ( adr.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED && adr.id() == calculationId )
            {
                RifEclipseSummaryAddress updatedAdr =
                    RifEclipseSummaryAddress::calculatedAddress( currentCurveName.toStdString(), calculationId );
                curve->setSummaryAddressYAndApplyInterpolation( updatedAdr );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RiaSummaryTools::parentSummaryPlot( caf::PdmObject* object )
{
    if ( parentCrossPlot( object ) )
    {
        return nullptr;
    }

    RimSummaryPlot* summaryPlot = nullptr;

    if ( object )
    {
        object->firstAncestorOrThisOfType( summaryPlot );
    }

    return summaryPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection* RiaSummaryTools::parentSummaryPlotCollection( caf::PdmObject* object )
{
    RimSummaryPlotCollection* summaryPlotColl = nullptr;

    if ( object )
    {
        object->firstAncestorOrThisOfType( summaryPlotColl );
    }

    return summaryPlotColl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlot* RiaSummaryTools::parentCrossPlot( caf::PdmObject* object )
{
    RimSummaryCrossPlot* crossPlot = nullptr;

    if ( object )
    {
        object->firstAncestorOrThisOfType( crossPlot );
    }

    return crossPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCrossPlotCollection* RiaSummaryTools::parentCrossPlotCollection( caf::PdmObject* object )
{
    RimSummaryCrossPlotCollection* crossPlotColl = nullptr;

    if ( object )
    {
        object->firstAncestorOrThisOfType( crossPlotColl );
    }

    return crossPlotColl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSummaryTools::isSummaryCrossPlot( const RimSummaryPlot* plot )
{
    return dynamic_cast<const RimSummaryCrossPlot*>( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSummaryTools::hasAccumulatedData( const RifEclipseSummaryAddress& address )
{
    if ( address.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED )
    {
        std::vector<RimSummaryCase*>          cases;
        std::vector<RifEclipseSummaryAddress> addresses;

        getSummaryCasesAndAddressesForCalculation( address.id(), cases, addresses );
        for ( const RifEclipseSummaryAddress& variableAddress : addresses )
        {
            if ( !variableAddress.hasAccumulatedData() )
            {
                return false;
            }
        }

        // All the variables are accumulated
        return true;
    }

    return address.hasAccumulatedData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryTools::getSummaryCasesAndAddressesForCalculation( int                                    id,
                                                                 std::vector<RimSummaryCase*>&          cases,
                                                                 std::vector<RifEclipseSummaryAddress>& addresses )
{
    RimProject* proj = RimProject::current();

    RimSummaryCalculationCollection* calculationColl = proj->calculationCollection();
    if ( !calculationColl ) return;

    RimSummaryCalculation* calculation = calculationColl->findCalculationById( id );
    if ( !calculation ) return;

    for ( RimSummaryCalculationVariable* v : calculation->allVariables() )
    {
        cases.push_back( v->summaryCase() );
        addresses.push_back( v->summaryAddress()->address() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSummaryTools::findSuitableEnsembleName( const QStringList& summaryCaseFileNames )
{
    std::vector<QStringList> componentsForAllFilePaths;

    for ( auto filePath : summaryCaseFileNames )
    {
        QStringList components = RiaFilePathTools::splitPathIntoComponents( filePath );
        componentsForAllFilePaths.push_back( components );
    }

    // Find list of all folders inside a folder matching realization-*
    QRegularExpression realizationRe( "realization\\-\\d+" );

    QStringList iterations;
    for ( const auto& fileComponents : componentsForAllFilePaths )
    {
        QString lastComponent = "";
        for ( auto it = fileComponents.rbegin(); it != fileComponents.rend(); ++it )
        {
            if ( realizationRe.match( *it ).hasMatch() )
            {
                iterations.push_back( lastComponent );
            }
            lastComponent = *it;
        }
    }

    iterations.removeDuplicates();

    if ( iterations.size() == 1u )
    {
        return iterations.front();
    }
    else if ( !iterations.empty() )
    {
        return QString( "Multiple iterations: %1" ).arg( iterations.join( ", " ) );
    }

    QString root = RiaFilePathTools::commonRootOfFileNames( summaryCaseFileNames );

    QRegularExpression trimRe( "[^a-zA-Z0-9]+$" );
    QString            trimmedRoot = root.replace( trimRe, "" );
    if ( trimmedRoot.length() >= 4 )
    {
        return trimmedRoot;
    }

    return "Ensemble";
}
