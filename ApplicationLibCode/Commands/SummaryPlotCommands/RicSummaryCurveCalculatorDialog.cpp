/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicSummaryCurveCalculatorDialog.h"

#include <memory>

#include "RicCalculatorWidgetCreator.h"
#include "RicSummaryCurveCalculatorUi.h"

#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCalculatorDialog::RicSummaryCurveCalculatorDialog( QWidget* parent )
    : RicUserDefinedCalculatorDialog( parent, "Summary Curve Calculator" )
{
    setUp();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCalculatorDialog::~RicSummaryCurveCalculatorDialog()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorDialog::setCalculationAndUpdateUi( RimUserDefinedCalculation* calculation )
{
    CAF_ASSERT( m_summaryCalcEditor );
    m_summaryCalcEditor->calculator()->setCurrentCalculation( dynamic_cast<RimSummaryCalculation*>( calculation ) );
    updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorDialog::updateUi()
{
    CAF_ASSERT( m_summaryCalcEditor );
    m_summaryCalcEditor->updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculationCollection* RicSummaryCurveCalculatorDialog::calculationCollection() const
{
    CAF_ASSERT( m_summaryCalcEditor );
    return m_summaryCalcEditor->calculator()->calculationCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RicSummaryCurveCalculatorDialog::getCalculatorWidget()
{
    if ( !m_summaryCalcEditor )
    {
        m_summaryCalcEditor = std::make_unique<RicCalculatorWidgetCreator>( std::make_unique<RicSummaryCurveCalculatorUi>() );
    }

    return m_summaryCalcEditor->getOrCreateWidget( this );
}
