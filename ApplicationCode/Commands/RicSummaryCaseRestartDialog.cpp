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

#include "RicSummaryCaseRestartDialog.h"
#include "ExportCommands/RicSnapshotViewToClipboardFeature.h"
#include "ExportCommands/RicSnapshotViewToFileFeature.h"
#include "ExportCommands/RicSnapshotFilenameGenerator.h"

#include "RiaApplication.h"
#include "RiaFilePathTools.h"

#include "RifReaderEclipseSummary.h"
#include "RifEclipseSummaryTools.h"

#include "RimEclipseView.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimTools.h"

#include "RiuMainPlotWindow.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuTools.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QRadioButton>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QGroupBox>
#include <QListWidget>
#include <QAbstractItemView>
#include <QMenu>
#include <QDateTime>

#include <vector>
#include <time.h>
#include <thread>

#define DEFAULT_DIALOG_WIDTH        550
#define DEFAULT_DIALOG_INIT_HEIGHT  150


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::RicSummaryCaseRestartDialog(QWidget* parent)
    : QDialog(parent, RiuTools::defaultDialogFlags())
{
    // Create widgets
    m_currentFileGridLayout = new QGridLayout();
    m_readAllRadioButton = new QRadioButton(this);
    m_notReadRadionButton = new QRadioButton(this);
    m_separateCasesRadionButton = new QRadioButton(this);
    m_includeGridHistoryFiles = new QCheckBox(this);
    m_applyToAllCheckBox = new QCheckBox(this);

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect to signals
    connect(m_buttons, SIGNAL(accepted()), this, SLOT(slotDialogOkClicked()));
    connect(m_buttons, SIGNAL(rejected()), this, SLOT(slotDialogCancelClicked()));

    // Set widget properties
    m_readAllRadioButton->setText("Import All Restart Files");
    m_notReadRadionButton->setText("Do Not Import Restart Files");
    m_separateCasesRadionButton->setText("Import Restart Files as Separate Cases");
    m_includeGridHistoryFiles->setText("Import Historic Grid Case Files");
    m_applyToAllCheckBox->setText("Apply to All Files");

    // Define layout
    QVBoxLayout* dialogLayout = new QVBoxLayout();

    QGroupBox* currentFileGroup = new QGroupBox("Current Summary File");
    m_currentFileGridLayout = new QGridLayout();
    currentFileGroup->setLayout(m_currentFileGridLayout);

    QGroupBox* filesGroup = new QGroupBox("Found Restart Files");
    m_filesGridLayout = new QGridLayout();
    filesGroup->setLayout(m_filesGridLayout);

    QGroupBox* optionsGroup = new QGroupBox("Read Options");
    QVBoxLayout* optionsLayout = new QVBoxLayout();
    optionsLayout->addWidget(m_readAllRadioButton);
    optionsLayout->addWidget(m_notReadRadionButton);
    optionsLayout->addWidget(m_separateCasesRadionButton);
    optionsLayout->addWidget(m_includeGridHistoryFiles);
    optionsGroup->setLayout(optionsLayout);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(m_applyToAllCheckBox);
    buttonsLayout->addWidget(m_buttons);

    dialogLayout->addWidget(currentFileGroup);
    dialogLayout->addWidget(filesGroup);
    dialogLayout->addWidget(optionsGroup);
    dialogLayout->addLayout(buttonsLayout);

    setLayout(dialogLayout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::~RicSummaryCaseRestartDialog()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialogResult RicSummaryCaseRestartDialog::openDialog(const QString& summaryHeaderFile,
                                                                          bool showApplyToAllWidget,
                                                                          bool buildGridCaseFileList,
                                                                          ReadOptions defaultReadOption,
                                                                          RicSummaryCaseRestartDialogResult *lastResult,
                                                                          QWidget *parent)
{
    RicSummaryCaseRestartDialog  dialog(parent);

    RifRestartFileInfo currentFileInfo = dialog.getFileInfo(summaryHeaderFile);
    if (!currentFileInfo.valid())
    {
        return RicSummaryCaseRestartDialogResult();
    }

    RifReaderEclipseSummary reader;
    std::vector<RifRestartFileInfo> restartFileInfos = reader.getRestartFiles(summaryHeaderFile);

    // If no restart files are found, do not show dialog
    if (restartFileInfos.empty())
    {
        QString gridCaseFile = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(summaryHeaderFile);
        return RicSummaryCaseRestartDialogResult(true, NOT_IMPORT, QStringList({ summaryHeaderFile }), false, false, QStringList({ gridCaseFile }));
    }

    RicSummaryCaseRestartDialogResult dialogResult;
    if (lastResult && lastResult->applyToAll)
    {
        dialogResult = *lastResult;
        dialogResult.summaryFiles.clear();
        dialogResult.restartGridFilesToImport.clear();
    }
    else
    {
        dialog.setWindowTitle("Summary Case Restart Files");

        dialog.appendFileInfoToGridLayout(*dialog.m_currentFileGridLayout, currentFileInfo);
        for (const auto& restartFileInfo : restartFileInfos)
        {
            dialog.appendFileInfoToGridLayout(*dialog.m_filesGridLayout, restartFileInfo);
        }

        switch (defaultReadOption)
        {
        case ReadOptions::IMPORT_ALL:       dialog.m_readAllRadioButton->setChecked(true); break;
        case ReadOptions::NOT_IMPORT:       dialog.m_notReadRadionButton->setChecked(true); break;
        case ReadOptions::SEPARATE_CASES:   dialog.m_separateCasesRadionButton->setChecked(true); break;
        }
        dialog.m_includeGridHistoryFiles->setVisible(buildGridCaseFileList);
        dialog.m_includeGridHistoryFiles->setChecked(lastResult->includeGridHistoryFiles);
        dialog.m_applyToAllCheckBox->setVisible(showApplyToAllWidget);
        dialog.resize(DEFAULT_DIALOG_WIDTH, DEFAULT_DIALOG_INIT_HEIGHT);
        dialog.exec();

        dialogResult = RicSummaryCaseRestartDialogResult(dialog.result() == QDialog::Accepted,
                                                         dialog.selectedOption(),
                                                         {},
                                                         dialog.applyToAllSelected(),
                                                         dialog.includeGridHistoryFiles());
    }

    if (!dialogResult.ok)
    {
        return RicSummaryCaseRestartDialogResult(false, NOT_IMPORT, QStringList(), false, false);
    }

    dialogResult.summaryFiles.push_back(RiaFilePathTools::toInternalSeparator(summaryHeaderFile));
    if (dialogResult.option == SEPARATE_CASES)
    {
        for (const auto& restartFileInfo : restartFileInfos)
        {
            dialogResult.summaryFiles.push_back(RiaFilePathTools::toInternalSeparator(restartFileInfo.fileName));
        }
    }

    if (dialogResult.includeGridHistoryFiles)
    {
        QString gridCaseFile = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(summaryHeaderFile);
        dialogResult.restartGridFilesToImport.push_back(gridCaseFile);

        for (const auto& restartFileInfo : restartFileInfos)
        {
            QString gridCaseFile = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(restartFileInfo.fileName);
            if (buildGridCaseFileList && !gridCaseFile.isEmpty()) dialogResult.restartGridFilesToImport.push_back(gridCaseFile);
        }
    }
    return dialogResult;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::ReadOptions RicSummaryCaseRestartDialog::selectedOption() const
{
    return
        m_readAllRadioButton->isChecked() ?         IMPORT_ALL :
        m_separateCasesRadionButton->isChecked() ?  SEPARATE_CASES :
                                                    NOT_IMPORT;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSummaryCaseRestartDialog::applyToAllSelected() const
{
    return m_applyToAllCheckBox->isChecked();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSummaryCaseRestartDialog::includeGridHistoryFiles() const
{
    return m_includeGridHistoryFiles->isChecked();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::appendFileInfoToGridLayout(QGridLayout& gridLayout, const RifRestartFileInfo& fileInfo)
{
    QDateTime startDate = QDateTime::fromTime_t(fileInfo.startDate);
    QString startDateString = startDate.toString(RimTools::dateFormatString());
    QDateTime endDate = QDateTime::fromTime_t(fileInfo.endDate);
    QString endDateString = endDate.toString(RimTools::dateFormatString());
    int rowCount = gridLayout.rowCount();

    QLabel* fileNameLabel = new QLabel();
    QLabel* dateLabel = new QLabel();
    fileNameLabel->setText(QFileInfo(fileInfo.fileName).fileName());
    dateLabel->setText(startDateString + " - " + endDateString);

    fileNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    gridLayout.addWidget(fileNameLabel, rowCount, 0);
    gridLayout.addWidget(dateLabel, rowCount, 1);

    // Full path in tooltip
    fileNameLabel->setToolTip(fileInfo.fileName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifRestartFileInfo> RicSummaryCaseRestartDialog::getRestartFiles(const QString& summaryHeaderFile)
{
    RifReaderEclipseSummary reader;
    return reader.getRestartFiles(summaryHeaderFile);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifRestartFileInfo RicSummaryCaseRestartDialog::getFileInfo(const QString& summaryHeaderFile)
{
    RifReaderEclipseSummary reader;
    return reader.getFileInfo(summaryHeaderFile);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::slotDialogOkClicked()
{
    accept();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCaseRestartDialog::slotDialogCancelClicked()
{
    reject();
}

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------

