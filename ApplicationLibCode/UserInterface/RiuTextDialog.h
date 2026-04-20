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

#include <QDialog>
#include <QPlainTextEdit>

#include <memory>
#include <vector>

class QTabWidget;
class RimTabbedTextProvider;

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiuTabbedTextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RiuTabbedTextDialog( std::unique_ptr<RimTabbedTextProvider> textProvider, QWidget* parent = nullptr );
    ~RiuTabbedTextDialog() override;

    QString description() const;
    void    redrawText();

signals:
    void exportToFileRequested( const QString& title, const QString& text );

private:
    RiuQPlainTextEdit* currentTextEdit() const;
    void               updateTabText();

    QTabWidget*                            m_tabWidget;
    std::unique_ptr<RimTabbedTextProvider> m_textProvider;
    std::vector<QString>                   m_tabTexts;

private slots:
    void slotTabChanged( int index );
    void slotExportToFile();

protected:
    void contextMenuEvent( QContextMenuEvent* ) override;
};
