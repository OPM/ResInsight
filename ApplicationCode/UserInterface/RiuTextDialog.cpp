/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
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

#include "RiuTextDialog.h"
#include "RiuTools.h"

#include "RiaApplication.h"
#include "RiaQDateTimeTools.h"

#include "SummaryPlotCommands/RicAsciiExportSummaryPlotFeature.h"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QMenu>
#include <QTabWidget>

#include <cvfAssert.h>

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
//--------------------------------------------------------------------------------------------------
void RiuQPlainTextEdit::slotExportToFile()
{
    // Get dialog
    RiuShowTabbedPlotDataDialog* dialog = nullptr;
    auto curr = parent();
    while (dialog == nullptr)
    {
        if (!curr) break;
        dialog = dynamic_cast<RiuShowTabbedPlotDataDialog*>(curr);
        if (dialog) break;
        curr = curr->parent();
    }

    if(dialog)
    {
        QString defaultDir = RicAsciiExportSummaryPlotFeature::defaultExportDir();
        auto fileName = RicAsciiExportSummaryPlotFeature::getFileNameFromUserDialog(dialog->description(), defaultDir);
        RicAsciiExportSummaryPlotFeature::exportTextToFile(fileName, this->toPlainText());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
/// 
/// RiuTextDialog
/// 
/// 
//--------------------------------------------------------------------------------------------------
RiuTextDialog::RiuTextDialog(QWidget* parent)
    : QDialog(parent, RiuTools::defaultDialogFlags())
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
void RiuTextDialog::setText(const QString& text)
{
    m_textEdit->setPlainText(text);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuTextDialog::contextMenuEvent(QContextMenuEvent* event)
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
//--------------------------------------------------------------------------------------------------
RiuShowTabbedPlotDataDialog::RiuShowTabbedPlotDataDialog(QWidget* parent /*= nullptr*/)
    : QDialog(parent, RiuTools::defaultDialogFlags())
{
    m_tabWidget = new QTabWidget(this);

    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged(int)));

    for(auto timePeriod : RiaQDateTimeTools::dateTimePeriods())
    {
        QString tabTitle =
            timePeriod == DateTimePeriod::NONE ? "No Resampling" :
            QString("Plot Data, %1").arg(RiaQDateTimeTools::dateTimePeriodName(timePeriod));

        RiuQPlainTextEdit* textEdit = new RiuQPlainTextEdit();
        textEdit->setReadOnly(true);
        textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

        QFont font("Courier", 8);
        textEdit->setFont(font);
        textEdit->setContextMenuPolicy(Qt::NoContextMenu);

        auto fontWidth = QFontMetrics(font).width("m");
        textEdit->setTabStopWidth(fontWidth * 9);

        m_tabWidget->addTab(textEdit, tabTitle);
    }

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(m_tabWidget);
    setLayout(layout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuShowTabbedPlotDataDialog::setDescription(const QString& description)
{
    m_description = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuShowTabbedPlotDataDialog::description() const
{
    if (m_description.isEmpty()) return "Plot Data";
    return m_description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuShowTabbedPlotDataDialog::setTextProvider(std::function<QString(DateTimePeriod)> textProvider)
{
    m_textProvider = textProvider;

    updateText();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuQPlainTextEdit * RiuShowTabbedPlotDataDialog::currentTextEdit() const
{
    return dynamic_cast<RiuQPlainTextEdit*>(m_tabWidget->currentWidget());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
DateTimePeriod RiuShowTabbedPlotDataDialog::indexToPeriod(int index)
{
    auto currTabTitle = m_tabWidget->tabText(index);
    if (currTabTitle.contains(RiaQDateTimeTools::TIMESPAN_DAY_NAME)) return DateTimePeriod::DAY;
    if (currTabTitle.contains(RiaQDateTimeTools::TIMESPAN_WEEK_NAME)) return DateTimePeriod::WEEK;
    if (currTabTitle.contains(RiaQDateTimeTools::TIMESPAN_MONTH_NAME)) return DateTimePeriod::MONTH;
    if (currTabTitle.contains(RiaQDateTimeTools::TIMESPAN_QUARTER_NAME)) return DateTimePeriod::QUARTER;
    if (currTabTitle.contains(RiaQDateTimeTools::TIMESPAN_HALFYEAR_NAME)) return DateTimePeriod::HALFYEAR;
    if (currTabTitle.contains(RiaQDateTimeTools::TIMESPAN_YEAR_NAME)) return DateTimePeriod::YEAR;
    if (currTabTitle.contains(RiaQDateTimeTools::TIMESPAN_DECADE_NAME)) return DateTimePeriod::DECADE;
    return DateTimePeriod::NONE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuShowTabbedPlotDataDialog::updateText()
{
    auto textEdit = currentTextEdit();
    auto currIndex = m_tabWidget->currentIndex();
    if (textEdit && textEdit->toPlainText().isEmpty() && m_textProvider)
    {
        textEdit->setPlainText(m_textProvider(indexToPeriod(currIndex)));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuShowTabbedPlotDataDialog::slotTabChanged(int index)
{
    updateText();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuShowTabbedPlotDataDialog::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu;
    RiuQPlainTextEdit* textEdit = dynamic_cast<RiuQPlainTextEdit*>(m_tabWidget->currentWidget());

    {

        QAction* actionToSetup = new QAction(this);

        actionToSetup->setText("Copy");
        actionToSetup->setIcon(QIcon(":/Copy.png"));
        actionToSetup->setShortcuts(QKeySequence::Copy);

        connect(actionToSetup, SIGNAL(triggered()), textEdit, SLOT(slotCopyContentToClipboard()));

        menu.addAction(actionToSetup);
    }

    {
        QAction* actionToSetup = new QAction(this);

        actionToSetup->setText("Select All");
        actionToSetup->setShortcuts(QKeySequence::SelectAll);

        connect(actionToSetup, SIGNAL(triggered()), textEdit, SLOT(slotSelectAll()));

        menu.addAction(actionToSetup);
    }

    {
        QAction* actionToSetup = new QAction(this);

        actionToSetup->setText("Export to File...");
        //actionToSetup->setShortcuts(QKeySequence::);

        connect(actionToSetup, SIGNAL(triggered()), textEdit, SLOT(slotExportToFile()));

        menu.addAction(actionToSetup);
    }

    menu.exec(event->globalPos());
}
