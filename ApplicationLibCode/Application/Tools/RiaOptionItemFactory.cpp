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

#include "RiaOptionItemFactory.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOptionItemFactory::appendOptionItemFromViewNameAndCaseName( Rim3dView*                     view,
                                                                    QList<caf::PdmOptionItemInfo>* optionItems )
{
    if ( !view || !optionItems ) return;

    QString displayName = view->autoName();

    caf::IconProvider iconProvider = view->uiCapability()->uiIconProvider();

    optionItems->push_back( caf::PdmOptionItemInfo( displayName, view, false, iconProvider ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaOptionItemFactory::appendOptionItemsForEnsembleCurveSets( QList<caf::PdmOptionItemInfo>* options )
{
    options->push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

    RimMainPlotCollection*            mainPlotColl = RimProject::current()->mainPlotCollection();
    std::vector<RimEnsembleCurveSet*> ensembleCurveSets;
    mainPlotColl->descendantsOfType( ensembleCurveSets );
    for ( auto ensembleCurveSet : ensembleCurveSets )
    {
        options->push_back( caf::PdmOptionItemInfo( ensembleCurveSet->name(), ensembleCurveSet ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmOptionItemInfo
    RiaOptionItemFactory::optionItemFromSummaryType( RifEclipseSummaryAddress::SummaryVarCategory summaryType )
{
    auto uiText = caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::uiText( summaryType );

    // Use icons from https://github.com/equinor/webviz-subsurface-components

    QString iconText;

    switch ( summaryType )
    {
        case RifEclipseSummaryAddress::SUMMARY_INVALID:
            iconText = ":/summary/components/images/invalid.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_FIELD:
            iconText = ":/summary/components/images/field.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_AQUIFER:
            iconText = ":/summary/components/images/aquifer.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_NETWORK:
            iconText = ":/summary/components/images/network.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_MISC:
            iconText = ":/summary/components/images/misc.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_REGION:
            iconText = ":/summary/components/images/region.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
            iconText = ":/summary/components/images/region-region.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
            iconText = ":/summary/components/images/group.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_WELL:
            iconText = ":/summary/components/images/well.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
            iconText = ":/summary/components/images/well-completion.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
            iconText = ":/summary/components/images/well.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
            iconText = ":/summary/components/images/well-completion.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
            iconText = ":/summary/components/images/segment.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK:
            iconText = ":/summary/components/images/block.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
            iconText = ":/summary/components/images/block.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_CALCULATED:
            iconText = ":/summary/components/images/calculated.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_IMPORTED:
            iconText = ":/summary/components/images/others.svg";
            break;
        case RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS:
            break;
    }

    if ( iconText.isEmpty() )
    {
        return caf::PdmOptionItemInfo( uiText, summaryType );
    }

    caf::IconProvider iconProvider( iconText );
    return caf::PdmOptionItemInfo( uiText, summaryType, false, iconProvider );
}
