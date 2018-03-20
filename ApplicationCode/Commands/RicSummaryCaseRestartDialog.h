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
class QCheckBox;
class RicSummaryCaseRestartDialogResult;

//==================================================================================================
///  
//==================================================================================================
class RicSummaryCaseRestartDialog : public QDialog
{
    Q_OBJECT

public:
    enum ReadOptions { READ_SINGLE, READ_ALL, SEPARATE_CASES };

    RicSummaryCaseRestartDialog(QWidget* parent);
    ~RicSummaryCaseRestartDialog();

    static RicSummaryCaseRestartDialogResult    openDialog(const QString& summaryHeaderFile,
                                                           bool showApplyToAllWidget,
                                                           RicSummaryCaseRestartDialogResult *lastResult = nullptr,
                                                           QWidget *parent = nullptr);

    ReadOptions                                 selectedOption() const;
    bool                                        applyToAllSelected() const;

private:
    void                                        appendFileInfoToGridLayout(QGridLayout& gridLayout, const RifRestartFileInfo& fileInfo);
    std::vector<RifRestartFileInfo>             getRestartFiles(const QString& summaryHeaderFile);
    RifRestartFileInfo                          getFileInfo(const QString& summaryHeaderFile);

private slots:
    void slotDialogOkClicked();
    void slotDialogCancelClicked();

private:
    QGridLayout*                        m_currentFileGridLayout;

    QGridLayout*                        m_filesGridLayout;

    QRadioButton*                       m_readAllRadioButton;
    QRadioButton*                       m_notReadRadionButton;
    QRadioButton*                       m_separateCasesRadionButton;

    QCheckBox*                          m_applyToAllCheckBox;
    QDialogButtonBox*                   m_buttons;
};


//==================================================================================================
///  
//==================================================================================================
class RicSummaryCaseRestartDialogResult
{
public:
    RicSummaryCaseRestartDialogResult() :
        ok(false), option(RicSummaryCaseRestartDialog::READ_ALL), applyToAll(false) {}

    RicSummaryCaseRestartDialogResult(bool _ok,
                                      RicSummaryCaseRestartDialog::ReadOptions _option,
                                      QStringList _files,
                                      bool _applyToAll) :
        ok(_ok), option(_option), files(_files), applyToAll(_applyToAll) {
    }

    bool                                        ok;
    RicSummaryCaseRestartDialog::ReadOptions    option;
    QStringList                                 files;
    bool                                        applyToAll;
};