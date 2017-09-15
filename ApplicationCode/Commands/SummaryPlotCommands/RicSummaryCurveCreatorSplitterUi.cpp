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

#include "RicSummaryCurveCreatorSplitterUi.h"

#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorUiKeywords.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiTreeView.h"

#include "QMinimizePanel.h"

#include <QBoxLayout>
#include <QSplitter>
#include <QFrame>
#include <QTreeView>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorSplitterUi::RicSummaryCurveCreatorSplitterUi(QWidget* parent)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorSplitterUi::~RicSummaryCurveCreatorSplitterUi()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreatorSplitterUi::recursivelyConfigureAndUpdateTopLevelUiItems(const std::vector<caf::PdmUiItem *>& topLevelUiItems, const QString& uiConfigName)
{
    if (!m_layout) return;

    int splitterPositionIndex = 0;
    for (size_t i = 0; i < topLevelUiItems.size(); ++i)
    {
        if (topLevelUiItems[i]->isUiHidden(uiConfigName)) continue;

        if (topLevelUiItems[i]->isUiGroup())
        {
            caf::PdmUiGroup* group = static_cast<caf::PdmUiGroup*>(topLevelUiItems[i]);

            QMinimizePanel* groupBox = findOrCreateGroupBox(this->widget(), group, uiConfigName);

            m_firstRowLayout->addWidget(groupBox);

            const std::vector<caf::PdmUiItem*>& groupChildren = group->uiItems();
            recursivelyConfigureAndUpdateUiItemsInGridLayoutColumn(groupChildren, groupBox->contentFrame(), uiConfigName);

            // Add group boxes until summaries are detected

            if (group->keyword() == RicSummaryCurveCreatorUiKeywords::summaries())
                break;
        }
    }


    {
        caf::PdmUiGroup* group = findGroupByKeyword(topLevelUiItems, RicSummaryCurveCreatorUiKeywords::appearance(), uiConfigName);

        QMinimizePanel* groupBox = findOrCreateGroupBox(this->widget(), group, uiConfigName);

        m_lowerLeftLayout->insertWidget(0, groupBox);

        const std::vector<caf::PdmUiItem*>& groupChildren = group->uiItems();
        recursivelyConfigureAndUpdateUiItemsInGridLayoutColumn(groupChildren, groupBox->contentFrame(), uiConfigName);
    }


    m_lowerLeftLayout->insertWidget(1, getOrCreateCurveTreeWidget());
    
    m_secondRowLayout->insertWidget(1, getOrCreatePlotWidget());

    // NB! Only groups at top level are handled, fields at top level are not added to layout

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RicSummaryCurveCreatorSplitterUi::createWidget(QWidget* parent)
{
    QWidget* widget = new QWidget(parent);

    m_layout = new QVBoxLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(m_layout);

    QFrame* firstRowFrame = new QFrame(widget);
    m_firstRowLayout = new QHBoxLayout;
    firstRowFrame->setLayout(m_firstRowLayout);

    QFrame* secondRowFrame = new QFrame(widget);
    m_secondRowLayout = new QHBoxLayout;
    secondRowFrame->setLayout(m_secondRowLayout);

    m_lowerLeftLayout = new QVBoxLayout;
    m_secondRowLayout->addLayout(m_lowerLeftLayout);

    m_firstColumnSplitter = new QSplitter(Qt::Vertical);
    m_firstColumnSplitter->insertWidget(0, firstRowFrame);
    m_firstColumnSplitter->insertWidget(1, secondRowFrame);

    m_layout->addWidget(m_firstColumnSplitter);

    return widget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* RicSummaryCurveCreatorSplitterUi::findGroupByKeyword(const std::vector<caf::PdmUiItem *>& topLevelUiItems, const QString& keyword, const QString& uiConfigName)
{
    for (auto uiItem : topLevelUiItems)
    {
        if (uiItem->isUiHidden(uiConfigName)) continue;

        if (uiItem->isUiGroup())
        {
            caf::PdmUiGroup* group = static_cast<caf::PdmUiGroup*>(uiItem);
            if (group->keyword() == keyword)
            {
                return group;
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RicSummaryCurveCreatorSplitterUi::getOrCreateCurveTreeWidget()
{
    if (!m_curvesPanel)
    {
        m_curvesPanel = new QMinimizePanel(this->widget());
        m_curvesPanel->setTitle("Curves");
        QVBoxLayout* curvesLayout = new QVBoxLayout(m_curvesPanel->contentFrame());

        caf::PdmUiTreeView* curveTreeView = new caf::PdmUiTreeView(m_curvesPanel->contentFrame());
        curvesLayout->addWidget(curveTreeView);

        RicSummaryCurveCreator* sumCurveCreator = dynamic_cast<RicSummaryCurveCreator*>(this->pdmItem());
        if (sumCurveCreator)
        {
            curveTreeView->setPdmItem(sumCurveCreator->previewPlot());
        }

        curveTreeView->treeView()->setHeaderHidden(true);
    }

    return m_curvesPanel;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RicSummaryCurveCreatorSplitterUi::getOrCreatePlotWidget()
{
    RicSummaryCurveCreator* sumCurveCreator = dynamic_cast<RicSummaryCurveCreator*>(this->pdmItem());
    if (sumCurveCreator)
    {
        // TODO: Rename previewPlot()->createViewWidget to getOrCreateViewWidget
        return sumCurveCreator->previewPlot()->createViewWidget(this->widget());
    }

    return nullptr;
}

