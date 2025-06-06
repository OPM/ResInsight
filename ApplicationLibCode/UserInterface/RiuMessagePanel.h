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

#pragma once

#include "RiaLogging.h"

#include <QPointer>
#include <QWidget>

class QPlainTextEdit;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuMessagePanel : public QWidget
{
    Q_OBJECT

public:
    explicit RiuMessagePanel( QWidget* parent );

    void  addMessage( RILogLevel messageLevel, const QString& msg );
    QSize sizeHint() const override;

public slots:
    void slotClearMessages();

private slots:
    void slotShowContextMenu( const QPoint& pos );

private:
    QPointer<QPlainTextEdit> m_textEdit;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RiuMessagePanelLogger : public RiaLogger
{
public:
    explicit RiuMessagePanelLogger();

    void addMessagePanel( RiuMessagePanel* messagePanel );

    int  level() const override;
    void setLevel( int logLevel ) override;

    void error( const char* message ) override;
    void warning( const char* message ) override;
    void info( const char* message ) override;
    void debug( const char* message ) override;

private:
    void writeToMessagePanel( RILogLevel messageLevel, const char* message );

private:
    std::vector<QPointer<RiuMessagePanel>> m_messagePanels;
    int                                    m_logLevel;
};
