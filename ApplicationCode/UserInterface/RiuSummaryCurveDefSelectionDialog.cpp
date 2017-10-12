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

#include "RiuSummaryCurveDefSelectionWidget.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryCurveDefSelectionDialog::RiuSummaryCurveDefSelectionDialog(QWidget* parent)
    : QDialog(parent)
{
    m_addrSelWidget = new RiuSummaryCurveDefSelectionWidget(this);
    QWidget* addrWidget = m_addrSelWidget->getOrCreateWidget(this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(addrWidget);

    setWindowTitle("Summary Address Selection");
    resize(1200, 800);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    mainLayout->addWidget(buttonBox);
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
RiuSummaryCurveDefSelection* RiuSummaryCurveDefSelectionDialog::summaryAddressSelection() const
{
    return m_addrSelWidget->summaryAddressSelection();
}

