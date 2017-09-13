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

#include "cafPdmUiTreeView.h"

#include "QMinimizePanel.h"

#include <QBoxLayout>
#include <QTreeView>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorUi::RicSummaryCurveCreatorUi(QWidget* parent, RicSummaryCurveCreator* summaryCurveCreator)
    : QDialog(parent),
    m_summaryCurveCreator(summaryCurveCreator)
{
    m_customObjectEditor = new RiuCustomObjectEditor;
    m_customObjectEditor->defineGridLayout(2, 4);

    QWidget* propertyWidget = m_customObjectEditor->getOrCreateWidget(this);

    QVBoxLayout* dummy = new QVBoxLayout(this);
    dummy->setContentsMargins(0, 0, 0, 0);
    dummy->addWidget(propertyWidget);

    m_customObjectEditor->setPdmObject(m_summaryCurveCreator);
    m_customObjectEditor->updateUi();
    m_customObjectEditor->addWidget(m_summaryCurveCreator->previewPlot()->createViewWidget(this), 1, 1, 1, 3);
    m_summaryCurveCreator->previewPlot()->viewWidget()->setFixedHeight(400);

    QMinimizePanel* curvesPanel = new QMinimizePanel(this);
    curvesPanel->setTitle("Curves");
    QVBoxLayout* curvesLayout = new QVBoxLayout(curvesPanel->contentFrame());

    caf::PdmUiTreeView* curveTreeView = new caf::PdmUiTreeView(curvesPanel->contentFrame());
    curvesLayout->addWidget(curveTreeView);
    curveTreeView->setPdmItem(m_summaryCurveCreator->previewPlot());
    curveTreeView->treeView()->setHeaderHidden(true);

    m_customObjectEditor->addWidget(curvesPanel, 1, 0, 1, 1);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorUi::~RicSummaryCurveCreatorUi()
{

}
