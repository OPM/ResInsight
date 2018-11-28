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
#include <QMenu>

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
class QGroupBox;
class RicSummaryCaseRestartDialogResult;
class QAbstractButton;

//==================================================================================================
///  Constants
//==================================================================================================
#define CURRENT_FILES_LIST_INDEX    0
#define SUMMARY_FILES_LIST_INDEX    1
#define GRID_FILES_LIST_INDEX       2

//==================================================================================================
///  
//==================================================================================================
class RicSummaryCaseRestartDialog : public QDialog
{
    Q_OBJECT

public:
    enum ImportOptions { IMPORT_ALL, SEPARATE_CASES, NOT_IMPORT };

    RicSummaryCaseRestartDialog(QWidget* parent);
    ~RicSummaryCaseRestartDialog() override;

    static RicSummaryCaseRestartDialogResult    openDialog(const QString& initialSummaryFile,
                                                           const QString& initialGridFile,
                                                           bool failOnSummaryImportError,
                                                           bool showApplyToAllWidget,
                                                           bool useFirstSummaryCaseAsTemplate,
                                                           ImportOptions defaultSummaryImportOption,
                                                           ImportOptions defaultGridImportOption,
                                                           RicSummaryCaseRestartDialogResult *lastResult = nullptr,
                                                           QWidget *parent = nullptr);

    ImportOptions                               selectedSummaryImportOption() const;
    ImportOptions                               selectedGridImportOption() const;
    bool                                        okToAllSelected() const;

private:
    void                                        updateFileListWidget(QGridLayout* gridLayout, int listIndex);
    void                                        appendFileInfoToGridLayout(QGridLayout* gridLayout, const RifRestartFileInfo& fileInfo, const QString& fullPathFileName);
    void                                        appendTextToGridLayout(QGridLayout* gridLayout, const QString& text);
    RifRestartFileInfo                          getFileInfo(const QString& summaryHeaderFile);
    void                                        displayWarningsIfAny(const QStringList& warnings);
    QString                                     fullFileName(const QString& shortOrFullFileName);

private slots:
    void slotShowFullPathToggled(int state);
    void slotDialogButtonClicked(QAbstractButton* button);
    void slotFileNameCopyCustomMenuRequested(const QPoint& point);
    void slotCopyFileNameToClipboard();

private:
    QGroupBox*                          m_currentFilesGroup;
    QGridLayout*                        m_currentFilesLayout;

    QGridLayout*                        m_summaryFilesLayout;

    QRadioButton*                       m_summaryReadAllBtn;
    QRadioButton*                       m_summarySeparateCasesBtn;
    QRadioButton*                       m_summaryNotReadBtn;

    QGroupBox*                          m_gridFilesGroup;
    QGridLayout*                        m_gridFilesLayout;

    QRadioButton*                       m_gridNotReadBtn;
    QRadioButton*                       m_gridSeparateCasesBtn;

    QCheckBox*                          m_showFullPathCheckBox;
    QDialogButtonBox*                   m_buttons;

    bool                                m_okToAllPressed;
    QListWidget*                        m_warnings;

    std::vector<std::vector<std::pair<RifRestartFileInfo, QString>>> m_fileLists;
};


//==================================================================================================
///  
//==================================================================================================
class RicSummaryCaseRestartDialogResult
{
public:
    enum Status
    {
        SUMMARY_OK = 0,
        SUMMARY_CANCELLED,
        SUMMARY_WARNING,
        SUMMARY_ERROR
    };
    RicSummaryCaseRestartDialogResult(Status _status = SUMMARY_ERROR) :
        status(_status),
        summaryImportOption(RicSummaryCaseRestartDialog::IMPORT_ALL),
        gridImportOption(RicSummaryCaseRestartDialog::NOT_IMPORT),
        applyToAll(false) {}

    RicSummaryCaseRestartDialogResult(Status _status,
                                      RicSummaryCaseRestartDialog::ImportOptions _summaryImportOption,
                                      RicSummaryCaseRestartDialog::ImportOptions _gridImportOption,
                                      QStringList _summaryFiles,
                                      QStringList _gridFiles,
                                      bool _applyToAll) :
        status(_status),
        summaryImportOption(_summaryImportOption),
        gridImportOption(_gridImportOption),
        summaryFiles(_summaryFiles),
        gridFiles(_gridFiles),
        applyToAll(_applyToAll)
    {
    }

    Status                                              status;
    RicSummaryCaseRestartDialog::ImportOptions          summaryImportOption;
    RicSummaryCaseRestartDialog::ImportOptions          gridImportOption;
    QStringList                                         summaryFiles;
    QStringList                                         gridFiles;
    bool                                                applyToAll;
};
