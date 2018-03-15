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

#include "Rim3dOverlayInfoConfig.h"

#include "RifReaderEclipseSummary.h"

#include "cafPdmPointer.h"

#include <QDialog>

class QLabel;
class QRadioButton;
class QLineEdit;
class QTextEdit;
class QDialogButtonBox;
class QPushButton;
class QMainWindow;
class QListWidget;
class QGridLayout;
class RicSummaryCaseRestartDialogResult;

//==================================================================================================
///  
//==================================================================================================
class RicSummaryCaseRestartDialog : public QDialog
{
    Q_OBJECT

public:
    enum ReadOptions { READ_ALL, NOT_READ, SEPARATE_CASES };

    RicSummaryCaseRestartDialog(QWidget* parent);
    ~RicSummaryCaseRestartDialog();

    static RicSummaryCaseRestartDialogResult    openDialog(const QString& summaryHeaderFile, QWidget *parent = nullptr);

    ReadOptions                                 selectedOption() const;

private:
    void                                        appendToFileList(const RifRestartFileInfo& fileInfo);
    std::vector<RifRestartFileInfo>             getRestartFiles(const QString& summaryHeaderFile);

private slots:
    void slotDialogOkClicked();
    void slotDialogCancelClicked();

private:
    QGridLayout *                       m_filesGridLayout;

    QRadioButton*                       m_readAllRadioButton;
    QRadioButton*                       m_notReadRadionButton;
    QRadioButton*                       m_separateCasesRadionButton;

    QDialogButtonBox*                   m_buttons;
};


//==================================================================================================
///  
//==================================================================================================
class RicSummaryCaseRestartDialogResult
{
public:
    RicSummaryCaseRestartDialogResult(bool ok, 
                                 RicSummaryCaseRestartDialog::ReadOptions option) :
        ok(ok), option(option) {}

    bool            ok;
    RicSummaryCaseRestartDialog::ReadOptions  option;
};