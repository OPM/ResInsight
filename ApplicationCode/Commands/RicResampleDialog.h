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

#include "RiaQDateTimeTools.h"

#include "cafPdmPointer.h"

#include <QDialog>

class QLabel;
class QComboBox;
class QLineEdit;
class QTextEdit;
class QDialogButtonBox;
class QPushButton;
class QMainWindow;
class QListWidget;
class RicResampleDialogResult;
class DateTimePeriodInfo;

//==================================================================================================
///  
//==================================================================================================
class RicResampleDialog : public QDialog
{
    Q_OBJECT

public:
    RicResampleDialog(QWidget* parent);
    ~RicResampleDialog() override;

    static RicResampleDialogResult  openDialog(QWidget *parent = nullptr,
                                                             const QString& caption = QString());

private:
    void            setPeriodOptions(const std::vector<DateTimePeriod>& dateTimePeriods);
    DateTimePeriod  selectedDateTimePeriod() const;

private slots:
    void slotDialogOkClicked();
    void slotDialogCancelClicked();

private:
    QLabel*                             m_label;
    QComboBox*                          m_timePeriodCombo;

    QDialogButtonBox*                   m_buttons;
};


//==================================================================================================
///  
//==================================================================================================
class RicResampleDialogResult
{
public:
    RicResampleDialogResult(bool ok, DateTimePeriod period) :
        ok(ok), period(period) {}

    bool            ok;
    DateTimePeriod  period;
};