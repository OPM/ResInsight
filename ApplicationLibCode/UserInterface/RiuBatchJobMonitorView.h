/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include <QStringList>
#include <QWidget>

#include "Hpc/RiaHpcTools.h"

class QTableWidget;
class QCheckBox;
class QLabel;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiuBatchJobMonitorView : public QWidget
{
    Q_OBJECT

public:
    RiuBatchJobMonitorView( QWidget* parent );
    ~RiuBatchJobMonitorView() override;

    void rebuildGui();

protected:
    void showEvent( QShowEvent* event ) override;
    void hideEvent( QHideEvent* event ) override;

    void stopJob( QString jobId );

private slots:
    void slotUpdateView( const std::map<QString, RiaHpcTools::BatchJobInformation> information );
    void toggleUpdates();
    void showContextMenu( const QPoint& pos );

private:
    QCheckBox*    m_checkBoxAutoUpdate;
    QCheckBox*    m_onlyResInsightJobs;
    QTableWidget* m_jobView;
    QLabel*       m_infoLabel;
};
