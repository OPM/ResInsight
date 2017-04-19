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

#include "cafCmdFeature.h"

#include <QDialog>
#include <QPlainTextEdit>


class RiuQPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit RiuQPlainTextEdit(QWidget *parent = 0) : QPlainTextEdit(parent) {}

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;

private slots:
    void slotCopyContentToClipboard();
    void slotSelectAll();

};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RicTextWidget : public QDialog
{
    Q_OBJECT

public:
    explicit RicTextWidget(QWidget* parent = 0);

    void setText(const QString& text);

private:
    RiuQPlainTextEdit* m_textEdit;
protected:
    virtual void contextMenuEvent(QContextMenuEvent *) override;

};


//==================================================================================================
/// 
//==================================================================================================
class RicShowPlotDataFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // Overrides
    virtual bool isCommandEnabled();
    virtual void onActionTriggered( bool isChecked );
    virtual void setupActionLook( QAction* actionToSetup );

public:
    static void showTextWindow(const QString& title, const QString& text);
};


