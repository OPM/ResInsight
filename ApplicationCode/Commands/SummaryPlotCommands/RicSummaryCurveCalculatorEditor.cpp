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

#include "RicSummaryCurveCalculatorEditor.h"

#include "RicSummaryCurveCalculator.h"
#include "RimSummaryCalculation.h"

#include "cafPdmUiTableView.h"

#include "QMinimizePanel.h"

#include <QBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSplitter>
#include <QTableView>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCalculatorEditor::RicSummaryCurveCalculatorEditor()
{
    m_calculator = std::unique_ptr<RicSummaryCurveCalculator>(new RicSummaryCurveCalculator);

    this->setPdmObject(m_calculator.get());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCalculatorEditor::~RicSummaryCurveCalculatorEditor()
{
    if (m_pdmTableView)
    {
        m_pdmTableView->setListField(nullptr);

        delete m_pdmTableView;
        m_pdmTableView = nullptr;
    }

    this->setPdmObject(nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorEditor::recursivelyConfigureAndUpdateTopLevelUiItems(const std::vector<caf::PdmUiItem *>& topLevelUiItems, const QString& uiConfigName)
{
    if (!m_firstRowLeftLayout || !m_firstRowRightLayout) return;

    int layoutItemIndex = 0;
    for (size_t i = 0; i < topLevelUiItems.size(); ++i)
    {
        if (topLevelUiItems[i]->isUiHidden(uiConfigName)) continue;

        if (topLevelUiItems[i]->isUiGroup())
        {
            caf::PdmUiGroup* group = static_cast<caf::PdmUiGroup*>(topLevelUiItems[i]);
            auto groupBox = updateGroupBoxWithContent(group, uiConfigName);

            if (group->keyword() == RicSummaryCurveCalculator::calculatedSummariesGroupName())
            {
                m_firstRowLeftLayout->addWidget(groupBox);
            }
            else if (group->keyword() == RicSummaryCurveCalculator::calulationGroupName())
            {
                m_firstRowRightLayout->insertWidget(layoutItemIndex++, groupBox);
            }
        }
    }

    m_firstRowRightLayout->insertLayout(layoutItemIndex++, m_parseButtonLayout);

    if (m_calculator->currentCalculation())
    {
        m_pdmTableView->setListField(m_calculator->currentCalculation()->variables());
    }
    else
        m_pdmTableView->setListField(nullptr);

    m_firstRowRightLayout->insertWidget(layoutItemIndex++, m_pdmTableView);
    m_firstRowRightLayout->insertLayout(layoutItemIndex++, m_calculateButtonLayout);

    m_pdmTableView->tableView()->resizeColumnsToContents();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RicSummaryCurveCalculatorEditor::createWidget(QWidget* parent)
{
    m_pdmTableView = new caf::PdmUiTableView(parent);
    m_pdmTableView->tableView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pdmTableView->enableHeaderText(false);

    QHeaderView* verticalHeader = m_pdmTableView->tableView()->verticalHeader();
    verticalHeader->setResizeMode(QHeaderView::Interactive);

    m_pdmTableView->tableView()->resizeColumnsToContents();

    QWidget* widget = new QWidget(parent);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(5, 5, 5, 5);
    widget->setLayout(mainLayout);

    QFrame* firstRowFrame = new QFrame(widget);
    QHBoxLayout* firstRowLayout = new QHBoxLayout;
    firstRowLayout->setContentsMargins(0, 0, 0, 0);
    firstRowFrame->setLayout(firstRowLayout);

    QFrame* firstRowLeftFrame = new QFrame(widget);
    m_firstRowLeftLayout = new QHBoxLayout;
    m_firstRowLeftLayout->setContentsMargins(0, 0, 0, 0);
    firstRowLeftFrame->setLayout(m_firstRowLeftLayout);

    QFrame* firstRowRightFrame = new QFrame(widget);
    m_firstRowRightLayout = new QVBoxLayout;
    m_firstRowRightLayout->setContentsMargins(0, 0, 0, 0);
    firstRowRightFrame->setLayout(m_firstRowRightLayout);

    QSplitter* rowSplitter = new QSplitter(Qt::Horizontal);
    rowSplitter->setContentsMargins(0, 0, 0, 0);
    rowSplitter->setHandleWidth(6);
    rowSplitter->setStyleSheet("QSplitter::handle { image: url(:/SplitterV.png); }");
    rowSplitter->insertWidget(0, firstRowLeftFrame);
    rowSplitter->insertWidget(1, firstRowRightFrame);
    rowSplitter->setSizes(QList<int>() << 1 << 1);
    firstRowLayout->addWidget(rowSplitter);

    mainLayout->addWidget(rowSplitter);

    {
        QPushButton* pushButton = new QPushButton("Parse Expression");
        connect(pushButton, SIGNAL(clicked()), this, SLOT(slotParseExpression()));
        m_parseButtonLayout = new QHBoxLayout;
        m_parseButtonLayout->addStretch(10);
        m_parseButtonLayout->addWidget(pushButton);
    }

    {
        QPushButton* pushButton = new QPushButton("Calculate");
        connect(pushButton, SIGNAL(clicked()), this, SLOT(slotCalculate()));
        m_calculateButtonLayout = new QHBoxLayout;
        m_calculateButtonLayout->addStretch(10);
        m_calculateButtonLayout->addWidget(pushButton);
    }

    return widget;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMinimizePanel* RicSummaryCurveCalculatorEditor::updateGroupBoxWithContent(caf::PdmUiGroup* group,
                                                                            const QString& uiConfigName)
{
    QMinimizePanel* groupBox = findOrCreateGroupBox(this->widget(), group, uiConfigName);

    const std::vector<caf::PdmUiItem*>& groupChildren = group->uiItems();
    recursivelyConfigureAndUpdateUiItemsInGridLayoutColumn(groupChildren, groupBox->contentFrame(), uiConfigName);
    return groupBox;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorEditor::slotCalculate()
{
    m_calculator->calculate();

    m_calculator->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorEditor::slotParseExpression()
{
    m_calculator->parseExpression();

    m_calculator->updateConnectedEditors();
}

