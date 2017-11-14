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

#include "RicSummaryCurveCalculator.h"
#include "RicSummaryCurveCalculatorEditor.h"

#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculation.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCalculatorDialog::RicSummaryCurveCalculatorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Summary Curve Calculator");
    resize(1200, 800);

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
void RicSummaryCurveCalculatorDialog::setCalculationAndUpdateUi(RimSummaryCalculation* calculation)
{
    m_summaryCalcEditor->calculator()->setCurrentCalculation(calculation);

    m_summaryCalcEditor->updateUi();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorDialog::slotTryCloseDialog()
{
    RimSummaryCalculationCollection* calculationCollection = RicSummaryCurveCalculator::calculationCollection();

    if (dirtyCount() > 0)
    {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);

        QString questionText = QString("Detected calculation expression text modifications.");

        msgBox.setText(questionText);
        msgBox.setInformativeText("Do you want to trigger calculation?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        int ret = msgBox.exec();
        if (ret == QMessageBox::No)
        {
            reject();
        }
        else if (ret == QMessageBox::Yes)
        {
            for (auto c : calculationCollection->calculations())
            {
                if (c->isDirty())
                {
                    c->calculate();
                    c->updateDependentCurvesAndPlots();
                }
            }

            if (dirtyCount() > 0)
            {
                return;
            }
        }
        else
        {
            return;
        }
    }

    accept();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorDialog::setUp()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_summaryCalcEditor = std::unique_ptr<RicSummaryCurveCalculatorEditor>(new RicSummaryCurveCalculatorEditor());
    mainLayout->addWidget(m_summaryCalcEditor->getOrCreateWidget(this));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(slotTryCloseDialog()));

    mainLayout->addWidget(buttonBox);
    
    m_summaryCalcEditor->updateUi();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RicSummaryCurveCalculatorDialog::dirtyCount() const
{
    size_t count = 0;

    RimSummaryCalculationCollection* calculationCollection = RicSummaryCurveCalculator::calculationCollection();
    for (auto c : calculationCollection->calculations())
    {
        if (c->isDirty())
        {
            count++;
        }
    }

    return count;
}
