////////////////////////////////////////////////////////////////////////////////
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

#include "RicSummaryPlotTemplateTools.h"

#include "RiaLogging.h"
#include "RiaSummaryCurveAnalyzer.h"

#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

#include <QFile>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSummaryPlotTemplateTools::createPlotFromTemplateFile( const QString& fileName )
{
    QFile importFile( fileName );
    if ( !importFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        RiaLogging::error( QString( "Create Plot from Template : Could not open the file: %1" ).arg( fileName ) );
        return nullptr;
    }

    QTextStream stream( &importFile );

    QString objectAsText = stream.readAll();

    caf::PdmObjectHandle* obj =
        caf::PdmXmlObjectHandle::readUnknownObjectFromXmlString( objectAsText, caf::PdmDefaultObjectFactory::instance() );

    RimSummaryPlot* newSummaryPlot = dynamic_cast<RimSummaryPlot*>( obj );
    if ( newSummaryPlot )
    {
        return newSummaryPlot;
    }

    delete obj;

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::htmlTextFromPlotAndSelection(
    const RimSummaryPlot*                     templatePlot,
    const std::set<RifEclipseSummaryAddress>& selectedSummaryAddresses,
    const std::vector<caf::PdmObject*>&       selectedSources )
{
    QString text;

    RiaSummaryCurveAnalyzer selectionAnalyzer;

    selectionAnalyzer.appendAdresses( selectedSummaryAddresses );

    if ( templatePlot )
    {
        std::set<QString>       templateSources;
        RiaSummaryCurveAnalyzer templateAnalyzer;

        {
            std::set<RifEclipseSummaryAddress> templateAddresses;

            for ( const auto& curve : templatePlot->summaryCurves() )
            {
                auto adr = curve->summaryAddressY();
                templateAddresses.insert( adr );

                auto fieldHandle = curve->findField( "SummaryCase" );
                if ( fieldHandle )
                {
                    auto test = fieldHandle->xmlCapability()->referenceString();
                    templateSources.insert( test );
                }
            }

            templateAnalyzer.appendAdresses( templateAddresses );
        }

        text += "<b> Requirements </b><br>";

        if ( !templateSources.empty() )
        {
            QString itemText      = "Source Cases";
            size_t  requiredCount = templateSources.size();
            size_t  selectedCount = selectedSources.size();
            text += htmlTextFromCount( itemText, requiredCount, selectedCount );
        }

        if ( !templateAnalyzer.wellNames().empty() )
        {
            QString itemText      = "Wells";
            size_t  requiredCount = templateAnalyzer.wellNames().size();
            size_t  selectedCount = selectionAnalyzer.wellNames().size();
            text += htmlTextFromCount( itemText, requiredCount, selectedCount );
        }

        if ( !templateAnalyzer.wellGroupNames().empty() )
        {
            QString itemText      = "Well Groups";
            size_t  requiredCount = templateAnalyzer.wellGroupNames().size();
            size_t  selectedCount = selectionAnalyzer.wellGroupNames().size();
            text += htmlTextFromCount( itemText, requiredCount, selectedCount );
        }

        if ( !templateAnalyzer.regionNumbers().empty() )
        {
            QString itemText      = "Regions";
            size_t  requiredCount = templateAnalyzer.regionNumbers().size();
            size_t  selectedCount = selectionAnalyzer.regionNumbers().size();
            text += htmlTextFromCount( itemText, requiredCount, selectedCount );
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::htmlTextFromCount( const QString& itemText,
                                                        size_t         requiredItemCount,
                                                        size_t         selectionCount )
{
    QString text;

    QString colorString = "green";

    if ( selectionCount < requiredItemCount )
        colorString = "red";
    else if ( selectionCount > requiredItemCount )
        colorString = "orange";

    text += QString( "<b><font color='%1'>" ).arg( colorString );
    text += QString( "%1 : %2 selected (%3 required)\n" ).arg( itemText ).arg( selectionCount ).arg( requiredItemCount );
    text += "</font></b>";

    text += "<br>";

    return text;
}
