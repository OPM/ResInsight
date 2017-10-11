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

#include "RiuSummaryCurveDefSelectionWidget.h"

#include "RiuSummaryCurveDefSelection.h"
#include "RiuSummaryCurveDefinitionKeywords.h"

#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiGroup.h"

#include "QMinimizePanel.h"

#include <QBoxLayout>
#include <QFrame>
#include <QSplitter>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryCurveDefSelectionWidget::RiuSummaryCurveDefSelectionWidget(QWidget* parent)
{
    m_summaryAddressSelection = std::unique_ptr<RiuSummaryCurveDefSelection>(new RiuSummaryCurveDefSelection());

    this->setPdmObject(m_summaryAddressSelection.get());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryCurveDefSelectionWidget::~RiuSummaryCurveDefSelectionWidget()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryCurveDefSelection* RiuSummaryCurveDefSelectionWidget::summaryAddressSelection() const
{
    return m_summaryAddressSelection.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelectionWidget::recursivelyConfigureAndUpdateTopLevelUiItems(const std::vector<caf::PdmUiItem *>& topLevelUiItems, const QString& uiConfigName)
{
    if (!m_firstRowLeftLayout || !m_firstRowRightLayout) return;

    for (size_t i = 0; i < topLevelUiItems.size(); ++i)
    {
        if (topLevelUiItems[i]->isUiHidden(uiConfigName)) continue;

        if (topLevelUiItems[i]->isUiGroup())
        {
            caf::PdmUiGroup* group = static_cast<caf::PdmUiGroup*>(topLevelUiItems[i]);
            auto groupBox = createGroupBoxWithContent(group, uiConfigName);

            bool isSources = group->keyword() == RiuSummaryCurveDefinitionKeywords::sources();
            bool isSummaryTypes = group->keyword() == RiuSummaryCurveDefinitionKeywords::summaryTypes();
            bool isSummaries = group->keyword() == RiuSummaryCurveDefinitionKeywords::summaries();
            bool isDynamicGroup = !isSources && !isSummaryTypes && !isSummaries;
            bool leftColumn = isSources || isSummaryTypes;

            if (isSummaryTypes || isDynamicGroup)
            {
                groupBox->setFixedWidth(170);
            }

            if(leftColumn)
                m_firstRowLeftLayout->addWidget(groupBox);
            else
                m_firstRowRightLayout->addWidget(groupBox);

            // Add group boxes until summaries are detected

            if (group->keyword() == RiuSummaryCurveDefinitionKeywords::summaries())
                break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RiuSummaryCurveDefSelectionWidget::createWidget(QWidget* parent)
{
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
    m_firstRowRightLayout = new QHBoxLayout;
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

    return widget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelectionWidget::configureAndUpdateFields(int widgetStartIndex, 
                                                                QBoxLayout* layout,
                                                                const std::vector<caf::PdmUiItem *>& uiItems,
                                                                const QString& uiConfigName)
{
    int currentWidgetIndex = widgetStartIndex;

    for (size_t i = 0; i < uiItems.size(); ++i)
    {
        if (uiItems[i]->isUiHidden(uiConfigName)) continue;
        if (uiItems[i]->isUiGroup()) continue;

        {
            caf::PdmUiFieldHandle* field = dynamic_cast<caf::PdmUiFieldHandle*>(uiItems[i]);

            caf::PdmUiFieldEditorHandle* fieldEditor = findOrCreateFieldEditor(this->widget(), field, uiConfigName);

            if (fieldEditor)
            {
                fieldEditor->setField(field);

                // Place the widget(s) into the correct parent and layout
                QWidget* fieldCombinedWidget = fieldEditor->combinedWidget();

                if (fieldCombinedWidget)
                {
                    fieldCombinedWidget->setParent(this->widget());
                    layout->insertWidget(currentWidgetIndex++, fieldCombinedWidget);
                }
                else
                {
                    caf::PdmUiItemInfo::LabelPosType labelPos = field->uiLabelPosition(uiConfigName);

                    QWidget* fieldEditorWidget = fieldEditor->editorWidget();

                    if (labelPos != caf::PdmUiItemInfo::HIDDEN)
                    {
                        QWidget* fieldLabelWidget = fieldEditor->labelWidget();
                        if (fieldLabelWidget)
                        {
                            fieldLabelWidget->setParent(this->widget());

                            layout->insertWidget(currentWidgetIndex++, fieldLabelWidget);

                            fieldLabelWidget->show();
                        }
                    }
                    else
                    {
                        QWidget* fieldLabelWidget = fieldEditor->labelWidget();
                        if (fieldLabelWidget) fieldLabelWidget->hide();
                    }

                    if (fieldEditorWidget)
                    {
                        fieldEditorWidget->setParent(this->widget()); // To make sure this widget has the current group box as parent.

                        layout->insertWidget(currentWidgetIndex++, fieldEditorWidget);
                    }
                }

                fieldEditor->updateUi(uiConfigName);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMinimizePanel* RiuSummaryCurveDefSelectionWidget::createGroupBoxWithContent(caf::PdmUiGroup* group,
                                                                            const QString& uiConfigName)
{
    QMinimizePanel* groupBox = findOrCreateGroupBox(this->widget(), group, uiConfigName);

    const std::vector<caf::PdmUiItem*>& groupChildren = group->uiItems();
    recursivelyConfigureAndUpdateUiItemsInGridLayoutColumn(groupChildren, groupBox->contentFrame(), uiConfigName);
    return groupBox;
}
