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

#include "RicSummaryCurveCreatorDialog.h"

#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorSplitterUi.h"

#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorDialog::RicSummaryCurveCreatorDialog(QWidget* parent, RicSummaryCurveCreator* summaryCurveCreator)
    : QDialog(parent)
{
    m_curveCreatorSplitterUi = new RicSummaryCurveCreatorSplitterUi(this);

    QWidget* propertyWidget = m_curveCreatorSplitterUi->getOrCreateWidget(this);

    QVBoxLayout* dummy = new QVBoxLayout(this);
    dummy->setContentsMargins(0, 0, 0, 0);
    dummy->addWidget(propertyWidget);

    m_curveCreatorSplitterUi->setPdmObject(summaryCurveCreator);
    m_curveCreatorSplitterUi->updateUi();

    connect(m_curveCreatorSplitterUi, SIGNAL(signalCloseButtonPressed()), this, SLOT(accept()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorDialog::~RicSummaryCurveCreatorDialog()
{

}
