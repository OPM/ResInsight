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
#include <QPointer>

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
    explicit RiuQPlainTextEdit( QWidget* parent = nullptr )
        : QPlainTextEdit( parent )
    {
    }

protected:
    void keyPressEvent( QKeyEvent* e ) override;

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
    explicit RiuTextDialog( QWidget* parent = nullptr );

    void setText( const QString& text );

private:
    RiuQPlainTextEdit* m_textEdit;

protected:
    void contextMenuEvent( QContextMenuEvent* ) override;
};

class RiuTabbedTextProvider : public QObject
{
    Q_OBJECT

public:
    virtual bool    isValid() const                = 0;
    virtual QString description() const            = 0;
    virtual QString tabTitle( int tabIndex ) const = 0;
    virtual QString tabText( int tabIndex ) const  = 0;
    virtual int     tabCount() const               = 0;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiuTabbedTextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RiuTabbedTextDialog( RiuTabbedTextProvider* textProvider, QWidget* parent = nullptr );

    QString description() const;
    void    redrawText();

private:
    RiuQPlainTextEdit* currentTextEdit() const;
    void               updateTabText();

    QTabWidget*                     m_tabWidget;
    QPointer<RiuTabbedTextProvider> m_textProvider;
    std::vector<QString>            m_tabTexts;

private slots:
    void slotTabChanged( int index );

protected:
    void contextMenuEvent( QContextMenuEvent* ) override;
};
