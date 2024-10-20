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

#pragma once

#include "RicUserDefinedCalculatorDialog.h"

#include <memory>

class RicCalculatorWidgetCreator;

//==================================================================================================
///
///
//==================================================================================================
class RicSummaryCurveCalculatorDialog : public RicUserDefinedCalculatorDialog
{
    Q_OBJECT

public:
    RicSummaryCurveCalculatorDialog( QWidget* parent );
    ~RicSummaryCurveCalculatorDialog() override;

    void                                 setCalculationAndUpdateUi( RimUserDefinedCalculation* calculation ) override;
    QWidget*                             getCalculatorWidget() override;
    void                                 updateUi() override;
    RimUserDefinedCalculationCollection* calculationCollection() const override;

private:
    std::unique_ptr<RicCalculatorWidgetCreator> m_summaryCalcEditor;
};
