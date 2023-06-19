/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimEnsembleCurveInfoTextProvider.h"

#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"

#include "RiuPlotCurve.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCurveInfoTextProvider::curveInfoText( RiuPlotCurve* riuCurve ) const
{
    if ( riuCurve )
    {
        RimSummaryCurve* sumCurve = dynamic_cast<RimSummaryCurve*>( riuCurve->ownerRimCurve() );

        if ( sumCurve && sumCurve->summaryCaseY() )
        {
            auto caseNameAndAdrText = sumCurve->summaryCaseY()->displayCaseName();

            auto itemText = sumCurve->summaryAddressY().itemUiText();
            if ( !itemText.empty() )
            {
                caseNameAndAdrText += ", " + QString::fromStdString( itemText );
            }

            return caseNameAndAdrText;
        }
    }

    return {};
}
