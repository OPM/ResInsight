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

#pragma once

#include "RiaQDateTimeTools.h"

#include <QDialog>
#include <QPlainTextEdit>

#include <functional>

class QTabWidget;
class RimSummaryPlot;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RiuQPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit RiuQPlainTextEdit(QWidget *parent = nullptr) : QPlainTextEdit(parent) {}

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;

private slots:
    void slotCopyContentToClipboard();
    void slotSelectAll();
    void slotExportToFile();
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RiuTextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RiuTextDialog(QWidget* parent = nullptr);

    void setText(const QString& text);

private:
    RiuQPlainTextEdit* m_textEdit;
protected:
    virtual void contextMenuEvent(QContextMenuEvent *) override;

};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RiuShowTabbedPlotDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RiuShowTabbedPlotDataDialog(QWidget* parent = nullptr);

    void setDescription(const QString& description);
    QString description() const;
    void setTextProvider(std::function<QString (DateTimePeriod)> textProvider);

private:
    RiuQPlainTextEdit * currentTextEdit() const;
    DateTimePeriod      indexToPeriod(int index);
    void                updateText();

    QTabWidget* m_tabWidget;
    QString     m_description;
    std::function<QString(DateTimePeriod)> m_textProvider;

private slots:
    void slotTabChanged(int index);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *) override;
};

