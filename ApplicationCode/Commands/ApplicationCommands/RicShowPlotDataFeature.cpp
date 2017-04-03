/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicShowPlotDataFeature.h"

#include "RiaApplication.h"

#include "RimSummaryPlot.h"
#include "RimWellLogPlot.h"

#include "RiuMainPlotWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QBoxLayout>
#include <QClipboard>
#include <QMenu>


CAF_CMD_SOURCE_INIT(RicShowPlotDataFeature, "RicShowPlotDataFeature");

//--------------------------------------------------------------------------------------------------
/// 
/// 
///  RiuQPlainTextEdit
/// 
/// 
//--------------------------------------------------------------------------------------------------
void RiuQPlainTextEdit::keyPressEvent(QKeyEvent *e)
{
    if ( e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier )
    {
        slotCopyContentToClipboard();
        e->setAccepted(true);
    }
    else
    {
        QPlainTextEdit::keyPressEvent(e);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuQPlainTextEdit::slotCopyContentToClipboard()
{
    QTextCursor cursor(this->textCursor());

    QString textForClipboard;

    QString selText = cursor.selectedText();
    if (!selText.isEmpty())
    {
        QTextDocument doc;
        doc.setPlainText(selText);

        textForClipboard = doc.toPlainText();
    }

    if (textForClipboard.isEmpty())
    {
        textForClipboard = this->toPlainText();
    }

    if (!textForClipboard.isEmpty())
    {
        QClipboard* clipboard = QApplication::clipboard();
        if (clipboard)
        {
            clipboard->setText(textForClipboard);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuQPlainTextEdit::slotSelectAll()
{
    this->selectAll();
}

//--------------------------------------------------------------------------------------------------
/// 
/// 
/// RicTextWidget
/// 
/// 
//--------------------------------------------------------------------------------------------------
RicTextWidget::RicTextWidget(QWidget* parent) : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    m_textEdit = new RiuQPlainTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

    QFont font("Courier", 8);
    m_textEdit->setFont(font);

    m_textEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(m_textEdit);
    setLayout(layout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTextWidget::setText(const QString& text)
{
    m_textEdit->setPlainText(text);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTextWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu;

    {
        QAction* actionToSetup = new QAction(this);

        actionToSetup->setText("Copy");
        actionToSetup->setIcon(QIcon(":/Copy.png"));
        actionToSetup->setShortcuts(QKeySequence::Copy);

        connect(actionToSetup, SIGNAL(triggered()), m_textEdit, SLOT(slotCopyContentToClipboard()));

        menu.addAction(actionToSetup);
    }

    {
        QAction* actionToSetup = new QAction(this);

        actionToSetup->setText("Select All");
        actionToSetup->setShortcuts(QKeySequence::SelectAll);

        connect(actionToSetup, SIGNAL(triggered()), m_textEdit, SLOT(slotSelectAll()));

        menu.addAction(actionToSetup);
    }

    menu.exec(event->globalPos());
}



//--------------------------------------------------------------------------------------------------
///
///
/// RicShowPlotDataFeature
/// 
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowPlotDataFeature::isCommandEnabled()
{
    std::vector<RimSummaryPlot*> selectedSummaryPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedSummaryPlots);
    if (selectedSummaryPlots.size() > 0) return true;

    std::vector<RimWellLogPlot*> wellLogPlots;
    caf::SelectionManager::instance()->objectsByType(&wellLogPlots);
    if (wellLogPlots.size() > 0) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSummaryPlot*> selectedSummaryPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedSummaryPlots);

    std::vector<RimWellLogPlot*> wellLogPlots;
    caf::SelectionManager::instance()->objectsByType(&wellLogPlots);

    if (selectedSummaryPlots.size() == 0 && wellLogPlots.size() == 0)
    {
        CVF_ASSERT(false);

        return;
    }

    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->mainPlotWindow();
    CVF_ASSERT(plotwindow);

    for (RimSummaryPlot* summaryPlot : selectedSummaryPlots)
    {
        QString title = summaryPlot->description();
        QString text = summaryPlot->asciiDataForPlotExport();

        RicShowPlotDataFeature::showTextWindow(title, text);
    }

    for (RimWellLogPlot* wellLogPlot : wellLogPlots)
    {
        QString title = wellLogPlot->description();
        QString text = wellLogPlot->asciiDataForPlotExport();

        RicShowPlotDataFeature::showTextWindow(title, text);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show Plot Data");
    actionToSetup->setIcon(QIcon(":/PlotWindow24x24.png"));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowPlotDataFeature::showTextWindow(const QString& title, const QString& text)
{
    RiuMainPlotWindow* plotwindow = RiaApplication::instance()->mainPlotWindow();
    CVF_ASSERT(plotwindow);

    RicTextWidget* textWiget = new RicTextWidget(plotwindow);
    textWiget->setMinimumSize(400, 600);

    textWiget->setWindowTitle(title);
    textWiget->setText(text);

    textWiget->show();

    plotwindow->addToTemporaryWidgets(textWiget);
}










