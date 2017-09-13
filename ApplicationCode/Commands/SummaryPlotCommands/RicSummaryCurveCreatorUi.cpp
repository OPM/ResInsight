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

#include "RicSummaryCurveCreatorUi.h"

#include "RicSummaryCurveCreator.h"

#include "RiuCustomObjectEditor.h"

#include <QBoxLayout>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorUi::RicSummaryCurveCreatorUi(QWidget* parent, RicSummaryCurveCreator* summaryCurveCreator)
    : QDialog(parent),
    m_summaryCurveCreator(summaryCurveCreator)
{
    m_customObjectEditor = new RiuCustomObjectEditor;
    m_customObjectEditor->defineGridLayout(4, 2);

    QWidget* propertyWidget = m_customObjectEditor->getOrCreateWidget(this);

    QVBoxLayout* dummy = new QVBoxLayout(this);
    dummy->setContentsMargins(0, 0, 0, 0);
    dummy->addWidget(propertyWidget);

    m_customObjectEditor->setPdmObject(m_summaryCurveCreator);
    m_customObjectEditor->updateUi();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorUi::~RicSummaryCurveCreatorUi()
{

}
