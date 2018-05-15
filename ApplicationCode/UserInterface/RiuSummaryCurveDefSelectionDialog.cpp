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

#include "RiuSummaryCurveDefSelectionDialog.h"

#include "RiaSummaryCurveDefinition.h"

#include "RimSummaryCaseCollection.h"

#include "RiuSummaryCurveDefSelection.h"
#include "RiuSummaryCurveDefSelectionEditor.h"
#include "RiuTools.h"

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryCurveDefSelectionDialog::RiuSummaryCurveDefSelectionDialog(QWidget* parent)
    : QDialog(parent, RiuTools::defaultDialogFlags())
{
    m_addrSelWidget = std::unique_ptr<RiuSummaryCurveDefSelectionEditor>(new RiuSummaryCurveDefSelectionEditor());
    QWidget* addrWidget = m_addrSelWidget->getOrCreateWidget(this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(addrWidget);

    setWindowTitle("Summary Address Selection");
    resize(1200, 800);

    m_label = new QLabel("", this);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout* labelLayout = new QHBoxLayout;
    labelLayout->addStretch(1);
    labelLayout->addWidget(m_label);
    labelLayout->addWidget(buttonBox);

    mainLayout->addLayout(labelLayout);

    m_addrSelWidget->summaryAddressSelection()->setFieldChangedHandler([this]() { this->updateLabel(); });

    updateLabel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryCurveDefSelectionDialog::~RiuSummaryCurveDefSelectionDialog()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelectionDialog::setCaseAndAddress(RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& address)
{
    if (summaryCase)
    {
        std::vector<RiaSummaryCurveDefinition> curveDefs;
        curveDefs.push_back(RiaSummaryCurveDefinition(summaryCase, address));
        summaryAddressSelection()->setSelectedCurveDefinitions(curveDefs);
    }

    summaryAddressSelection()->updateConnectedEditors();
    updateLabel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelectionDialog::setEnsembleAndAddress(RimSummaryCaseCollection* ensemble, const RifEclipseSummaryAddress& address)
{
    if (ensemble)
    {
        std::vector<RiaSummaryCurveDefinition> curveDefs;
        curveDefs.push_back(RiaSummaryCurveDefinition(nullptr, address, ensemble));
        summaryAddressSelection()->setSelectedCurveDefinitions(curveDefs);
    }

    summaryAddressSelection()->updateConnectedEditors();
    updateLabel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RiuSummaryCurveDefSelectionDialog::curveSelection() const
{
    return summaryAddressSelection()->allCurveDefinitionsFromSelection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelectionDialog::hideEnsembles()
{
    summaryAddressSelection()->hideEnsembles(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelectionDialog::hideSummaryCases()
{
    summaryAddressSelection()->hideSummaryCases(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryCurveDefSelection* RiuSummaryCurveDefSelectionDialog::summaryAddressSelection() const
{
    return m_addrSelWidget->summaryAddressSelection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelectionDialog::updateLabel()
{
    QString curveAddressText;
    std::vector<RiaSummaryCurveDefinition> sumCasePairs = this->summaryAddressSelection()->allCurveDefinitionsFromSelection();
    if (sumCasePairs.size() == 1)
    {
        curveAddressText = sumCasePairs.front().curveDefinitionText();
    }

    if (curveAddressText.isEmpty())
    {
        curveAddressText = "<None>";
    }

    QString txt = "Selected Address : ";
    txt += curveAddressText;

    m_label->setText(txt);
}

